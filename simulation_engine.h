#pragma once
#include <queue>
#include <vector>
#include <memory>
#include <chrono>

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

    public:
        SimulationEngine(double duration_hours = 3.0)
            : current_time_hours_(0.0), simulation_duration_hours_(duration_hours)
        {
        }

        template <typename Fleet>
        void run_simulation(Fleet &fleet)
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

                process_event(event, fleet);
            }
        }
        template <typename Fleet>
        void process_event(const SimulationEvent &event, Fleet &fleet)
        {
            std::visit([&](const auto &data)
                       {
            using T = std::decay_t<decltype(data)>;
            
            if constexpr (std::is_same_v<T, FlightCompleteData>) {
                handle_flight_complete(data, fleet);
            } else if constexpr (std::is_same_v<T, ChargingCompleteData>) {
                handle_charging_complete(data, fleet);
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
            // Adding to quiet the warning about unused variable for now
            std::cout << "Scheduling flight for aircraft ID: " << aircraft->get_id()
                      << " with distance: " << distance << " miles and flight time: "
                      << flight_time << " hours. Fault occurred: "
                      << (fault_occurred ? "yes" : "no") << "\n";
            schedule_event(EventType::FLIGHT_COMPLETE, current_time_hours_ + flight_time, flight_data);
        }

        template <typename Fleet>
        void handle_flight_complete(const FlightCompleteData &data, Fleet &fleet)
        {
            auto aircraft_it = std::find_if(fleet.begin(), fleet.end(),
                                            [&](const auto &aircraft)
                                            { return aircraft->get_id() == data.aircraft_id; });

            if (aircraft_it != fleet.end())
            {
                auto &aircraft = *aircraft_it;

                aircraft->discharge_battery();

                schedule_charging(aircraft.get());
            }
        }

        template <typename Fleet>
        void handle_charging_complete(const ChargingCompleteData &data, Fleet &fleet)
        {
            auto aircraft_it = std::find_if(fleet.begin(), fleet.end(),
                                            [&](const auto &aircraft)
                                            { return aircraft->get_id() == data.aircraft_id; });

            if (aircraft_it != fleet.end())
            {
                auto &aircraft = *aircraft_it;

                aircraft->charge_battery();

                if (current_time_hours_ < simulation_duration_hours_)
                {
                    schedule_flight(aircraft.get());
                }
            }
        }

        template <typename Fleet>
        void handle_fault(const FaultData &data, Fleet &fleet)
        {
            auto aircraft_it = std::find_if(fleet.begin(), fleet.end(),
                                            [&](const auto &aircraft)
                                            { return aircraft->get_id() == data.aircraft_id; });

            std::cout << "Handling fault for aircraft ID: " << data.aircraft_id << "\n";

            if (aircraft_it != fleet.end())
            {
                // Record the fault occurrence
                std::cout << "Fault occurred for aircraft ID: " << data.aircraft_id << "\n";
                // Here you would typically log the fault or take necessary actions
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