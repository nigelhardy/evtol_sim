#pragma once
#include <queue>
#include <vector>
#include <memory>
#include <chrono>
#include <variant>

#include "charger_manager.h"
#include "statistics_engine.h"

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
    };

    struct FaultData
    {
        int aircraft_id;
        double fault_time;
    };

    using EventData = std::variant<FlightCompleteData, ChargingCompleteData, FaultData>;
    using SimulationEvent = Event<EventData>;

    class SimulationEngine
    {
    private:
        std::priority_queue<SimulationEvent> event_queue_;
        double current_time_hours_;
        double simulation_duration_hours_;
        StatisticsCollector &stats_collector_;

    public:
        SimulationEngine(StatisticsCollector &stats, double duration_hours = 3.0)
            : current_time_hours_(0.0), simulation_duration_hours_(duration_hours),
              stats_collector_(stats)
        {
        }

        template <typename Fleet>
        void run_simulation(ChargerManager &charger_mgr, Fleet &fleet)
        {
            schedule_initial_flights(fleet);

            // process events
            while (!event_queue_.empty() && current_time_hours_ < simulation_duration_hours_)
            {
                auto event = event_queue_.top();
                event_queue_.pop();

                current_time_hours_ = event.time_hours;

                if (current_time_hours_ >= simulation_duration_hours_)
                {
                    break;
                }

                process_event(event, charger_mgr, fleet);
            }
        }
        template <typename Fleet>
        void process_event(const SimulationEvent &event, ChargerManager &charger_mgr, Fleet &fleet)
        {
            std::visit([&](const auto &data)
                       {
            using T = std::decay_t<decltype(data)>;
            
            if constexpr (std::is_same_v<T, FlightCompleteData>) {
                handle_flight_complete(data, charger_mgr, fleet);
            } else if constexpr (std::is_same_v<T, ChargingCompleteData>) {
                handle_charging_complete(data, charger_mgr, fleet);
            } else if constexpr (std::is_same_v<T, FaultData>) {
                handle_fault(data, fleet);
            } }, event.data);
        }

        void schedule_event(EventType type, double time_hours, EventData data)
        {
            if (time_hours <= simulation_duration_hours_)
            {
                event_queue_.emplace(type, time_hours, data);
            }
        }

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

            bool fault_occurred = aircraft->check_fault_during_flight(flight_time);

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

                aircraft->discharge_battery();

                stats_collector_.record_flight(aircraft->get_type(), data.flight_time,
                                               data.distance, aircraft->get_passenger_count());
                if (data.fault_occurred)
                {
                    stats_collector_.record_fault(aircraft->get_type());
                }

                if (charger_mgr.request_charger(aircraft->get_id()))
                {
                    schedule_charging(aircraft.get());
                }
                else
                {
                    charger_mgr.add_to_queue(aircraft->get_id());
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

                aircraft->charge_battery();

                stats_collector_.record_charge_session(aircraft->get_type(), data.charge_time);

                if (current_time_hours_ < simulation_duration_hours_)
                {
                    schedule_flight(aircraft.get());
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
                        schedule_charging(next_aircraft_it->get());
                    }
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
                stats_collector_.record_fault((*aircraft_it)->get_type());
            }
        }

        template <typename Aircraft>
        void schedule_charging(Aircraft *aircraft)
        {
            double charge_time = aircraft->get_charge_time_hours();

            ChargingCompleteData charge_data{
                aircraft->get_id(),
                charge_time};

            schedule_event(EventType::CHARGING_COMPLETE,
                           current_time_hours_ + charge_time,
                           charge_data);
        }
    };
}