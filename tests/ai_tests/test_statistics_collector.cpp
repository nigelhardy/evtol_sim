#include "test_utilities.h"

namespace evtol_test
{

    // Test StatisticsCollector construction and initial state
    TEST_F(StatisticsCollectorTest, Construction)
    {
        // Verify all aircraft types are initialized with empty stats
        auto alpha_stats = stats_collector_->get_stats(evtol::AircraftType::ALPHA);
        auto beta_stats = stats_collector_->get_stats(evtol::AircraftType::BETA);
        auto charlie_stats = stats_collector_->get_stats(evtol::AircraftType::CHARLIE);
        auto delta_stats = stats_collector_->get_stats(evtol::AircraftType::DELTA);
        auto echo_stats = stats_collector_->get_stats(evtol::AircraftType::ECHO);

        EXPECT_NEAR_TOLERANCE(alpha_stats.total_flight_time_hours, 0.0);
        EXPECT_NEAR_TOLERANCE(beta_stats.total_distance_miles, 0.0);
        EXPECT_NEAR_TOLERANCE(charlie_stats.total_charging_time_hours, 0.0);
        EXPECT_EQ(delta_stats.total_faults, 0);
        EXPECT_EQ(echo_stats.flight_count, 0);
    }

    // Test recording single flight
    TEST_F(StatisticsCollectorTest, RecordSingleFlight)
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

    // Test recording multiple flights for same aircraft type
    TEST_F(StatisticsCollectorTest, RecordMultipleFlightsSameType)
    {
        stats_collector_->record_flight(evtol::AircraftType::ALPHA, 2.5, 150.0, 4);
        stats_collector_->record_flight(evtol::AircraftType::ALPHA, 1.5, 100.0, 3);
        stats_collector_->record_flight(evtol::AircraftType::ALPHA, 3.0, 200.0, 2);

        auto alpha_stats = stats_collector_->get_stats(evtol::AircraftType::ALPHA);

        EXPECT_NEAR_TOLERANCE(alpha_stats.total_flight_time_hours, 7.0);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_distance_miles, 450.0);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_passenger_miles, 1300.0); // 600 + 300 + 400
        EXPECT_EQ(alpha_stats.flight_count, 3);

        // Test average calculations
        EXPECT_NEAR_TOLERANCE(alpha_stats.avg_flight_time(), 7.0 / 3.0);
        EXPECT_NEAR_TOLERANCE(alpha_stats.avg_distance(), 450.0 / 3.0);
    }

    // Test recording flights for different aircraft types
    TEST_F(StatisticsCollectorTest, RecordFlightsDifferentTypes)
    {
        stats_collector_->record_flight(evtol::AircraftType::ALPHA, 2.5, 150.0, 4);
        stats_collector_->record_flight(evtol::AircraftType::BETA, 1.5, 100.0, 5);
        stats_collector_->record_flight(evtol::AircraftType::CHARLIE, 3.0, 200.0, 3);
        stats_collector_->record_flight(evtol::AircraftType::DELTA, 2.0, 120.0, 2);
        stats_collector_->record_flight(evtol::AircraftType::ECHO, 1.0, 80.0, 2);

        // Verify each aircraft type has correct stats
        auto alpha_stats = stats_collector_->get_stats(evtol::AircraftType::ALPHA);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_flight_time_hours, 2.5);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_passenger_miles, 600.0);
        EXPECT_EQ(alpha_stats.flight_count, 1);

        auto beta_stats = stats_collector_->get_stats(evtol::AircraftType::BETA);
        EXPECT_NEAR_TOLERANCE(beta_stats.total_distance_miles, 100.0);
        EXPECT_NEAR_TOLERANCE(beta_stats.total_passenger_miles, 500.0);
        EXPECT_EQ(beta_stats.flight_count, 1);

        auto charlie_stats = stats_collector_->get_stats(evtol::AircraftType::CHARLIE);
        EXPECT_NEAR_TOLERANCE(charlie_stats.total_flight_time_hours, 3.0);
        EXPECT_NEAR_TOLERANCE(charlie_stats.total_passenger_miles, 600.0);
        EXPECT_EQ(charlie_stats.flight_count, 1);

        auto delta_stats = stats_collector_->get_stats(evtol::AircraftType::DELTA);
        EXPECT_NEAR_TOLERANCE(delta_stats.total_distance_miles, 120.0);
        EXPECT_NEAR_TOLERANCE(delta_stats.total_passenger_miles, 240.0);
        EXPECT_EQ(delta_stats.flight_count, 1);

        auto echo_stats = stats_collector_->get_stats(evtol::AircraftType::ECHO);
        EXPECT_NEAR_TOLERANCE(echo_stats.total_flight_time_hours, 1.0);
        EXPECT_NEAR_TOLERANCE(echo_stats.total_passenger_miles, 160.0);
        EXPECT_EQ(echo_stats.flight_count, 1);
    }

    // Test recording single charge session
    TEST_F(StatisticsCollectorTest, RecordSingleChargeSession)
    {
        stats_collector_->record_charge_session(evtol::AircraftType::ALPHA, 1.5);

        auto alpha_stats = stats_collector_->get_stats(evtol::AircraftType::ALPHA);

        EXPECT_NEAR_TOLERANCE(alpha_stats.total_charging_time_hours, 1.5);
        EXPECT_EQ(alpha_stats.charge_count, 1);
        EXPECT_NEAR_TOLERANCE(alpha_stats.avg_charging_time(), 1.5);

        // Other stats should remain zero
        EXPECT_EQ(alpha_stats.flight_count, 0);
        EXPECT_EQ(alpha_stats.total_faults, 0);
    }

    // Test recording multiple charge sessions
    TEST_F(StatisticsCollectorTest, RecordMultipleChargeSessions)
    {
        stats_collector_->record_charge_session(evtol::AircraftType::BETA, 1.0);
        stats_collector_->record_charge_session(evtol::AircraftType::BETA, 1.5);
        stats_collector_->record_charge_session(evtol::AircraftType::BETA, 2.0);

        auto beta_stats = stats_collector_->get_stats(evtol::AircraftType::BETA);

        EXPECT_NEAR_TOLERANCE(beta_stats.total_charging_time_hours, 4.5);
        EXPECT_EQ(beta_stats.charge_count, 3);
        EXPECT_NEAR_TOLERANCE(beta_stats.avg_charging_time(), 1.5);
    }

    // Test recording faults
    TEST_F(StatisticsCollectorTest, RecordFaults)
    {
        stats_collector_->record_fault(evtol::AircraftType::CHARLIE);
        stats_collector_->record_fault(evtol::AircraftType::CHARLIE);
        stats_collector_->record_fault(evtol::AircraftType::DELTA);

        auto charlie_stats = stats_collector_->get_stats(evtol::AircraftType::CHARLIE);
        auto delta_stats = stats_collector_->get_stats(evtol::AircraftType::DELTA);
        auto alpha_stats = stats_collector_->get_stats(evtol::AircraftType::ALPHA);

        EXPECT_EQ(charlie_stats.total_faults, 2);
        EXPECT_EQ(delta_stats.total_faults, 1);
        EXPECT_EQ(alpha_stats.total_faults, 0);
    }

    // Test mixed operations (flights, charges, faults)
    TEST_F(StatisticsCollectorTest, MixedOperations)
    {
        // Record various operations for Alpha aircraft
        stats_collector_->record_flight(evtol::AircraftType::ALPHA, 2.0, 120.0, 4);
        stats_collector_->record_charge_session(evtol::AircraftType::ALPHA, 1.0);
        stats_collector_->record_fault(evtol::AircraftType::ALPHA);
        stats_collector_->record_flight(evtol::AircraftType::ALPHA, 1.5, 90.0, 3);
        stats_collector_->record_charge_session(evtol::AircraftType::ALPHA, 1.5);
        stats_collector_->record_fault(evtol::AircraftType::ALPHA);

        auto alpha_stats = stats_collector_->get_stats(evtol::AircraftType::ALPHA);

        EXPECT_NEAR_TOLERANCE(alpha_stats.total_flight_time_hours, 3.5);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_distance_miles, 210.0);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_charging_time_hours, 2.5);
        EXPECT_EQ(alpha_stats.total_faults, 2);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_passenger_miles, 750.0); // 4*120 + 3*90
        EXPECT_EQ(alpha_stats.flight_count, 2);
        EXPECT_EQ(alpha_stats.charge_count, 2);

        // Test averages
        EXPECT_NEAR_TOLERANCE(alpha_stats.avg_flight_time(), 1.75);
        EXPECT_NEAR_TOLERANCE(alpha_stats.avg_distance(), 105.0);
        EXPECT_NEAR_TOLERANCE(alpha_stats.avg_charging_time(), 1.25);
    }

    // Test get_summary_stats functionality
    TEST_F(StatisticsCollectorTest, GetSummaryStats)
    {
        // Add data for multiple aircraft types
        stats_collector_->record_flight(evtol::AircraftType::ALPHA, 2.0, 100.0, 4);
        stats_collector_->record_flight(evtol::AircraftType::BETA, 1.5, 80.0, 5);
        stats_collector_->record_charge_session(evtol::AircraftType::ALPHA, 1.0);
        stats_collector_->record_charge_session(evtol::AircraftType::BETA, 0.5);
        stats_collector_->record_fault(evtol::AircraftType::ALPHA);
        stats_collector_->record_fault(evtol::AircraftType::BETA);
        stats_collector_->record_fault(evtol::AircraftType::BETA);

        auto summary = stats_collector_->get_summary_stats();

        EXPECT_NEAR_TOLERANCE(summary.total_flight_time, 3.5);
        EXPECT_NEAR_TOLERANCE(summary.total_distance, 180.0);
        EXPECT_NEAR_TOLERANCE(summary.total_charging_time, 1.5);
        EXPECT_EQ(summary.total_faults, 3);
        EXPECT_NEAR_TOLERANCE(summary.total_passenger_miles, 800.0); // 4*100 + 5*80
        EXPECT_EQ(summary.total_flights, 2);
        EXPECT_EQ(summary.total_charges, 2);
    }

    // Test report generation (basic format check)
    TEST_F(StatisticsCollectorTest, GenerateReport)
    {
        // Add some test data
        stats_collector_->record_flight(evtol::AircraftType::ALPHA, 2.0, 100.0, 4);
        stats_collector_->record_charge_session(evtol::AircraftType::ALPHA, 1.0);
        stats_collector_->record_fault(evtol::AircraftType::ALPHA);

        std::string report = stats_collector_->generate_report();

        // Check that report contains expected content
        EXPECT_NE(report.find("eVTOL Simulation Results"), std::string::npos);
        EXPECT_NE(report.find("Alpha Aircraft"), std::string::npos);
        EXPECT_NE(report.find("Beta Aircraft"), std::string::npos);
        EXPECT_NE(report.find("Charlie Aircraft"), std::string::npos);
        EXPECT_NE(report.find("Delta Aircraft"), std::string::npos);
        EXPECT_NE(report.find("Echo Aircraft"), std::string::npos);
        EXPECT_NE(report.find("Average Flight Time"), std::string::npos);
        EXPECT_NE(report.find("Total Faults"), std::string::npos);

        // Check that numerical values appear in report
        EXPECT_NE(report.find("2.00"), std::string::npos); // Flight time
        EXPECT_NE(report.find("1"), std::string::npos);    // Fault count
    }

    // Test filtered stats functionality
    TEST_F(StatisticsCollectorTest, GetFilteredStats)
    {
        // Add data for multiple aircraft types
        stats_collector_->record_flight(evtol::AircraftType::ALPHA, 2.0, 100.0, 4);
        stats_collector_->record_flight(evtol::AircraftType::BETA, 1.5, 80.0, 5);
        stats_collector_->record_flight(evtol::AircraftType::CHARLIE, 3.0, 150.0, 3);
        stats_collector_->record_fault(evtol::AircraftType::ALPHA);
        stats_collector_->record_fault(evtol::AircraftType::CHARLIE);
        stats_collector_->record_fault(evtol::AircraftType::CHARLIE);

        // Filter for aircraft types with more than 1 fault
        auto filtered = stats_collector_->get_filtered_stats(
            [](evtol::AircraftType /*type*/, const evtol::FlightStats &stats)
            {
                return stats.total_faults > 1;
            });

        EXPECT_EQ(filtered.size(), 1);
        auto [type1, stats1] = filtered[0];
        EXPECT_EQ(type1, evtol::AircraftType::CHARLIE);
        EXPECT_EQ(stats1.total_faults, 2);

        // Filter for aircraft types with flight time > 2.5 hours
        auto filtered2 = stats_collector_->get_filtered_stats(
            [](evtol::AircraftType /*type*/, const evtol::FlightStats &stats)
            {
                return stats.total_flight_time_hours > 2.5;
            });

        EXPECT_EQ(filtered2.size(), 1);
        auto [type2, stats2] = filtered2[0];
        EXPECT_EQ(type2, evtol::AircraftType::CHARLIE);
        EXPECT_NEAR_TOLERANCE(stats2.total_flight_time_hours, 3.0);
    }

    // Test aggregate stats functionality
    TEST_F(StatisticsCollectorTest, AggregateStats)
    {
        // Add data for multiple aircraft types
        stats_collector_->record_flight(evtol::AircraftType::ALPHA, 2.0, 100.0, 4);
        stats_collector_->record_flight(evtol::AircraftType::BETA, 1.5, 80.0, 5);
        stats_collector_->record_flight(evtol::AircraftType::CHARLIE, 3.0, 150.0, 3);

        // Aggregate to find total flight time across all aircraft types
        auto total_flight_time = stats_collector_->aggregate_stats(
            [](const std::unordered_map<evtol::AircraftType, evtol::FlightStats> &stats_map)
            {
                double total = 0.0;
                for (const auto &[type, stats] : stats_map)
                {
                    total += stats.total_flight_time_hours;
                }
                return total;
            });

        EXPECT_NEAR_TOLERANCE(total_flight_time, 6.5);

        // Aggregate to find maximum distance in a single flight type
        auto max_distance = stats_collector_->aggregate_stats(
            [](const std::unordered_map<evtol::AircraftType, evtol::FlightStats> &stats_map)
            {
                double max_dist = 0.0;
                for (const auto &[type, stats] : stats_map)
                {
                    if (stats.total_distance_miles > max_dist)
                    {
                        max_dist = stats.total_distance_miles;
                    }
                }
                return max_dist;
            });

        EXPECT_NEAR_TOLERANCE(max_distance, 150.0);
    }

    // Test edge cases: zero values
    TEST_F(StatisticsCollectorTest, ZeroValues)
    {
        stats_collector_->record_flight(evtol::AircraftType::ALPHA, 0.0, 0.0, 0);
        stats_collector_->record_charge_session(evtol::AircraftType::ALPHA, 0.0);

        auto alpha_stats = stats_collector_->get_stats(evtol::AircraftType::ALPHA);

        EXPECT_NEAR_TOLERANCE(alpha_stats.total_flight_time_hours, 0.0);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_distance_miles, 0.0);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_charging_time_hours, 0.0);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_passenger_miles, 0.0);
        EXPECT_EQ(alpha_stats.flight_count, 1);
        EXPECT_EQ(alpha_stats.charge_count, 1);

        // Average calculations with zero denominators
        EXPECT_NEAR_TOLERANCE(alpha_stats.avg_flight_time(), 0.0);
        EXPECT_NEAR_TOLERANCE(alpha_stats.avg_distance(), 0.0);
        EXPECT_NEAR_TOLERANCE(alpha_stats.avg_charging_time(), 0.0);
    }

    // Test large scale data collection
    TEST_F(StatisticsCollectorTest, LargeScaleDataCollection)
    {
        const int num_operations = 1000;

        // Add many operations for each aircraft type
        for (int i = 0; i < num_operations; ++i)
        {
            evtol::AircraftType type = static_cast<evtol::AircraftType>(i % 5);

            stats_collector_->record_flight(type, 1.0 + i * 0.001, 50.0 + i * 0.1, 2 + (i % 3));
            stats_collector_->record_charge_session(type, 0.5 + i * 0.0001);

            if (i % 10 == 0)
            {
                stats_collector_->record_fault(type);
            }
        }

        // Verify data integrity
        auto summary = stats_collector_->get_summary_stats();

        EXPECT_EQ(summary.total_flights, num_operations);
        EXPECT_EQ(summary.total_charges, num_operations);
        EXPECT_EQ(summary.total_faults, 100); // Every 10th operation

        EXPECT_GT(summary.total_flight_time, 0.0);
        EXPECT_GT(summary.total_distance, 0.0);
        EXPECT_GT(summary.total_charging_time, 0.0);
        EXPECT_GT(summary.total_passenger_miles, 0.0);
    }

    // Test precision with very small values
    TEST_F(StatisticsCollectorTest, PrecisionSmallValues)
    {
        stats_collector_->record_flight(evtol::AircraftType::ALPHA, 0.001, 0.01, 1);
        stats_collector_->record_charge_session(evtol::AircraftType::ALPHA, 0.001);

        auto alpha_stats = stats_collector_->get_stats(evtol::AircraftType::ALPHA);

        EXPECT_NEAR_TOLERANCE(alpha_stats.total_flight_time_hours, 0.001);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_distance_miles, 0.01);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_charging_time_hours, 0.001);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_passenger_miles, 0.01);
    }

    // Test precision with very large values
    TEST_F(StatisticsCollectorTest, PrecisionLargeValues)
    {
        stats_collector_->record_flight(evtol::AircraftType::ECHO, 1000.0, 50000.0, 10);
        stats_collector_->record_charge_session(evtol::AircraftType::ECHO, 100.0);

        auto echo_stats = stats_collector_->get_stats(evtol::AircraftType::ECHO);

        EXPECT_NEAR_TOLERANCE(echo_stats.total_flight_time_hours, 1000.0);
        EXPECT_NEAR_TOLERANCE(echo_stats.total_distance_miles, 50000.0);
        EXPECT_NEAR_TOLERANCE(echo_stats.total_charging_time_hours, 100.0);
        EXPECT_NEAR_TOLERANCE(echo_stats.total_passenger_miles, 500000.0);
    }

    // Test template-based record_flight with additional metrics
    TEST_F(StatisticsCollectorTest, TemplateRecordFlightWithMetrics)
    {
        double metric1 = 25.5;
        double metric2 = 1.0;
        double metric3 = 2.0;
        double metric4 = 3.0;
        int passengers = 4;
        double miles = 100.0;
        // Test the variadic template functionality
        stats_collector_->record_flight(evtol::AircraftType::ALPHA, 2.0, miles, passengers, metric1, metric2, metric3, metric4);

        auto alpha_stats = stats_collector_->get_stats(evtol::AircraftType::ALPHA);

        EXPECT_NEAR_TOLERANCE(alpha_stats.total_flight_time_hours, 2.0);
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_distance_miles, 100.0);
        // arbitrarily, metrics are just added to passenger miles, so verify that is working
        EXPECT_NEAR_TOLERANCE(alpha_stats.total_passenger_miles, (miles * passengers) + metric1 + metric2 + metric3 + metric4);
        EXPECT_EQ(alpha_stats.flight_count, 1);
    }

    // Test template-based record_charge_session with additional metrics
    TEST_F(StatisticsCollectorTest, TemplateRecordChargeWithMetrics)
    {
        // Test the variadic template functionality
        stats_collector_->record_charge_session(evtol::AircraftType::BETA, 1.5, 42.0);

        auto beta_stats = stats_collector_->get_stats(evtol::AircraftType::BETA);

        EXPECT_NEAR_TOLERANCE(beta_stats.total_charging_time_hours, 1.5);
        EXPECT_EQ(beta_stats.charge_count, 1);
    }

    // Test report format consistency
    TEST_F(StatisticsCollectorTest, ReportFormatConsistency)
    {
        // Add identical data to multiple aircraft types
        for (int type_int = 0; type_int < 5; ++type_int)
        {
            evtol::AircraftType type = static_cast<evtol::AircraftType>(type_int);
            stats_collector_->record_flight(type, 2.0, 100.0, 3);
            stats_collector_->record_charge_session(type, 1.0);
            stats_collector_->record_fault(type);
        }

        std::string report = stats_collector_->generate_report();

        // Count occurrences of key phrases - should appear once for each aircraft type
        size_t flight_time_count = 0;
        size_t pos = 0;
        while ((pos = report.find("Average Flight Time", pos)) != std::string::npos)
        {
            flight_time_count++;
            pos++;
        }
        EXPECT_EQ(flight_time_count, 5); // One for each aircraft type
    }

} // namespace evtol_test