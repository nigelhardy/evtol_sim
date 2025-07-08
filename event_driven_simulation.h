#pragma once
#include <queue>
#include <vector>
#include <memory>
#include <chrono>
#include <variant>
#include <iostream>
#include <unordered_map>

#include "charger_manager.h"
#include "statistics_engine.h"
#include "simulation_interface.h"

namespace evtol
{
    enum class EventType
    {
        FLIGHT_COMPLETE,
        CHARGING_COMPLETE,
        FAULT_OCCURRED
    };

    template <typename T>
    struct Event
    {
        EventType type;
        double time_hours;
        T data;

        Event(EventType t, double time, T d) : type(t), time_hours(time), data(std::move(d)) {}

        bool operator<(const Event &other) const
        {
            return time_hours > other.time_hours;
        }
    };

    struct FlightCompleteData
    {
        int aircraft_id;
        double flight_time;
        double distance;
        bool fault_occurred;
    };

    struct ChargingCompleteData
    {
        int aircraft_id;
        double charge_time;
        double waiting_time;
    };

    struct FaultData
    {
        int aircraft_id;
        double fault_time;
    };

    using EventData = std::variant<FlightCompleteData, ChargingCompleteData, FaultData>;
    using SimulationEvent = Event<EventData>;

    class EventDrivenSimulation
    {
    private:
        std::priority_queue<SimulationEvent> event_queue_;
        double current_time_hours_;
        double simulation_duration_hours_;
        StatisticsCollector &stats_collector_;
        std::unordered_map<int, double> waiting_start_times_;
        std::unordered_map<int, double> flight_start_times_;
        std::unordered_map<int, double> charging_start_times_;
        bool enable_detailed_logging_;
        bool enable_partial_flights_;

        void log_event(const std::string& message) const
        {
            if (enable_detailed_logging_)
            {
                std::cout << "[" << current_time_hours_ << "h] " << message << std::endl;
            }
        }

    public:
        EventDrivenSimulation(StatisticsCollector &stats, double duration_hours = 3.0, bool detailed_logging = false, bool partial_flights = true)
            : current_time_hours_(0.0), simulation_duration_hours_(duration_hours),
              stats_collector_(stats), enable_detailed_logging_(detailed_logging), enable_partial_flights_(partial_flights)
        {
        }

        template <typename Fleet>
        void run_simulation(ChargerManager &charger_mgr, Fleet &fleet)
        {
            log_event("=== Starting event-driven simulation ===");
            log_event("Fleet size: " + std::to_string(fleet.size()));
            log_event("Available chargers: " + std::to_string(charger_mgr.get_available_chargers()));
            
            schedule_initial_flights(fleet);

            // process events
            while (!event_queue_.empty() && current_time_hours_ < simulation_duration_hours_)
            {
                auto event = event_queue_.top();
                event_queue_.pop();

                current_time_hours_ = event.time_hours;

                if (current_time_hours_ >= simulation_duration_hours_)
                {
                    log_event("Simulation time limit reached");
                    break;
                }

                process_event(event, charger_mgr, fleet);
            }

            // Process any remaining activities at simulation end
            log_event("=== Finalizing simulation ===");
            finalize_simulation(fleet);
            log_event("=== Simulation completed ===");
        }

        template <typename Fleet>
        void process_event(const SimulationEvent &event, ChargerManager &charger_mgr, Fleet &fleet)
        {
            std::visit([&](const auto &data)
                       {
            using T = std::decay_t<decltype(data)>;
            
            if constexpr (std::is_same_v<T, FlightCompleteData>) {
                log_event("Processing FLIGHT_COMPLETE event for aircraft " + std::to_string(data.aircraft_id));
                handle_flight_complete(data, charger_mgr, fleet);
            } else if constexpr (std::is_same_v<T, ChargingCompleteData>) {
                log_event("Processing CHARGING_COMPLETE event for aircraft " + std::to_string(data.aircraft_id));
                handle_charging_complete(data, charger_mgr, fleet);
            } else if constexpr (std::is_same_v<T, FaultData>) {
                log_event("Processing FAULT_OCCURRED event for aircraft " + std::to_string(data.aircraft_id));
                handle_fault(data, fleet);
            } }, event.data);
        }

        void schedule_event(EventType type, double time_hours, EventData data)
        {
            double scheduled_time = time_hours;
            bool is_partial_event = false;
            
            // For flight events that would extend beyond simulation duration,
            // schedule them at simulation end time as partial events (if enabled)
            if (time_hours > simulation_duration_hours_ && type == EventType::FLIGHT_COMPLETE && enable_partial_flights_)
            {
                scheduled_time = simulation_duration_hours_;
                is_partial_event = true;
            }
            
            // For charging events that would extend beyond simulation duration,
            // schedule them at simulation end time as partial events (if enabled)
            if (time_hours > simulation_duration_hours_ && type == EventType::CHARGING_COMPLETE && enable_partial_flights_)
            {
                scheduled_time = simulation_duration_hours_;
                is_partial_event = true;
            }
            
            // Only schedule if within simulation duration or it's a partial event
            if (scheduled_time <= simulation_duration_hours_)
            {
                std::string event_type_str;
                switch (type) {
                    case EventType::FLIGHT_COMPLETE: event_type_str = "FLIGHT_COMPLETE"; break;
                    case EventType::CHARGING_COMPLETE: event_type_str = "CHARGING_COMPLETE"; break;
                    case EventType::FAULT_OCCURRED: event_type_str = "FAULT_OCCURRED"; break;
                }
                
                int aircraft_id = std::visit([](const auto &data) -> int {
                    using T = std::decay_t<decltype(data)>;
                    if constexpr (std::is_same_v<T, FlightCompleteData>) {
                        return data.aircraft_id;
                    } else if constexpr (std::is_same_v<T, ChargingCompleteData>) {
                        return data.aircraft_id;
                    } else if constexpr (std::is_same_v<T, FaultData>) {
                        return data.aircraft_id;
                    }
                    return -1;
                }, data);
                
                if (is_partial_event)
                {
                    log_event("Scheduled partial " + event_type_str + " event for aircraft " + std::to_string(aircraft_id) + 
                             " at time " + std::to_string(scheduled_time) + "h (originally " + std::to_string(time_hours) + "h)");
                }
                else
                {
                    log_event("Scheduled " + event_type_str + " event for aircraft " + std::to_string(aircraft_id) + " at time " + std::to_string(scheduled_time) + "h");
                }
                
                event_queue_.emplace(type, scheduled_time, data);
            }
        }

        double get_current_time() const { return current_time_hours_; }
        double get_duration() const { return simulation_duration_hours_; }

    private:
        template <typename Fleet>
        void schedule_initial_flights(Fleet &fleet)
        {
            for (auto &aircraft : fleet)
            {
                schedule_flight(aircraft.get());
            }
        }

        template <typename Aircraft>
        void schedule_flight(Aircraft *aircraft)
        {
            double distance = aircraft->get_flight_distance_miles();
            double flight_time = aircraft->get_flight_time_hours();

            log_event("Starting flight for aircraft " + std::to_string(aircraft->get_id()) + 
                     " (distance: " + std::to_string(distance) + " miles, flight time: " + 
                     std::to_string(flight_time) + "h)");

            // Record flight start time
            flight_start_times_[aircraft->get_id()] = current_time_hours_;

            double fault_time = aircraft->check_fault_during_flight(flight_time);
            bool fault_occurred = (fault_time >= 0.0);

            if (fault_occurred) {
                log_event("Aircraft " + std::to_string(aircraft->get_id()) + " will experience fault at " + 
                         std::to_string(fault_time) + "h into flight");
                FaultData fault_data{
                    aircraft->get_id(),
                    fault_time};
                schedule_event(EventType::FAULT_OCCURRED, current_time_hours_ + fault_time, fault_data);
            }

            FlightCompleteData flight_data{
                aircraft->get_id(),
                flight_time,
                distance,
                fault_occurred};

            schedule_event(EventType::FLIGHT_COMPLETE, current_time_hours_ + flight_time, flight_data);
        }

        template <typename Fleet>
        void handle_flight_complete(const FlightCompleteData &data, ChargerManager &charger_mgr, Fleet &fleet)
        {
            auto aircraft_it = std::find_if(fleet.begin(), fleet.end(),
                                            [&](const auto &aircraft)
                                            { return aircraft->get_id() == data.aircraft_id; });

            if (aircraft_it != fleet.end())
            {
                auto &aircraft = *aircraft_it;

                log_event("Aircraft " + std::to_string(data.aircraft_id) + " completed flight (" + 
                         std::to_string(data.distance) + " miles, " + std::to_string(data.flight_time) + "h)");

                aircraft->discharge_battery();

                stats_collector_.record_flight(aircraft->get_type(), data.flight_time,
                                               data.distance, aircraft->get_passenger_count());

                // Clean up flight start time tracking
                flight_start_times_.erase(data.aircraft_id);

                if (!aircraft->is_faulty())
                {
                    if (charger_mgr.request_charger(aircraft->get_id()))
                    {
                        log_event("Aircraft " + std::to_string(data.aircraft_id) + " assigned to charger immediately");
                        schedule_charging(aircraft.get(), 0.0);
                    }
                    else
                    {
                        log_event("Aircraft " + std::to_string(data.aircraft_id) + " added to charging queue (no chargers available)");
                        charger_mgr.add_to_queue(aircraft->get_id());
                        waiting_start_times_[aircraft->get_id()] = current_time_hours_;
                    }
                }
                else
                {
                    log_event("Aircraft " + std::to_string(data.aircraft_id) + " is faulty - not scheduling charging");
                }
            }
        }

        template <typename Fleet>
        void handle_charging_complete(const ChargingCompleteData &data, ChargerManager &charger_mgr, Fleet &fleet)
        {
            auto aircraft_it = std::find_if(fleet.begin(), fleet.end(),
                                            [&](const auto &aircraft)
                                            { return aircraft->get_id() == data.aircraft_id; });

            if (aircraft_it != fleet.end())
            {
                auto &aircraft = *aircraft_it;

                log_event("Aircraft " + std::to_string(data.aircraft_id) + " completed charging (" + 
                         std::to_string(data.charge_time) + "h charge, " + std::to_string(data.waiting_time) + "h wait)");

                aircraft->charge_battery();

                stats_collector_.record_charge_session(aircraft->get_type(), data.charge_time, data.waiting_time);

                // Clean up charging start time tracking
                charging_start_times_.erase(data.aircraft_id);

                if (current_time_hours_ < simulation_duration_hours_ && !aircraft->is_faulty())
                {
                    log_event("Aircraft " + std::to_string(data.aircraft_id) + " ready for next flight");
                    schedule_flight(aircraft.get());
                }
                else if (current_time_hours_ >= simulation_duration_hours_)
                {
                    log_event("Aircraft " + std::to_string(data.aircraft_id) + " charging complete but simulation time exceeded");
                }

                // start charging any waiting aircraft
                int next_aircraft_id = charger_mgr.get_next_from_queue();
                if (next_aircraft_id != -1)
                {
                    auto next_aircraft_it = fleet.end();
                    for (auto it = fleet.begin(); it != fleet.end(); ++it)
                    {
                        if ((*it)->get_id() == next_aircraft_id)
                        {
                            next_aircraft_it = it;
                            break;
                        }
                    }

                    if (next_aircraft_it != fleet.end())
                    {
                        charger_mgr.assign_charger(next_aircraft_id);
                        
                        double waiting_time = 0.0;
                        auto waiting_it = waiting_start_times_.find(next_aircraft_id);
                        if (waiting_it != waiting_start_times_.end())
                        {
                            waiting_time = current_time_hours_ - waiting_it->second;
                            waiting_start_times_.erase(waiting_it);
                        }
                        
                        log_event("Aircraft " + std::to_string(next_aircraft_id) + " removed from queue and assigned charger (waited " + 
                                 std::to_string(waiting_time) + "h)");
                        schedule_charging(next_aircraft_it->get(), waiting_time);
                    }
                }
                else
                {
                    log_event("Charger freed but no aircraft waiting in queue");
                }
            }
        }

        template <typename Fleet>
        void handle_fault(const FaultData &data, Fleet &fleet)
        {
            auto aircraft_it = std::find_if(fleet.begin(), fleet.end(),
                                            [&](const auto &aircraft)
                                            { return aircraft->get_id() == data.aircraft_id; });

            if (aircraft_it != fleet.end())
            {
                auto &aircraft = *aircraft_it;
                log_event("Aircraft " + std::to_string(data.aircraft_id) + " experienced fault during flight - aircraft grounded");
                aircraft->set_faulty(true);
                stats_collector_.record_fault(aircraft->get_type());
            }
        }

        template <typename Aircraft>
        void schedule_charging(Aircraft *aircraft, double waiting_time)
        {
            double charge_time = aircraft->get_charge_time_hours();

            log_event("Starting charging for aircraft " + std::to_string(aircraft->get_id()) + 
                     " (charge time: " + std::to_string(charge_time) + "h, waited: " + 
                     std::to_string(waiting_time) + "h)");

            // Record charging start time
            charging_start_times_[aircraft->get_id()] = current_time_hours_;

            ChargingCompleteData charge_data{
                aircraft->get_id(),
                charge_time,
                waiting_time};

            schedule_event(EventType::CHARGING_COMPLETE,
                           current_time_hours_ + charge_time,
                           charge_data);
        }

        template <typename Fleet>
        void finalize_simulation(Fleet &fleet)
        {
            // Set current time to simulation end for partial calculation
            current_time_hours_ = simulation_duration_hours_;

            // Process remaining unprocessed events to record partial activities
            while (!event_queue_.empty())
            {
                auto event = event_queue_.top();
                event_queue_.pop();

                std::visit([&](const auto &data)
                {
                    using T = std::decay_t<decltype(data)>;
                    
                    if constexpr (std::is_same_v<T, FlightCompleteData>) {
                        handle_partial_flight(data, fleet);
                    } else if constexpr (std::is_same_v<T, ChargingCompleteData>) {
                        handle_partial_charge(data, fleet);
                    }
                }, event.data);
            }
        }

        template <typename Fleet>
        void handle_partial_flight(const FlightCompleteData &data, Fleet &fleet)
        {
            auto aircraft_it = std::find_if(fleet.begin(), fleet.end(),
                                            [&](const auto &aircraft)
                                            { return aircraft->get_id() == data.aircraft_id; });

            if (aircraft_it != fleet.end())
            {
                auto &aircraft = *aircraft_it;
                
                // Find flight start time
                auto start_it = flight_start_times_.find(data.aircraft_id);
                if (start_it != flight_start_times_.end())
                {
                    double flight_start_time = start_it->second;
                    double partial_flight_time = simulation_duration_hours_ - flight_start_time;
                    
                    // Calculate partial distance based on partial flight time
                    double partial_distance = (partial_flight_time / data.flight_time) * data.distance;
                    
                    if (enable_detailed_logging_)
                    {
                        log_event("Processing partial flight for aircraft " + std::to_string(data.aircraft_id) + 
                                 " (flew " + std::to_string(partial_flight_time) + "h/" + std::to_string(data.flight_time) + 
                                 "h, " + std::to_string(partial_distance) + "/" + std::to_string(data.distance) + " miles)");
                    }
                    
                    stats_collector_.record_partial_flight(aircraft->get_type(), partial_flight_time,
                                                          partial_distance, aircraft->get_passenger_count());
                }
            }
        }

        template <typename Fleet>
        void handle_partial_charge(const ChargingCompleteData &data, Fleet &fleet)
        {
            auto aircraft_it = std::find_if(fleet.begin(), fleet.end(),
                                            [&](const auto &aircraft)
                                            { return aircraft->get_id() == data.aircraft_id; });

            if (aircraft_it != fleet.end())
            {
                auto &aircraft = *aircraft_it;
                
                // Find charging start time
                auto start_it = charging_start_times_.find(data.aircraft_id);
                if (start_it != charging_start_times_.end())
                {
                    double charge_start_time = start_it->second;
                    double partial_charge_time = simulation_duration_hours_ - charge_start_time;
                    
                    if (enable_detailed_logging_)
                    {
                        log_event("Processing partial charge for aircraft " + std::to_string(data.aircraft_id) + 
                                 " (charged " + std::to_string(partial_charge_time) + "h/" + std::to_string(data.charge_time) + 
                                 "h, waited: " + std::to_string(data.waiting_time) + "h)");
                    }
                    
                    stats_collector_.record_partial_charge(aircraft->get_type(), partial_charge_time);
                }
            }
        }
    };

    /**
     * Event-driven simulation engine with complete simulation logic
     * This provides a consistent interface for the simulation factory and runner
     */
    class EventDrivenSimulationEngine : public SimulationEngineBase
    {
    private:
        std::unique_ptr<EventDrivenSimulation> simulation_;

    public:
        EventDrivenSimulationEngine(StatisticsCollector &stats, double duration_hours = 3.0, bool detailed_logging = false, bool partial_flights = true)
            : SimulationEngineBase(stats, duration_hours), 
              simulation_(std::make_unique<EventDrivenSimulation>(stats, duration_hours, detailed_logging, partial_flights))
        {
        }

    protected:
        void run_simulation_impl(ChargerManager &charger_mgr, void *fleet_ptr) override
        {
            is_running_ = true;

            // Type-erase back to template - this is a limitation of the current design
            // In a real implementation, we might use std::function or type erasure
            // For now, we'll use a simple approach
            auto *fleet = static_cast<std::vector<std::unique_ptr<AircraftBase>> *>(fleet_ptr);
            simulation_->run_simulation(charger_mgr, *fleet);

            // Update our time tracking from the simulation
            current_time_hours_ = simulation_->get_current_time();
            
            is_running_ = false;
        }
    };
}