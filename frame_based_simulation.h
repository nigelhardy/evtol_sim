#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <chrono>
#include <future>
#include <iostream>

#include "simulation_interface.h"
#include "simulation_config.h"
#include "aircraft_state.h"

namespace evtol
{
    /**
     * Frame-based simulation engine with multithreading support
     */
    class FrameBasedSimulationEngine : public SimulationEngineBase
    {
    private:
        SimulationConfig config_;
        std::vector<std::thread> worker_threads_;

        // Threading synchronization
        std::condition_variable frame_cv_;
        std::mutex frame_mutex_;
        std::atomic<int> threads_completed_{0};

        // Frame-based state
        std::vector<AircraftFrameData> aircraft_frame_data_;
        std::vector<std::unique_ptr<AircraftStateMachine>> state_machines_;

        // Performance tracking
        std::chrono::high_resolution_clock::time_point last_frame_time_;
        double frame_time_seconds_;

        // Logging helper
        void log_event(const std::string& message) const
        {
            if (config_.enable_detailed_logging)
            {
                std::cout << "[" << current_time_hours_ << "h] " << message << std::endl;
            }
        }

    public:
        FrameBasedSimulationEngine(StatisticsCollector &stats, const SimulationConfig &config);
        ~FrameBasedSimulationEngine();

        // ISimulationEngine interface
        void stop() override;

    protected:
        void run_simulation_impl(ChargerManager &charger_mgr, void *fleet_ptr) override;

    private:
        template <typename Fleet>
        void run_frame_based_simulation(ChargerManager &charger_mgr, Fleet &fleet);

        template <typename Fleet>
        void initialize_aircraft_states(Fleet &fleet);

        template <typename Fleet>
        void update_frame(ChargerManager &charger_mgr, Fleet &fleet);

        template <typename Fleet>
        void update_aircraft_subset(ChargerManager &charger_mgr, Fleet &fleet,
                                    size_t start_idx, size_t end_idx);

        template <typename Fleet>
        void process_aircraft_state(ChargerManager &charger_mgr, Fleet &fleet,
                                    size_t aircraft_idx);

        template <typename Fleet>
        void handle_flight_completion(ChargerManager &charger_mgr, Fleet &fleet,
                                      size_t aircraft_idx);

        template <typename Fleet>
        void handle_charging_completion(ChargerManager &charger_mgr, Fleet &fleet,
                                        size_t aircraft_idx);

        template <typename Fleet>
        void start_new_flight(Fleet &fleet, size_t aircraft_idx);

        template <typename Fleet>
        void start_charging(ChargerManager &charger_mgr, Fleet &fleet, size_t aircraft_idx);

        // Partial activity handling
        template <typename Fleet>
        void finalize_simulation(Fleet &fleet);

        void handle_partial_flight(std::unique_ptr<AircraftBase> &aircraft, AircraftFrameData &frame_data);

        void handle_partial_charging(std::unique_ptr<AircraftBase> &aircraft, AircraftFrameData &frame_data);

        // Threading support
        void start_worker_threads();
        void stop_worker_threads();
        void worker_thread_main(int thread_id);
        void wait_for_all_threads();
        void signal_frame_start();
        void signal_frame_complete();

        // Timing
        void wait_for_frame_time();
        double get_frame_delta_time();

        // State validation
        template <typename Fleet>
        bool validate_simulation_state(Fleet &fleet) const;

        // Utility functions
        static std::string aircraft_type_to_string(AircraftType type);
    };

    // Template implementation
    template <typename Fleet>
    void FrameBasedSimulationEngine::run_frame_based_simulation(ChargerManager &charger_mgr, Fleet &fleet)
    {
        log_event("=== Starting frame-based simulation ===");
        log_event("Fleet size: " + std::to_string(fleet.size()));
        log_event("Available chargers: " + std::to_string(charger_mgr.get_available_chargers()));
        log_event("Frame time: " + std::to_string(frame_time_seconds_) + " seconds");

        initialize_aircraft_states(fleet);

        // Disable multithreading for now to avoid deadlocks
        // if (config_.enable_multithreading && config_.num_threads > 1)
        // {
        //     start_worker_threads();
        // }

        is_running_ = true;
        last_frame_time_ = std::chrono::high_resolution_clock::now();

        int frame_count = 0;
        double last_log_time = 0.0;

        while (is_running_ && current_time_hours_ < simulation_duration_hours_)
        {
            // Update frame
            update_frame(charger_mgr, fleet);

            // Advance time
            current_time_hours_ += frame_time_seconds_ / 3600.0;
            frame_count++;

            // Log frame progress every 0.5 hours
            if (current_time_hours_ - last_log_time >= 0.5)
            {
                log_event("Frame " + std::to_string(frame_count) + " completed - Time: " + std::to_string(current_time_hours_) + "h");
                last_log_time = current_time_hours_;
            }
        }

        // Disable multithreading for now to avoid deadlocks
        // if (config_.enable_multithreading && config_.num_threads > 1)
        // {
        //     stop_worker_threads();
        // }

        // Handle partial activities if enabled
        if (config_.enable_partial_flights)
        {
            log_event("=== Processing partial activities ===");
            finalize_simulation(fleet);
        }
        
        log_event("=== Frame-based simulation completed ===");
        log_event("Total frames processed: " + std::to_string(frame_count));
        is_running_ = false;
    }

    template <typename Fleet>
    void FrameBasedSimulationEngine::initialize_aircraft_states(Fleet &fleet)
    {
        log_event("Initializing aircraft states...");
        aircraft_frame_data_.resize(fleet.size());
        state_machines_.clear();
        state_machines_.reserve(fleet.size());

        for (size_t i = 0; i < fleet.size(); ++i)
        {
            auto &frame_data = aircraft_frame_data_[i];
            frame_data.reset_for_activity(AircraftState::IDLE, 0.0);
            state_machines_.push_back(std::make_unique<AircraftStateMachine>(frame_data));

            // Schedule initial flight
            start_new_flight(fleet, i);
        }
        log_event("Aircraft states initialized - all aircraft scheduled for initial flights");
    }

    template <typename Fleet>
    void FrameBasedSimulationEngine::update_frame(ChargerManager &charger_mgr, Fleet &fleet)
    {
        // Always use single-threaded for now
        update_aircraft_subset(charger_mgr, fleet, 0, fleet.size());
    }

    template <typename Fleet>
    void FrameBasedSimulationEngine::update_aircraft_subset(ChargerManager &charger_mgr, Fleet &fleet,
                                                            size_t start_idx, size_t end_idx)
    {
        for (size_t i = start_idx; i < end_idx; ++i)
        {
            process_aircraft_state(charger_mgr, fleet, i);
        }
    }

    template <typename Fleet>
    void FrameBasedSimulationEngine::process_aircraft_state(ChargerManager &charger_mgr, Fleet &fleet,
                                                            size_t aircraft_idx)
    {
        auto &frame_data = aircraft_frame_data_[aircraft_idx];
        auto &state_machine = *state_machines_[aircraft_idx];

        double delta_time = frame_time_seconds_; // This is in seconds

        // Update state machine
        // TODO does this return something useful? I removed monitor, so maybe we can update that return
        state_machine.update_frame(delta_time);

        // Process based on current state
        switch (frame_data.get_state())
        {
        case AircraftState::FLYING:
            frame_data.update_time_remaining(delta_time);
            if (frame_data.time_remaining <= 0.0)
            {
                handle_flight_completion(charger_mgr, fleet, aircraft_idx);
            }
            break;

        case AircraftState::CHARGING:
            frame_data.update_time_remaining(delta_time);
            if (frame_data.time_remaining <= 0.0)
            {
                handle_charging_completion(charger_mgr, fleet, aircraft_idx);
            }
            break;

        case AircraftState::WAITING_FOR_CHARGER:
            // Try to get a charger
            if (charger_mgr.request_charger(fleet[aircraft_idx]->get_id()))
            {
                // Calculate waiting time
                double waiting_time = (current_time_hours_ - frame_data.waiting_start_time) * 3600.0; // Convert to seconds
                frame_data.accumulated_waiting_time = waiting_time;

                log_event("Aircraft " + std::to_string(fleet[aircraft_idx]->get_id()) + " assigned charger after waiting " + 
                         std::to_string(waiting_time / 3600.0) + "h");
                start_charging(charger_mgr, fleet, aircraft_idx);
            }
            break;

        case AircraftState::IDLE:
            // Aircraft is ready to fly
            start_new_flight(fleet, aircraft_idx);
            break;

        case AircraftState::FAULT:
            // Aircraft has a fault, stays in this state
            break;
        }
    }

    template <typename Fleet>
    void FrameBasedSimulationEngine::handle_flight_completion(ChargerManager &charger_mgr, Fleet &fleet,
                                                              size_t aircraft_idx)
    {
        auto &aircraft = fleet[aircraft_idx];
        auto &frame_data = aircraft_frame_data_[aircraft_idx];

        log_event("Aircraft " + std::to_string(aircraft->get_id()) + " completed flight (" + 
                 std::to_string(frame_data.current_flight_distance) + " miles, " + 
                 std::to_string(frame_data.current_flight_time) + "h)");

        // Discharge battery
        aircraft->discharge_battery();

        // Record statistics
        stats_collector_.record_flight(aircraft->get_type(),
                                       frame_data.current_flight_time,
                                       frame_data.current_flight_distance,
                                       aircraft->get_passenger_count());

        if (frame_data.fault_occurred)
        {
            log_event("Aircraft " + std::to_string(aircraft->get_id()) + " experienced fault during flight - aircraft grounded");
            stats_collector_.record_fault(aircraft->get_type());
            frame_data.transition_to(AircraftState::FAULT);
            return;
        }

        // Try to get a charger
        if (charger_mgr.request_charger(aircraft->get_id()))
        {
            log_event("Aircraft " + std::to_string(aircraft->get_id()) + " assigned to charger immediately");
            start_charging(charger_mgr, fleet, aircraft_idx);
        }
        else
        {
            log_event("Aircraft " + std::to_string(aircraft->get_id()) + " added to charging queue (no chargers available)");
            charger_mgr.add_to_queue(aircraft->get_id());
            frame_data.waiting_start_time = current_time_hours_;
            frame_data.accumulated_waiting_time = 0.0;
            frame_data.transition_to(AircraftState::WAITING_FOR_CHARGER);
        }
    }

    template <typename Fleet>
    void FrameBasedSimulationEngine::handle_charging_completion(ChargerManager &charger_mgr, Fleet &fleet,
                                                                size_t aircraft_idx)
    {
        auto &aircraft = fleet[aircraft_idx];
        auto &frame_data = aircraft_frame_data_[aircraft_idx];

        double waiting_time_hours = frame_data.accumulated_waiting_time / 3600.0; // Convert to hours
        log_event("Aircraft " + std::to_string(aircraft->get_id()) + " completed charging (" + 
                 std::to_string(aircraft->get_charge_time_hours()) + "h charge, " + 
                 std::to_string(waiting_time_hours) + "h wait)");

        // Charge battery
        aircraft->charge_battery();

        // Record statistics including waiting time
        stats_collector_.record_charge_session(aircraft->get_type(),
                                               aircraft->get_charge_time_hours(),
                                               waiting_time_hours);

        // Release charger
        charger_mgr.release_charger(aircraft->get_id());
        frame_data.charger_id = -1;

        // Start next aircraft in queue
        int next_aircraft_id = charger_mgr.get_next_from_queue();
        if (next_aircraft_id != -1)
        {
            // Find the aircraft in the fleet
            for (size_t i = 0; i < fleet.size(); ++i)
            {
                if (fleet[i]->get_id() == next_aircraft_id)
                {
                    charger_mgr.assign_charger(next_aircraft_id);

                    // Calculate waiting time for this aircraft
                    auto &next_frame_data = aircraft_frame_data_[i];
                    double waiting_time = (current_time_hours_ - next_frame_data.waiting_start_time) * 3600.0; // Convert to seconds
                    next_frame_data.accumulated_waiting_time = waiting_time;

                    log_event("Aircraft " + std::to_string(next_aircraft_id) + " removed from queue and assigned charger (waited " + 
                             std::to_string(waiting_time / 3600.0) + "h)");
                    start_charging(charger_mgr, fleet, i);
                    break;
                }
            }
        }
        else
        {
            log_event("Charger freed but no aircraft waiting in queue");
        }

        log_event("Aircraft " + std::to_string(aircraft->get_id()) + " ready for next flight");
        // Transition to idle state
        frame_data.transition_to(AircraftState::IDLE);
    }

    template <typename Fleet>
    void FrameBasedSimulationEngine::start_new_flight(Fleet &fleet, size_t aircraft_idx)
    {
        auto &aircraft = fleet[aircraft_idx];
        auto &frame_data = aircraft_frame_data_[aircraft_idx];

        // Only start if aircraft is idle
        if (frame_data.get_state() != AircraftState::IDLE)
        {
            return;
        }

        double flight_time = aircraft->get_flight_time_hours();
        double flight_distance = aircraft->get_flight_distance_miles();

        // Set flight-specific data first
        frame_data.current_flight_time = flight_time;
        frame_data.current_flight_distance = flight_distance;
        bool will_fault = aircraft->check_fault_during_flight(flight_time) > 0;

        log_event("Starting flight for aircraft " + std::to_string(aircraft->get_id()) + 
                 " (distance: " + std::to_string(flight_distance) + " miles, flight time: " + 
                 std::to_string(flight_time) + "h)");

        if (will_fault)
        {
            log_event("Aircraft " + std::to_string(aircraft->get_id()) + " will experience fault during this flight");
        }

        frame_data.reset_for_activity(AircraftState::FLYING, flight_time * 3600.0); // Convert to seconds

        // Set fault status after reset (since reset clears fault_occurred)
        frame_data.fault_occurred = will_fault;
    }

    template <typename Fleet>
    void FrameBasedSimulationEngine::start_charging(ChargerManager &charger_mgr, Fleet &fleet, size_t aircraft_idx)
    {
        auto &aircraft = fleet[aircraft_idx];
        auto &frame_data = aircraft_frame_data_[aircraft_idx];

        double charge_time = aircraft->get_charge_time_hours();
        frame_data.charger_id = charger_mgr.get_charger_id(aircraft->get_id());

        double waiting_time_hours = frame_data.accumulated_waiting_time / 3600.0; // Convert to hours
        log_event("Starting charging for aircraft " + std::to_string(aircraft->get_id()) + 
                 " (charge time: " + std::to_string(charge_time) + "h, waited: " + 
                 std::to_string(waiting_time_hours) + "h)");

        // If accumulated_waiting_time is 0, it means this aircraft got a charger immediately
        // and wasn't waiting

        frame_data.reset_for_activity(AircraftState::CHARGING, charge_time * 3600.0); // Convert to seconds
    }

    template <typename Fleet>
    void FrameBasedSimulationEngine::finalize_simulation(Fleet &fleet)
    {
        // Set current time to simulation end for partial calculation
        current_time_hours_ = simulation_duration_hours_;

        // Process partial activities for aircraft still in flight or charging
        for (size_t i = 0; i < fleet.size(); ++i)
        {
            auto &aircraft = fleet[i];
            auto &frame_data = aircraft_frame_data_[i];
            
            switch (frame_data.get_state())
            {
                case AircraftState::FLYING:
                    handle_partial_flight(aircraft, frame_data);
                    break;
                    
                case AircraftState::CHARGING:
                    handle_partial_charging(aircraft, frame_data);
                    break;
                    
                default:
                    // No partial activity for other states
                    break;
            }
        }
    }


    template <typename Fleet>
    bool FrameBasedSimulationEngine::validate_simulation_state(Fleet &fleet) const
    {
        // Validate that all aircraft are in valid states
        for (size_t i = 0; i < fleet.size(); ++i)
        {
            auto state = aircraft_frame_data_[i].get_state();
            if (state == AircraftState::CHARGING)
            {
                if (aircraft_frame_data_[i].charger_id == -1)
                {
                    return false; // Aircraft charging but no charger assigned
                }
            }
        }
        return true;
    }
}