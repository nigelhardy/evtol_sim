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
#include "simulation_monitor.h"

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
        std::atomic<bool> paused_{false};
        std::atomic<double> speed_multiplier_{1.0};
        
        // Threading synchronization
        std::condition_variable frame_cv_;
        std::mutex frame_mutex_;
        std::atomic<int> threads_completed_{0};
        
        // Frame-based state
        std::vector<AircraftFrameData> aircraft_frame_data_;
        std::vector<std::unique_ptr<AircraftStateMachine>> state_machines_;
        
        // Monitoring
        std::unique_ptr<ISimulationMonitor> monitor_;
        
        // Performance tracking
        std::chrono::high_resolution_clock::time_point last_frame_time_;
        double frame_time_seconds_;
        
    public:
        FrameBasedSimulationEngine(StatisticsCollector &stats, const SimulationConfig& config);
        ~FrameBasedSimulationEngine();
        
        // ISimulationEngine interface
        void pause() override;
        void resume() override;
        void stop() override;
        void set_speed_multiplier(double multiplier) override;
        
        // Configuration
        void set_monitor(std::unique_ptr<ISimulationMonitor> monitor);
        
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
        
        template <typename Fleet>
        SimulationSnapshot create_snapshot(ChargerManager &charger_mgr, Fleet &fleet);
        
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
        initialize_aircraft_states(fleet);
        
        // Disable multithreading for now to avoid deadlocks
        // if (config_.enable_multithreading && config_.num_threads > 1)
        // {
        //     start_worker_threads();
        // }
        
        if (monitor_)
        {
            monitor_->on_simulation_start(config_);
        }
        
        is_running_ = true;
        last_frame_time_ = std::chrono::high_resolution_clock::now();
        
        // std::cout << "Starting frame-based simulation loop..." << std::endl;
        
        while (is_running_ && current_time_hours_ < simulation_duration_hours_)
        {
            // Handle pause
            if (paused_)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            
            // Check monitor for pause/stop requests
            if (monitor_)
            {
                if (monitor_->should_stop())
                {
                    stop();
                    break;
                }
                if (monitor_->should_pause())
                {
                    pause();
                    continue;
                }
            }
            
            // Update frame
            update_frame(charger_mgr, fleet);
            
            // Update monitoring
            if (monitor_)
            {
                auto snapshot = create_snapshot(charger_mgr, fleet);
                monitor_->on_frame_update(snapshot);
            }
            
            // Advance time
            current_time_hours_ += frame_time_seconds_ / 3600.0;
            
            // Debug output (disabled)
            // if (static_cast<int>(current_time_hours_ * 10) % 5 == 0) { // Every 0.5 hours
            //     std::cout << "Time: " << current_time_hours_ << " hours" << std::endl;
            // }
            
            // Wait for frame time if real-time mode
            if (config_.enable_real_time)
            {
                wait_for_frame_time();
            }
        }
        
        // Disable multithreading for now to avoid deadlocks
        // if (config_.enable_multithreading && config_.num_threads > 1)
        // {
        //     stop_worker_threads();
        // }
        
        if (monitor_)
        {
            auto final_stats = stats_collector_.get_summary_stats();
            monitor_->on_simulation_end(final_stats);
        }
        
        is_running_ = false;
    }

    template <typename Fleet>
    void FrameBasedSimulationEngine::initialize_aircraft_states(Fleet &fleet)
    {
        aircraft_frame_data_.resize(fleet.size());
        state_machines_.clear();
        state_machines_.reserve(fleet.size());
        
        for (size_t i = 0; i < fleet.size(); ++i)
        {
            auto& frame_data = aircraft_frame_data_[i];
            frame_data.reset_for_activity(AircraftState::IDLE, 0.0);
            state_machines_.push_back(std::make_unique<AircraftStateMachine>(frame_data));
            
            // Schedule initial flight
            start_new_flight(fleet, i);
        }
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
        auto& frame_data = aircraft_frame_data_[aircraft_idx];
        auto& state_machine = *state_machines_[aircraft_idx];
        
        double delta_time = frame_time_seconds_; // This is in seconds
        
        // Update state machine
        if (state_machine.update_frame(delta_time))
        {
            // State changed, notify monitor
            if (monitor_)
            {
                // Get old and new states (simplified - would need to track previous state)
                auto current_state = frame_data.get_state();
                monitor_->on_aircraft_state_change(fleet[aircraft_idx]->get_id(), 
                                                 current_state, current_state);
            }
        }
        
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
        auto& aircraft = fleet[aircraft_idx];
        auto& frame_data = aircraft_frame_data_[aircraft_idx];
        
        // Discharge battery
        aircraft->discharge_battery();
        
        // Record statistics
        stats_collector_.record_flight(aircraft->get_type(), 
                                     frame_data.current_flight_time,
                                     frame_data.current_flight_distance,
                                     aircraft->get_passenger_count());
        
        if (frame_data.fault_occurred)
        {
            stats_collector_.record_fault(aircraft->get_type());
            if (monitor_)
            {
                monitor_->on_fault_occurred(aircraft->get_id(), current_time_hours_);
            }
            frame_data.transition_to(AircraftState::FAULT);
            return;
        }
        
        // Try to get a charger
        if (charger_mgr.request_charger(aircraft->get_id()))
        {
            start_charging(charger_mgr, fleet, aircraft_idx);
        }
        else
        {
            charger_mgr.add_to_queue(aircraft->get_id());
            frame_data.transition_to(AircraftState::WAITING_FOR_CHARGER);
        }
    }

    template <typename Fleet>
    void FrameBasedSimulationEngine::handle_charging_completion(ChargerManager &charger_mgr, Fleet &fleet,
                                                               size_t aircraft_idx)
    {
        auto& aircraft = fleet[aircraft_idx];
        auto& frame_data = aircraft_frame_data_[aircraft_idx];
        
        // Charge battery
        aircraft->charge_battery();
        
        // Record statistics
        stats_collector_.record_charge_session(aircraft->get_type(),
                                              aircraft->get_charge_time_hours());
        
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
                    start_charging(charger_mgr, fleet, i);
                    break;
                }
            }
        }
        
        // Transition to idle state
        frame_data.transition_to(AircraftState::IDLE);
    }

    template <typename Fleet>
    void FrameBasedSimulationEngine::start_new_flight(Fleet &fleet, size_t aircraft_idx)
    {
        auto& aircraft = fleet[aircraft_idx];
        auto& frame_data = aircraft_frame_data_[aircraft_idx];
        
        // Only start if aircraft is idle
        if (frame_data.get_state() != AircraftState::IDLE) {
            return;
        }
        
        double flight_time = aircraft->get_flight_time_hours();
        double flight_distance = aircraft->get_flight_distance_miles();
        
        frame_data.current_flight_time = flight_time;
        frame_data.current_flight_distance = flight_distance;
        frame_data.fault_occurred = aircraft->check_fault_during_flight(flight_time);
        
        frame_data.reset_for_activity(AircraftState::FLYING, flight_time * 3600.0); // Convert to seconds
    }

    template <typename Fleet>
    void FrameBasedSimulationEngine::start_charging(ChargerManager &charger_mgr, Fleet &fleet, size_t aircraft_idx)
    {
        auto& aircraft = fleet[aircraft_idx];
        auto& frame_data = aircraft_frame_data_[aircraft_idx];
        
        double charge_time = aircraft->get_charge_time_hours();
        frame_data.charger_id = charger_mgr.get_charger_id(aircraft->get_id());
        
        frame_data.reset_for_activity(AircraftState::CHARGING, charge_time * 3600.0); // Convert to seconds
    }

    template <typename Fleet>
    SimulationSnapshot FrameBasedSimulationEngine::create_snapshot(ChargerManager &charger_mgr, Fleet &fleet)
    {
        SimulationSnapshot snapshot;
        snapshot.current_time_hours = current_time_hours_;
        snapshot.simulation_duration_hours = simulation_duration_hours_;
        snapshot.is_running = is_running_;
        snapshot.is_paused = paused_;
        snapshot.speed_multiplier = speed_multiplier_;
        
        // Aircraft states
        snapshot.aircraft_states.reserve(fleet.size());
        for (size_t i = 0; i < fleet.size(); ++i)
        {
            auto& aircraft = fleet[i];
            auto& frame_data = aircraft_frame_data_[i];
            
            SimulationSnapshot::AircraftInfo info;
            info.id = aircraft->get_id();
            info.type = aircraft_type_to_string(aircraft->get_type());
            info.state = frame_data.get_state();
            info.time_remaining = frame_data.time_remaining;
            info.battery_level = aircraft->get_battery_level();
            info.charger_id = frame_data.charger_id;
            info.fault_occurred = frame_data.fault_occurred;
            
            snapshot.aircraft_states.push_back(info);
        }
        
        // Charger states
        for (int i = 0; i < charger_mgr.get_num_chargers(); ++i)
        {
            SimulationSnapshot::ChargerInfo info;
            info.id = i;
            info.is_occupied = charger_mgr.is_charger_occupied(i);
            info.aircraft_id = charger_mgr.get_aircraft_at_charger(i);
            info.waiting_queue = charger_mgr.get_waiting_queue();
            
            snapshot.charger_states.push_back(info);
        }
        
        // Current statistics
        snapshot.current_stats = stats_collector_.get_summary_stats();
        
        return snapshot;
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