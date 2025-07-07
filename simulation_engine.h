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

    struct Event
    {
        EventType type;
        double time_hours;
        FlightCompleteData flight_data;
        Event(EventType t, double time, FlightCompleteData fd) : type(t), time_hours(time), flight_data(std::move(fd)) {}

        bool operator<(const Event &other) const
        {
            return time_hours > other.time_hours;
        }
    };

    struct FaultData
    {
        int aircraft_id;
        double fault_time;
    };

    class SimulationEngine
    {
    private:
        std::priority_queue<Event> event_queue_;
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
        void process_event(const Event &event, Fleet &fleet)
        {
            auto aircraft_it = fleet.end();
            for (auto it = fleet.begin(); it != fleet.end(); ++it)
            {
                if ((*it)->get_id() == event.flight_data.aircraft_id)
                {
                    aircraft_it = it;
                    break;
                }
            }
            switch (event.type)
            {
            case EventType::FLIGHT_COMPLETE:
                if (aircraft_it != fleet.end())
                {
                    auto &aircraft = *aircraft_it;

                    aircraft->discharge_battery();

                    if (event.flight_data.fault_occurred)
                    {
                        std::cout << "Fault occurred for aircraft ID: " << aircraft->get_id() << "\n";
                    }
                    // TODO need to handle charging queue or manager here
                    // for now just have infinite chargers
                    schedule_charging(aircraft.get_id());
                }
                break;
            case EventType::CHARGING_COMPLETE:
                // handle charging started event here
                // e.g., handle_charging_started(...);
                break;
            case EventType::FAULT_OCCURRED:
                // handle fault event here
                // e.g., handle_fault(...);
                break;
            default:
                // unknown event type
                break;
            }
        }

        void schedule_event(EventType type, double time_hours, FlightCompleteData flight_data)
        {
            if (time_hours <= simulation_duration_hours_)
            {
                event_queue_.emplace(type, time_hours, flight_data);
            }
        }

        void print_state()
        {
            std::cout << "Current simulation time: " << current_time_hours_ << " hours\n";
            std::cout << "Simulation duration: " << simulation_duration_hours_ << " hours\n";
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