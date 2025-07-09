#include "test_utilities.h"

namespace evtol_test
{
    // Test fixture for core functionality tests
    class CoreFunctionalityTest : public ::testing::Test
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

    // Test 1: Complete simulation workflow with real components
    TEST_F(CoreFunctionalityTest, CompleteSimulationWorkflow)
    {
        const int fleet_size = 20;
        const double simulation_duration = 3.0;

        // Create real fleet using factory
        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);

        // Create simulation engine
        evtol::EventDrivenSimulation sim_engine(*stats_collector_, simulation_duration);

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

    // Test 2: Basic fleet creation
    TEST_F(CoreFunctionalityTest, BasicFleetCreation)
    {
        const int fleet_size = 5;
        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);
        
        EXPECT_EQ(fleet.size(), static_cast<size_t>(fleet_size));
        
        // Verify each aircraft is created and has unique ID
        for (size_t i = 0; i < fleet.size(); ++i) {
            EXPECT_NE(fleet[i], nullptr);
            EXPECT_EQ(fleet[i]->get_id(), static_cast<int>(i));
        }
    }

    // Test 3: AlphaAircraft construction and basic properties
    TEST_F(CoreFunctionalityTest, AlphaAircraftConstruction)
    {
        const int test_id = 42;
        evtol::AlphaAircraft aircraft(test_id);

        EXPECT_EQ(aircraft.get_id(), test_id);
        EXPECT_EQ(aircraft.get_type(), evtol::AircraftType::ALPHA);
        EXPECT_EQ(aircraft.get_manufacturer(), "Alpha");
        EXPECT_EQ(aircraft.get_passenger_count(), 4);
        EXPECT_NEAR_TOLERANCE(aircraft.get_charge_time_hours(), 0.6);
        EXPECT_NEAR_TOLERANCE(aircraft.get_battery_level(), 1.0);
    }

    // Test 4: Single charger request
    TEST_F(CoreFunctionalityTest, SingleChargerRequest)
    {
        int aircraft_id = 100;
        
        bool request_result = charger_manager_->request_charger(aircraft_id);
        
        EXPECT_TRUE(request_result);
        EXPECT_EQ(charger_manager_->get_available_chargers(), 2);
        EXPECT_EQ(charger_manager_->get_active_chargers(), 1);
        EXPECT_EQ(charger_manager_->get_queue_size(), 0);
    }

    // Test 5: Record single flight
    TEST_F(CoreFunctionalityTest, RecordSingleFlight)
    {
        stats_collector_->record_flight(evtol::AircraftType::ALPHA, 2.5, 150.0, 4);

        auto alpha_stats = stats_collector_->get_stats(evtol::AircraftType::ALPHA);

        EXPECT_NEAR_TOLERANCE(alpha_stats.total_flight_time_hours, 2.5);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_distance_miles, 150.0);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_passenger_miles, 600.0); // 4 passengers * 150 miles
        EXPECT_EQ(alpha_stats.flight_count, 1);

        // Other aircraft types should remain unchanged
        auto beta_stats = stats_collector_->get_stats(evtol::AircraftType::BETA);
        EXPECT_EQ(beta_stats.flight_count, 0);
    }

    // Test 6: Multiple aircraft simulation
    TEST_F(CoreFunctionalityTest, MultipleAircraftSimulation)
    {
        auto fleet = TestDataGenerator::create_test_fleet(5);
        evtol::EventDrivenSimulation sim_engine(*stats_collector_, 3.0);
        
        sim_engine.run_simulation(*charger_manager_, fleet);
        
        // Check using the base StatisticsCollector's summary stats
        auto summary = stats_collector_->get_summary_stats();
        EXPECT_GE(summary.total_flights, 5);  // All aircraft should have completed at least one flight
        EXPECT_GT(summary.total_charges, 0);  // Some charge sessions should have completed
    }

    // Test 7: Aircraft type distribution in simulation results
    TEST_F(CoreFunctionalityTest, AircraftTypeDistributionInResults)
    {
        // This test isn't guaranteed to pass, since distribution is random
        // TODO Originally, it wasn't random, but this needs addressing
        const int fleet_size = 50;
        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);

        evtol::EventDrivenSimulation sim_engine(*stats_collector_, 25.0);
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

    // Test 8: Report generation with real data
    TEST_F(CoreFunctionalityTest, ReportGenerationWithRealData)
    {
        auto fleet = evtol::AircraftFactory<>::create_fleet(15);
        evtol::EventDrivenSimulation sim_engine(*stats_collector_, 2.0);
        sim_engine.run_simulation(*charger_manager_, fleet);

        std::string report = stats_collector_->generate_report();

        // Report should contain meaningful data
        EXPECT_FALSE(report.empty());
        EXPECT_GT(static_cast<int>(report.length()), 500); // Substantial content

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

} // namespace evtol_test