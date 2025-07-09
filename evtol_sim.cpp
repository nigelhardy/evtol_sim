#include <iostream>
#include <iomanip>
#include <chrono>

#include "aircraft.h"
#include "aircraft_types.h"
#include "simulation_runner.h"
#include "simulation_config.h"

using namespace std;
using namespace evtol;

template <typename Duration>
class PerformanceTimer
{
private:
    std::chrono::high_resolution_clock::time_point start_time_;

public:
    PerformanceTimer() : start_time_(std::chrono::high_resolution_clock::now()) {}

    auto elapsed() const
    {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<Duration>(end_time - start_time_);
    }

    void reset()
    {
        start_time_ = std::chrono::high_resolution_clock::now();
    }
};

class EvtolSimulationApp
{
private:
    static constexpr int FLEET_SIZE = 20;
    static constexpr int NUM_CHARGERS = 3;
    static constexpr double SIMULATION_DURATION_HOURS = 3.0;

    std::vector<std::unique_ptr<AircraftBase>> fleet_;
    std::unique_ptr<StatisticsCollector> stats_collector_;
    std::unique_ptr<evtol::SimulationRunner> sim_runner_;
    ChargerManager charger_manager_;
    SimulationConfig config_;

public:
    EvtolSimulationApp(int argc, char *argv[])
    {
        initialize_configuration(argc, argv);
        initialize_simulation();
    }

    void run_simulation()
    {
        cout << "========== eVTOL Aircraft Simulation ==========\n";
        cout << "Fleet Size: " << FLEET_SIZE << " aircraft\n";
        cout << "Chargers Available: " << NUM_CHARGERS << "\n";
        cout << "Simulation Duration: " << config_.simulation_duration_hours << " hours\n";
        cout << "Mode: " << (config_.mode == SimulationMode::FRAME_BASED ? "Frame-Based" : "Event-Driven") << "\n";

        if (config_.mode == SimulationMode::FRAME_BASED)
        {
            cout << "Frame Time: " << config_.frame_time_seconds << " seconds\n";
        }

        cout << "Starting simulation...\n\n";

        PerformanceTimer<std::chrono::microseconds> timer;

        sim_runner_->run_simulation(charger_manager_, fleet_);

        auto elapsed = timer.elapsed();

        cout << "Simulation completed in " << elapsed.count() << " microseconds ("
             << std::fixed << std::setprecision(3) << elapsed.count() / 1000.0 << " ms)\n\n";

        display_results();
    }

private:
    void initialize_configuration(int argc, char *argv[])
    {
        // use default 3.0 duration as specified in problem statement
        config_.simulation_duration_hours = SIMULATION_DURATION_HOURS;
        config_.parse_args(argc, argv);

        if (!config_.validate())
        {
            throw std::runtime_error("Invalid configuration");
        }
    }

    void initialize_simulation()
    {
        fleet_ = AircraftFactory<>::create_fleet(FLEET_SIZE);
        stats_collector_ = std::make_unique<StatisticsCollector>();

        // Set aircraft counts for proper reporting
        stats_collector_->set_aircraft_counts(fleet_);

        sim_runner_ = std::make_unique<evtol::SimulationRunner>(*stats_collector_, config_);
    }

    void display_results()
    {
        cout << stats_collector_->generate_report(config_.enable_detailed_logging);
    }

    void display_performance_metrics()
    {
        auto summary = stats_collector_->get_summary_stats();

        cout << "========== Summary Statistics ==========\n";
        cout << "Total Flight Time: " << summary.total_flight_time << " hours\n";
        cout << "Total Distance: " << summary.total_distance << " miles\n";
        cout << "Total Charging Time: " << summary.total_charging_time << " hours\n";
        cout << "Total Faults: " << summary.total_faults << "\n";
        cout << "Total Passenger Miles: " << summary.total_passenger_miles << "\n";
        cout << "Total Flights: " << summary.total_flights << "\n";
        cout << "Total Charge Sessions: " << summary.total_charges << "\n\n";
    }
};

int main(int argc, char *argv[])
{
    try
    {
        EvtolSimulationApp simulation(argc, argv);
        simulation.run_simulation();

        return 0;
    }
    catch (const std::exception &e)
    {
        cerr << "Simulation error: " << e.what() << endl;
        return 1;
    }
    catch (...)
    {
        cerr << "Unknown error occurred during simulation" << endl;
        return 1;
    }
}
