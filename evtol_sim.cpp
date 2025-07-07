#include <iostream>
#include <iomanip>
#include <chrono>

#include "aircraft.h"
#include "aircraft_types.h"

using namespace std;
using namespace evtol;

class SimulationRunner
{
private:
    static constexpr int FLEET_SIZE = 20;
    static constexpr int NUM_CHARGERS = 3;
    static constexpr double SIMULATION_DURATION_HOURS = 3.0;

    std::vector<std::unique_ptr<AircraftBase>> fleet_;

public:
    SimulationRunner()
    {
        initialize_simulation();
    }

    void run_simulation()
    {
        cout << "========== eVTOL Aircraft Simulation ==========\n";
        cout << "Fleet Size: " << FLEET_SIZE << " aircraft\n";
        cout << "Chargers Available: " << NUM_CHARGERS << "\n";
        cout << "Simulation Duration: " << SIMULATION_DURATION_HOURS << " hours\n";
        cout << "Starting simulation...\n\n";

        auto start = chrono::high_resolution_clock::now();

        // TODO Implement Simulation
        for (size_t i = 0; i < fleet_.size(); ++i)
        {
            cout << "Aircraft " << i + 1 << ": " << fleet_[i]->get_manufacturer() << endl;
        }

        auto end = chrono::high_resolution_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);

        cout << "Simulation completed in " << elapsed.count() << " ms\n\n";

        // TODO: Print statistics
    }

private:
    void initialize_simulation()
    {
        fleet_ = AircraftFactory<>::create_fleet(FLEET_SIZE);
    }

    void display_results()
    {
        cout << "Simulation Results" << endl;
    }
};

int main()
{
    try
    {
        SimulationRunner simulation;
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
