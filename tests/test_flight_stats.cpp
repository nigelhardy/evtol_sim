#include "test_utilities.h"

namespace evtol_test {

// Test FlightStats default construction
TEST_F(FlightStatsTest, DefaultConstruction) {
    EXPECT_NEAR_TOLERANCE(stats_.total_flight_time_hours, 0.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_distance_miles, 0.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_charging_time_hours, 0.0);
    EXPECT_EQ(stats_.total_faults, 0);
    EXPECT_NEAR_TOLERANCE(stats_.total_passenger_miles, 0.0);
    EXPECT_EQ(stats_.flight_count, 0);
    EXPECT_EQ(stats_.charge_count, 0);
}

// Test FlightStats copy constructor
TEST_F(FlightStatsTest, CopyConstructor) {
    // Set up original stats
    stats_.total_flight_time_hours = 10.5;
    stats_.total_distance_miles = 250.0;
    stats_.total_charging_time_hours = 5.0;
    stats_.total_faults = 3;
    stats_.total_passenger_miles = 500.0;
    stats_.flight_count = 8;
    stats_.charge_count = 12;
    
    // Create copy
    evtol::FlightStats copy_stats(stats_);
    
    // Verify all fields are copied correctly
    EXPECT_NEAR_TOLERANCE(copy_stats.total_flight_time_hours, 10.5);
    EXPECT_NEAR_TOLERANCE(copy_stats.total_distance_miles, 250.0);
    EXPECT_NEAR_TOLERANCE(copy_stats.total_charging_time_hours, 5.0);
    EXPECT_EQ(copy_stats.total_faults, 3);
    EXPECT_NEAR_TOLERANCE(copy_stats.total_passenger_miles, 500.0);
    EXPECT_EQ(copy_stats.flight_count, 8);
    EXPECT_EQ(copy_stats.charge_count, 12);
}

// Test FlightStats assignment operator
TEST_F(FlightStatsTest, AssignmentOperator) {
    // Set up original stats
    stats_.total_flight_time_hours = 15.5;
    stats_.total_distance_miles = 350.0;
    stats_.total_charging_time_hours = 7.5;
    stats_.total_faults = 5;
    stats_.total_passenger_miles = 700.0;
    stats_.flight_count = 12;
    stats_.charge_count = 18;
    
    // Create another instance and assign
    evtol::FlightStats assigned_stats;
    assigned_stats = stats_;
    
    // Verify all fields are assigned correctly
    EXPECT_NEAR_TOLERANCE(assigned_stats.total_flight_time_hours, 15.5);
    EXPECT_NEAR_TOLERANCE(assigned_stats.total_distance_miles, 350.0);
    EXPECT_NEAR_TOLERANCE(assigned_stats.total_charging_time_hours, 7.5);
    EXPECT_EQ(assigned_stats.total_faults, 5);
    EXPECT_NEAR_TOLERANCE(assigned_stats.total_passenger_miles, 700.0);
    EXPECT_EQ(assigned_stats.flight_count, 12);
    EXPECT_EQ(assigned_stats.charge_count, 18);
}

// Test self-assignment
TEST_F(FlightStatsTest, SelfAssignment) {
    // Set up stats
    stats_.total_flight_time_hours = 20.0;
    stats_.total_distance_miles = 400.0;
    stats_.flight_count = 10;
    
    // Self-assign
    stats_ = stats_;
    
    // Verify data is unchanged
    EXPECT_NEAR_TOLERANCE(stats_.total_flight_time_hours, 20.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_distance_miles, 400.0);
    EXPECT_EQ(stats_.flight_count, 10);
}

// Test add_flight method
TEST_F(FlightStatsTest, AddFlightSingleFlight) {
    stats_.add_flight(2.5, 150.0, 3);
    
    EXPECT_NEAR_TOLERANCE(stats_.total_flight_time_hours, 2.5);
    EXPECT_NEAR_TOLERANCE(stats_.total_distance_miles, 150.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_passenger_miles, 450.0); // 3 passengers * 150 miles
    EXPECT_EQ(stats_.flight_count, 1);
}

// Test add_flight method with multiple flights
TEST_F(FlightStatsTest, AddFlightMultipleFlights) {
    stats_.add_flight(2.5, 150.0, 3);
    stats_.add_flight(1.5, 100.0, 2);
    stats_.add_flight(3.0, 200.0, 4);
    
    EXPECT_NEAR_TOLERANCE(stats_.total_flight_time_hours, 7.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_distance_miles, 450.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_passenger_miles, 1450.0); // 450 + 200 + 800
    EXPECT_EQ(stats_.flight_count, 3);
}

// Test add_flight with zero values
TEST_F(FlightStatsTest, AddFlightZeroValues) {
    stats_.add_flight(0.0, 0.0, 0);
    
    EXPECT_NEAR_TOLERANCE(stats_.total_flight_time_hours, 0.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_distance_miles, 0.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_passenger_miles, 0.0);
    EXPECT_EQ(stats_.flight_count, 1);
}

// Test add_charge_session method
TEST_F(FlightStatsTest, AddChargeSessionSingle) {
    stats_.add_charge_session(1.5);
    
    EXPECT_NEAR_TOLERANCE(stats_.total_charging_time_hours, 1.5);
    EXPECT_EQ(stats_.charge_count, 1);
}

// Test add_charge_session method with multiple sessions
TEST_F(FlightStatsTest, AddChargeSessionMultiple) {
    stats_.add_charge_session(1.5);
    stats_.add_charge_session(2.0);
    stats_.add_charge_session(0.5);
    
    EXPECT_NEAR_TOLERANCE(stats_.total_charging_time_hours, 4.0);
    EXPECT_EQ(stats_.charge_count, 3);
}

// Test add_charge_session with zero value
TEST_F(FlightStatsTest, AddChargeSessionZero) {
    stats_.add_charge_session(0.0);
    
    EXPECT_NEAR_TOLERANCE(stats_.total_charging_time_hours, 0.0);
    EXPECT_EQ(stats_.charge_count, 1);
}

// Test add_fault method
TEST_F(FlightStatsTest, AddFaultSingle) {
    stats_.add_fault();
    
    EXPECT_EQ(stats_.total_faults, 1);
}

// Test add_fault method multiple times
TEST_F(FlightStatsTest, AddFaultMultiple) {
    stats_.add_fault();
    stats_.add_fault();
    stats_.add_fault();
    
    EXPECT_EQ(stats_.total_faults, 3);
}

// Test avg_flight_time method
TEST_F(FlightStatsTest, AvgFlightTimeWithFlights) {
    stats_.add_flight(2.0, 100.0, 2);
    stats_.add_flight(4.0, 200.0, 3);
    stats_.add_flight(3.0, 150.0, 1);
    
    EXPECT_NEAR_TOLERANCE(stats_.avg_flight_time(), 3.0); // (2 + 4 + 3) / 3 = 3.0
}

// Test avg_flight_time method with no flights
TEST_F(FlightStatsTest, AvgFlightTimeNoFlights) {
    EXPECT_NEAR_TOLERANCE(stats_.avg_flight_time(), 0.0);
}

// Test avg_distance method
TEST_F(FlightStatsTest, AvgDistanceWithFlights) {
    stats_.add_flight(2.0, 100.0, 2);
    stats_.add_flight(4.0, 200.0, 3);
    stats_.add_flight(3.0, 300.0, 1);
    
    EXPECT_NEAR_TOLERANCE(stats_.avg_distance(), 200.0); // (100 + 200 + 300) / 3 = 200.0
}

// Test avg_distance method with no flights
TEST_F(FlightStatsTest, AvgDistanceNoFlights) {
    EXPECT_NEAR_TOLERANCE(stats_.avg_distance(), 0.0);
}

// Test avg_charging_time method
TEST_F(FlightStatsTest, AvgChargingTimeWithSessions) {
    stats_.add_charge_session(1.0);
    stats_.add_charge_session(2.0);
    stats_.add_charge_session(3.0);
    
    EXPECT_NEAR_TOLERANCE(stats_.avg_charging_time(), 2.0); // (1 + 2 + 3) / 3 = 2.0
}

// Test avg_charging_time method with no sessions
TEST_F(FlightStatsTest, AvgChargingTimeNoSessions) {
    EXPECT_NEAR_TOLERANCE(stats_.avg_charging_time(), 0.0);
}

// Test mixed operations
TEST_F(FlightStatsTest, MixedOperations) {
    stats_.add_flight(1.5, 75.0, 2);
    stats_.add_charge_session(1.0);
    stats_.add_fault();
    stats_.add_flight(2.5, 125.0, 3);
    stats_.add_charge_session(1.5);
    stats_.add_fault();
    stats_.add_fault();
    
    EXPECT_NEAR_TOLERANCE(stats_.total_flight_time_hours, 4.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_distance_miles, 200.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_charging_time_hours, 2.5);
    EXPECT_EQ(stats_.total_faults, 3);
    EXPECT_NEAR_TOLERANCE(stats_.total_passenger_miles, 525.0); // 2*75 + 3*125 = 150 + 375
    EXPECT_EQ(stats_.flight_count, 2);
    EXPECT_EQ(stats_.charge_count, 2);
    
    // Test averages
    EXPECT_NEAR_TOLERANCE(stats_.avg_flight_time(), 2.0);
    EXPECT_NEAR_TOLERANCE(stats_.avg_distance(), 100.0);
    EXPECT_NEAR_TOLERANCE(stats_.avg_charging_time(), 1.25);
}

// Test precision with small values
TEST_F(FlightStatsTest, PrecisionWithSmallValues) {
    stats_.add_flight(0.001, 0.01, 1);
    stats_.add_charge_session(0.001);
    
    EXPECT_NEAR_TOLERANCE(stats_.total_flight_time_hours, 0.001);
    EXPECT_NEAR_TOLERANCE(stats_.total_distance_miles, 0.01);
    EXPECT_NEAR_TOLERANCE(stats_.total_charging_time_hours, 0.001);
    EXPECT_NEAR_TOLERANCE(stats_.total_passenger_miles, 0.01);
    EXPECT_EQ(stats_.flight_count, 1);
    EXPECT_EQ(stats_.charge_count, 1);
}

// Test precision with large values
TEST_F(FlightStatsTest, PrecisionWithLargeValues) {
    stats_.add_flight(1000.0, 50000.0, 10);
    stats_.add_charge_session(100.0);
    
    EXPECT_NEAR_TOLERANCE(stats_.total_flight_time_hours, 1000.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_distance_miles, 50000.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_charging_time_hours, 100.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_passenger_miles, 500000.0);
    EXPECT_EQ(stats_.flight_count, 1);
    EXPECT_EQ(stats_.charge_count, 1);
}

// Test edge case: single flight with single passenger
TEST_F(FlightStatsTest, SingleFlightSinglePassenger) {
    stats_.add_flight(1.0, 50.0, 1);
    
    EXPECT_NEAR_TOLERANCE(stats_.total_passenger_miles, 50.0);
    EXPECT_NEAR_TOLERANCE(stats_.avg_flight_time(), 1.0);
    EXPECT_NEAR_TOLERANCE(stats_.avg_distance(), 50.0);
}

// Test edge case: flight with no passengers
TEST_F(FlightStatsTest, FlightWithNoPassengers) {
    stats_.add_flight(2.0, 100.0, 0);
    
    EXPECT_NEAR_TOLERANCE(stats_.total_flight_time_hours, 2.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_distance_miles, 100.0);
    EXPECT_NEAR_TOLERANCE(stats_.total_passenger_miles, 0.0);
    EXPECT_EQ(stats_.flight_count, 1);
}

// Test cumulative behavior over many operations
TEST_F(FlightStatsTest, CumulativeBehavior) {
    const int num_operations = 100;
    
    for (int i = 1; i <= num_operations; ++i) {
        stats_.add_flight(i * 0.1, i * 10.0, i % 5 + 1);
        stats_.add_charge_session(i * 0.05);
        if (i % 10 == 0) {
            stats_.add_fault();
        }
    }
    
    EXPECT_EQ(stats_.flight_count, num_operations);
    EXPECT_EQ(stats_.charge_count, num_operations);
    EXPECT_EQ(stats_.total_faults, 10);
    
    // Check that averages are reasonable
    EXPECT_GT(stats_.avg_flight_time(), 0.0);
    EXPECT_GT(stats_.avg_distance(), 0.0);
    EXPECT_GT(stats_.avg_charging_time(), 0.0);
}

} // namespace evtol_test