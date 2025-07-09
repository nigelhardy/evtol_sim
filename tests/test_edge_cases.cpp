#include "test_utilities.h"

namespace evtol_test
{
    // Test fixture for edge case tests
    class EdgeCasesTest : public ::testing::Test
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

    // Test 1: Zero duration simulation
    TEST_F(EdgeCasesTest, ZeroDurationSimulation)
    {
        auto fleet = evtol::AircraftFactory<>::create_fleet(5);
        evtol::EventDrivenSimulation sim_engine(*stats_collector_, 0.0);

        sim_engine.run_simulation(*charger_manager_, fleet);

        auto summary = stats_collector_->get_summary_stats();

        // No events should complete with zero duration
        EXPECT_EQ(summary.total_flights, 0);
        EXPECT_EQ(summary.total_charges, 0);
    }

    // Test 2: Empty fleet with positive duration
    TEST_F(EdgeCasesTest, EmptyFleetWithPositiveDuration)
    {
        std::vector<std::unique_ptr<evtol::AircraftBase>> empty_fleet;
        evtol::EventDrivenSimulation sim_engine(*stats_collector_, 3.0);

        sim_engine.run_simulation(*charger_manager_, empty_fleet);

        auto summary = stats_collector_->get_summary_stats();
        EXPECT_EQ(summary.total_flights, 0);
        EXPECT_EQ(summary.total_charges, 0);
        EXPECT_EQ(summary.total_faults, 0);

        // Charger state should remain unchanged
        EXPECT_EQ(charger_manager_->get_available_chargers(), 3);
        EXPECT_EQ(charger_manager_->get_active_chargers(), 0);
    }

    // Test 3: Extreme number of chargers vs aircraft
    TEST_F(EdgeCasesTest, ExtremeChargerToAircraftRatio)
    {
        // Test with many more aircraft than chargers
        auto large_fleet = evtol::AircraftFactory<>::create_fleet(100);
        evtol::EventDrivenSimulation sim_engine(*stats_collector_, 1.0);

        sim_engine.run_simulation(*charger_manager_, large_fleet);

        auto summary = stats_collector_->get_summary_stats();
        EXPECT_GT(summary.total_flights, 0);

        // Should still function despite high contention
        EXPECT_EQ(charger_manager_->get_total_chargers(), 3);
        EXPECT_LE(charger_manager_->get_active_chargers(), 3);
    }

    // Test 4: Very small simulation duration
    TEST_F(EdgeCasesTest, VerySmallSimulationDuration)
    {
        auto fleet = evtol::AircraftFactory<>::create_fleet(10);
        evtol::EventDrivenSimulation sim_engine(*stats_collector_, 0.0001); // 0.36 seconds

        sim_engine.run_simulation(*charger_manager_, fleet);

        auto summary = stats_collector_->get_summary_stats();
        // With very small duration, minimal activity expected (but partial flights still)
        // TODO we shoud make a version of this test where we disable partial flights, and expect 0
        EXPECT_GE(summary.total_flights, 0);
    }

    // Test 5: Charging queue overflow scenario
    TEST_F(EdgeCasesTest, ChargingQueueOverflowScenario)
    {
        // Create scenario where many aircraft need charging simultaneously
        auto fleet = evtol::AircraftFactory<>::create_fleet(50);

        // Discharge all aircraft batteries
        for (auto &aircraft : fleet)
        {
            aircraft->discharge_battery();
        }

        evtol::EventDrivenSimulation sim_engine(*stats_collector_, 1.0);
        sim_engine.run_simulation(*charger_manager_, fleet);

        // Should handle large charging queue gracefully
        auto summary = stats_collector_->get_summary_stats();
        EXPECT_GT(summary.total_charges, 0);

        // Queue should be managed effectively
        EXPECT_LT(charger_manager_->get_queue_size(), 50); // Reasonable final state
    }

    // Test 6: Aircraft independence (each aircraft is separate object)
    TEST_F(EdgeCasesTest, AircraftIndependence)
    {
        const int fleet_size = 10;
        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);
        
        // Verify each aircraft is a unique object
        for (size_t i = 0; i < fleet.size(); ++i) {
            for (size_t j = i + 1; j < fleet.size(); ++j) {
                EXPECT_NE(fleet[i].get(), fleet[j].get());
            }
        }
        
        // Modify one aircraft and verify others are not affected
        fleet[0]->discharge_battery();
        EXPECT_NEAR_TOLERANCE(fleet[0]->get_battery_level(), 0.0);
        
        for (size_t i = 1; i < fleet.size(); ++i) {
            EXPECT_NEAR_TOLERANCE(fleet[i]->get_battery_level(), 1.0);
        }
    }

} // namespace evtol_test