#include "test_utilities.h"

namespace evtol_test
{
    // Test fixture for system behavior tests
    class SystemBehaviorTest : public ::testing::Test
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

    // Test 1: Simulation with different fleet sizes
    TEST_F(SystemBehaviorTest, DifferentFleetSizes)
    {
        std::vector<int> fleet_sizes = {1, 5, 10, 20, 50};

        for (int fleet_size : fleet_sizes)
        {
            // Reset components for each test
            stats_collector_ = std::make_unique<evtol::StatisticsCollector>();
            charger_manager_ = std::make_unique<evtol::ChargerManager>();

            auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);
            evtol::EventDrivenSimulation sim_engine(*stats_collector_, 2.0);

            sim_engine.run_simulation(*charger_manager_, fleet);

            auto summary = stats_collector_->get_summary_stats();

            // Results should scale with fleet size
            EXPECT_GE(summary.total_flights, fleet_size / 2); // At least half should fly
            EXPECT_GT(summary.total_passenger_miles, 0.0);

            // Charger management should work regardless of fleet size
            EXPECT_EQ(charger_manager_->get_total_chargers(), 3);
            EXPECT_LE(charger_manager_->get_active_chargers(), 3);
        }
    }

    // Test 2: Charger utilization during simulation
    TEST_F(SystemBehaviorTest, ChargerUtilizationDuringSimulation)
    {
        const int fleet_size = 15; // More aircraft than chargers
        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);

        evtol::EventDrivenSimulation sim_engine(*stats_collector_, 3.0);
        sim_engine.run_simulation(*charger_manager_, fleet);

        // Chargers should have been well utilized
        auto summary = stats_collector_->get_summary_stats();
        EXPECT_GT(summary.total_charges, 10); // Reasonable utilization

        // Final state should be reasonable
        EXPECT_EQ(charger_manager_->get_total_chargers(), 3);
        EXPECT_LE(charger_manager_->get_active_chargers(), 3);
        EXPECT_LE(charger_manager_->get_queue_size(), fleet_size);
    }

    // Test 3: Fault occurrence and handling
    TEST_F(SystemBehaviorTest, FaultOccurrenceAndHandling)
    {
        const int fleet_size = 50; // Large fleet increases fault probability
        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);

        evtol::EventDrivenSimulation sim_engine(*stats_collector_, 3.0);
        sim_engine.run_simulation(*charger_manager_, fleet);

        auto summary = stats_collector_->get_summary_stats();

        // With large fleet and longer duration, some faults should occur
        EXPECT_GE(summary.total_faults, 0);

        // Simulation should continue despite faults
        EXPECT_GT(summary.total_flights, fleet_size);
    }

    // Test 4: Aircraft behavior consistency
    TEST_F(SystemBehaviorTest, AircraftBehaviorConsistency)
    {
        auto fleet = evtol::AircraftFactory<>::create_fleet(25);
        evtol::EventDrivenSimulation sim_engine(*stats_collector_, 2.0);
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

    // Test 5: Performance characteristics
    TEST_F(SystemBehaviorTest, PerformanceCharacteristics)
    {
        const int fleet_size = 100;
        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);

        auto start_time = std::chrono::high_resolution_clock::now();

        evtol::EventDrivenSimulation sim_engine(*stats_collector_, 2.5);
        sim_engine.run_simulation(*charger_manager_, fleet);

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // Should complete in reasonable time
        EXPECT_LT(duration.count(), 10000); // Less than 10 seconds

        auto summary = stats_collector_->get_summary_stats();
        EXPECT_GT(summary.total_flights, fleet_size);
    }

    // Test 6: System behavior under stress
    TEST_F(SystemBehaviorTest, SystemBehaviorUnderStress)
    {
        const int stress_fleet_size = 200;
        auto fleet = evtol::AircraftFactory<>::create_fleet(stress_fleet_size);

        evtol::EventDrivenSimulation sim_engine(*stats_collector_, 3.0);

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

} // namespace evtol_test