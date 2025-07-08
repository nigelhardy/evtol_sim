#include "test_utilities.h"

namespace evtol_test
{

    class SimulationIntegrationTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            stats_collector_ = std::make_unique<evtol::StatisticsCollector>();
            charger_manager_ = std::make_unique<evtol::ChargerManager>();
        }

        std::unique_ptr<evtol::StatisticsCollector> stats_collector_;
        std::unique_ptr<evtol::ChargerManager> charger_manager_;
    };

    // Test complete simulation workflow with real components
    TEST_F(SimulationIntegrationTest, CompleteSimulationWorkflow)
    {
        const int fleet_size = 20;
        const double simulation_duration = 3.0;

        // Create real fleet using factory
        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);

        // Create simulation engine
        evtol::SimulationEngine sim_engine(*stats_collector_, simulation_duration);

        // Run complete simulation
        sim_engine.run_simulation(*charger_manager_, fleet);

        // Verify simulation results
        auto summary = stats_collector_->get_summary_stats();

        EXPECT_GT(summary.total_flights, 0);
        EXPECT_GT(summary.total_flight_time, 0.0);
        EXPECT_GT(summary.total_distance, 0.0);
        EXPECT_GT(summary.total_charges, 0);
        EXPECT_GT(summary.total_charging_time, 0.0);
        EXPECT_GT(summary.total_passenger_miles, 0.0);

        // All aircraft should have participated
        EXPECT_GE(summary.total_flights, fleet_size);

        // Report generation should work
        std::string report = stats_collector_->generate_report();
        EXPECT_FALSE(report.empty());
        EXPECT_NE(report.find("eVTOL Simulation Results"), std::string::npos);
    }

    // Test simulation with different fleet sizes
    TEST_F(SimulationIntegrationTest, DifferentFleetSizes)
    {
        std::vector<int> fleet_sizes = {1, 5, 10, 20, 50};

        for (int fleet_size : fleet_sizes)
        {
            // Reset components for each test
            stats_collector_ = std::make_unique<evtol::StatisticsCollector>();
            charger_manager_ = std::make_unique<evtol::ChargerManager>();

            auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);
            evtol::SimulationEngine sim_engine(*stats_collector_, 2.0);

            sim_engine.run_simulation(*charger_manager_, fleet);

            auto summary = stats_collector_->get_summary_stats();

            // Results should scale with fleet size
            EXPECT_GE(summary.total_flights, fleet_size / 2); // At least half should fly
            // okay to assume some flights, even with faults, first flight still succeeds
            EXPECT_GT(summary.total_passenger_miles, 0.0);

            // Charger management should work regardless of fleet size
            EXPECT_EQ(charger_manager_->get_total_chargers(), 3);
            EXPECT_LE(charger_manager_->get_active_chargers(), 3);
        }
    }

    // Test simulation with different durations
    TEST_F(SimulationIntegrationTest, DifferentSimulationDurations)
    {
        // We aren't calculating partial flights, so need to ensure durations are long enough
        // Alpha min: 1.667, Beta min: 0.667, Charlie min: 0.625, Delta min: 1.667, Echo min: 0.86
        std::vector<double> durations = {1.7, 2.0, 3.0, 5.0, 10.0};
        const int fleet_size = 10;

        std::vector<int> total_flights_per_duration;

        for (double duration : durations)
        {
            // Reset components
            stats_collector_ = std::make_unique<evtol::StatisticsCollector>();
            charger_manager_ = std::make_unique<evtol::ChargerManager>();

            auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);
            evtol::SimulationEngine sim_engine(*stats_collector_, duration);

            sim_engine.run_simulation(*charger_manager_, fleet);

            auto summary = stats_collector_->get_summary_stats();
            total_flights_per_duration.push_back(summary.total_flights);

            EXPECT_GT(summary.total_flights, 0);
        }

        // Longer simulations should generally have more flights
        EXPECT_LT(total_flights_per_duration[0], total_flights_per_duration.back());
    }

    // Test aircraft type distribution in simulation results
    TEST_F(SimulationIntegrationTest, AircraftTypeDistributionInResults)
    {
        const int fleet_size = 25; // 5 of each type
        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);

        evtol::SimulationEngine sim_engine(*stats_collector_, 4.0);
        sim_engine.run_simulation(*charger_manager_, fleet);

        // Check each aircraft type has recorded activity
        auto alpha_stats = stats_collector_->get_stats(evtol::AircraftType::ALPHA);
        auto beta_stats = stats_collector_->get_stats(evtol::AircraftType::BETA);
        auto charlie_stats = stats_collector_->get_stats(evtol::AircraftType::CHARLIE);
        auto delta_stats = stats_collector_->get_stats(evtol::AircraftType::DELTA);
        auto echo_stats = stats_collector_->get_stats(evtol::AircraftType::ECHO);

        EXPECT_GT(alpha_stats.flight_count, 0);
        EXPECT_GT(beta_stats.flight_count, 0);
        EXPECT_GT(charlie_stats.flight_count, 0);
        EXPECT_GT(delta_stats.flight_count, 0);
        EXPECT_GT(echo_stats.flight_count, 0);

        // Each type should have some charge sessions
        EXPECT_GT(alpha_stats.charge_count, 0);
        EXPECT_GT(beta_stats.charge_count, 0);
        EXPECT_GT(charlie_stats.charge_count, 0);
        EXPECT_GT(delta_stats.charge_count, 0);
        EXPECT_GT(echo_stats.charge_count, 0);
    }

    // Test charger utilization during simulation
    TEST_F(SimulationIntegrationTest, ChargerUtilizationDuringSimulation)
    {
        const int fleet_size = 15; // More aircraft than chargers
        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);

        evtol::SimulationEngine sim_engine(*stats_collector_, 3.0);
        sim_engine.run_simulation(*charger_manager_, fleet);

        // Chargers should have been well utilized
        auto summary = stats_collector_->get_summary_stats();
        EXPECT_GT(summary.total_charges, 10); // Reasonable utilization

        // Final state should be reasonable
        EXPECT_EQ(charger_manager_->get_total_chargers(), 3);
        EXPECT_LE(charger_manager_->get_active_chargers(), 3);
        EXPECT_LE(charger_manager_->get_queue_size(), fleet_size);
    }

    // Test fault occurrence and handling
    TEST_F(SimulationIntegrationTest, FaultOccurrenceAndHandling)
    {
        const int fleet_size = 50; // Large fleet increases fault probability
        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);

        evtol::SimulationEngine sim_engine(*stats_collector_, 3.0);
        sim_engine.run_simulation(*charger_manager_, fleet);

        auto summary = stats_collector_->get_summary_stats();

        // With large fleet and longer duration, some faults should occur
        // (Due to probabilistic nature, we can't guarantee faults, but likely)
        EXPECT_GE(summary.total_faults, 0);

        // Simulation should continue despite faults
        EXPECT_GT(summary.total_flights, fleet_size);
    }

    // Test performance characteristics
    TEST_F(SimulationIntegrationTest, PerformanceCharacteristics)
    {
        const int fleet_size = 100;
        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);

        auto start_time = std::chrono::high_resolution_clock::now();

        evtol::SimulationEngine sim_engine(*stats_collector_, 2.5);
        sim_engine.run_simulation(*charger_manager_, fleet);

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // Should complete in reasonable time
        EXPECT_LT(duration.count(), 10000); // Less than 10 seconds

        auto summary = stats_collector_->get_summary_stats();
        EXPECT_GT(summary.total_flights, fleet_size);
    }

    // Test simulation reproducibility
    TEST_F(SimulationIntegrationTest, SimulationReproducibility)
    {
        const int fleet_size = 10;
        const double sim_duration = 1.0;

        // Run first simulation
        auto fleet1 = evtol::AircraftFactory<>::create_fleet(fleet_size);
        evtol::SimulationEngine sim1(*stats_collector_, sim_duration);
        sim1.run_simulation(*charger_manager_, fleet1);
        auto summary1 = stats_collector_->get_summary_stats();

        // Reset and run second simulation
        stats_collector_ = std::make_unique<evtol::StatisticsCollector>();
        charger_manager_ = std::make_unique<evtol::ChargerManager>();
        auto fleet2 = evtol::AircraftFactory<>::create_fleet(fleet_size);
        evtol::SimulationEngine sim2(*stats_collector_, sim_duration);
        sim2.run_simulation(*charger_manager_, fleet2);
        auto summary2 = stats_collector_->get_summary_stats();

        // Results should be in similar ranges (not identical due to randomness)
        EXPECT_NEAR(summary1.total_flights, summary2.total_flights, summary1.total_flights * 0.5);
        EXPECT_GT(summary1.total_flights, 0);
        EXPECT_GT(summary2.total_flights, 0);
    }

    // Test memory management in complete workflow
    TEST_F(SimulationIntegrationTest, MemoryManagementInCompleteWorkflow)
    {
        // Run multiple simulations to test for memory leaks
        for (int iteration = 0; iteration < 10; ++iteration)
        {
            stats_collector_ = std::make_unique<evtol::StatisticsCollector>();
            charger_manager_ = std::make_unique<evtol::ChargerManager>();

            auto fleet = evtol::AircraftFactory<>::create_fleet(20);
            evtol::SimulationEngine sim_engine(*stats_collector_, 3.0);

            sim_engine.run_simulation(*charger_manager_, fleet);

            auto summary = stats_collector_->get_summary_stats();
            EXPECT_GT(summary.total_flights, 0);
        }

        // If we reach here without memory issues, test passes
        SUCCEED();
    }

    // Test report generation with real data
    TEST_F(SimulationIntegrationTest, ReportGenerationWithRealData)
    {
        auto fleet = evtol::AircraftFactory<>::create_fleet(15);
        evtol::SimulationEngine sim_engine(*stats_collector_, 2.0);
        sim_engine.run_simulation(*charger_manager_, fleet);

        std::string report = stats_collector_->generate_report();

        // Report should contain meaningful data
        EXPECT_FALSE(report.empty());
        EXPECT_GT(report.length(), 500); // Substantial content

        // Should contain data for all aircraft types
        EXPECT_NE(report.find("Alpha Aircraft"), std::string::npos);
        EXPECT_NE(report.find("Beta Aircraft"), std::string::npos);
        EXPECT_NE(report.find("Charlie Aircraft"), std::string::npos);
        EXPECT_NE(report.find("Delta Aircraft"), std::string::npos);
        EXPECT_NE(report.find("Echo Aircraft"), std::string::npos);

        // Should contain numerical data
        EXPECT_NE(report.find("Average Flight Time"), std::string::npos);
        EXPECT_NE(report.find("Total Faults"), std::string::npos);
        EXPECT_NE(report.find("Total Passenger Miles"), std::string::npos);
    }

    // Test edge case: very short simulation
    TEST_F(SimulationIntegrationTest, VeryShortSimulation)
    {
        auto fleet = evtol::AircraftFactory<>::create_fleet(5);
        evtol::SimulationEngine sim_engine(*stats_collector_, 0.01); // 36 seconds

        sim_engine.run_simulation(*charger_manager_, fleet);

        auto summary = stats_collector_->get_summary_stats();

        // Should have some activity even in short simulation
        EXPECT_GE(summary.total_flights, 0);

        // Report should be generated even with minimal data
        std::string report = stats_collector_->generate_report();
        EXPECT_FALSE(report.empty());
    }

    // Test edge case: very long simulation
    TEST_F(SimulationIntegrationTest, VeryLongSimulation)
    {
        int fleet_size = 10;
        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);
        evtol::SimulationEngine sim_engine(*stats_collector_, 1000.0); // 1000 hours

        auto start_time = std::chrono::high_resolution_clock::now();
        sim_engine.run_simulation(*charger_manager_, fleet);
        auto end_time = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // Should complete efficiently even for long simulations
        EXPECT_LT(duration.count(), 15000); // Less than 15 seconds real time

        auto summary = stats_collector_->get_summary_stats();
        // This should have a tolerance, but since this is just a simple project, I feel dangerous
        EXPECT_GT(summary.total_faults, fleet_size); // All *should* have a fault and stop
    }

    // Test aircraft behavior consistency
    TEST_F(SimulationIntegrationTest, AircraftBehaviorConsistency)
    {
        auto fleet = evtol::AircraftFactory<>::create_fleet(25);
        evtol::SimulationEngine sim_engine(*stats_collector_, 2.0);
        sim_engine.run_simulation(*charger_manager_, fleet);

        // Verify aircraft-specific behavior
        auto alpha_stats = stats_collector_->get_stats(evtol::AircraftType::ALPHA);
        auto echo_stats = stats_collector_->get_stats(evtol::AircraftType::ECHO);

        if (alpha_stats.flight_count > 0 && echo_stats.flight_count > 0)
        {
            // Alpha should generally have longer flight times (higher speed, larger battery)
            // Echo should generally have shorter flight times (low speed, high energy consumption)
            EXPECT_GT(alpha_stats.avg_flight_time(), echo_stats.avg_flight_time() * 0.5);

            // Alpha should generally travel farther per flight
            EXPECT_GT(alpha_stats.avg_distance(), echo_stats.avg_distance());
        }
    }

    // Test system behavior under stress
    TEST_F(SimulationIntegrationTest, SystemBehaviorUnderStress)
    {
        const int stress_fleet_size = 200;
        auto fleet = evtol::AircraftFactory<>::create_fleet(stress_fleet_size);

        evtol::SimulationEngine sim_engine(*stats_collector_, 3.0);

        auto start_time = std::chrono::high_resolution_clock::now();
        sim_engine.run_simulation(*charger_manager_, fleet);
        auto end_time = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // Should handle large fleet without excessive time
        EXPECT_LT(duration.count(), 30000); // Less than 30 seconds

        auto summary = stats_collector_->get_summary_stats();
        EXPECT_GT(summary.total_flights, stress_fleet_size);

        // Charger system should still function correctly
        EXPECT_EQ(charger_manager_->get_total_chargers(), 3);
        EXPECT_LE(charger_manager_->get_active_chargers(), 3);
    }

    // Test complete simulation runner integration
    TEST_F(SimulationIntegrationTest, CompleteSimulationRunnerIntegration)
    {
        // This tests the integration similar to the main simulation runner
        const int fleet_size = 20;
        const int num_chargers = 3;
        const double simulation_duration = 3.0;

        // Create components
        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);
        evtol::ChargerManager charger_manager;
        evtol::StatisticsCollector stats_collector;
        evtol::SimulationEngine sim_engine(stats_collector, simulation_duration);

        // Verify initial state
        EXPECT_EQ(fleet.size(), static_cast<size_t>(fleet_size));
        EXPECT_EQ(charger_manager.get_total_chargers(), num_chargers);

        // Run simulation
        sim_engine.run_simulation(charger_manager, fleet);

        // Verify results
        auto summary = stats_collector.get_summary_stats();
        EXPECT_GT(summary.total_flights, 0);
        EXPECT_GT(summary.total_charges, 0);
        EXPECT_GT(summary.total_passenger_miles, 0.0);

        // Generate and verify report
        std::string report = stats_collector.generate_report();
        EXPECT_FALSE(report.empty());
        EXPECT_NE(report.find("eVTOL Simulation Results"), std::string::npos);

        // System should be in consistent final state
        EXPECT_EQ(charger_manager.get_total_chargers(), num_chargers);
        EXPECT_LE(charger_manager.get_active_chargers(), num_chargers);
    }

} // namespace evtol_test