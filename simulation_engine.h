#pragma once
#include <vector>
#include <memory>
#include <chrono>

namespace evtol
{

    enum class EventType
    {
        FLIGHT_COMPLETE,
        CHARGING_STARTEDƒ,
        FAULT_OCCURRED
    };

    struct FlightCompleteData
    {
        int aircraft_id;
        double flight_time;
        double distance;
        bool fault_occurred;
    };

    struct FaultData
    {
        int aircraft_id;
        double fault_time;
    };

    class SimulationEngine
    {
    private:
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
                start_flight(aircraft.get());
            }
        }

        template <typename Aircraft>
        void start_flight(Aircraft *aircraft)
        {
            double distance = aircraft->get_flight_distance_miles();
            std::cout << "Starting flight for aircraft ID: " << aircraft->get_id()
                      << " with distance: " << distance << " miles\n";

            double flight_time = aircraft->get_flight_time_hours();
            std::cout << "Flight time: " << flight_time << " hours\n";
        }
    };

}