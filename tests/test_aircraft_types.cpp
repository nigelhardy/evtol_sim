#include "test_utilities.h"

namespace evtol_test {

// Test fixture for Aircraft base class functionality
class AircraftBaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for all aircraft tests
        test_id_ = DEFAULT_AIRCRAFT_ID;
    }
    
    int test_id_;
};

// Test AlphaAircraft construction and basic properties
TEST_F(AircraftBaseTest, AlphaAircraftConstruction) {
    evtol::AlphaAircraft aircraft(test_id_);
    
    EXPECT_EQ(aircraft.get_id(), test_id_);
    EXPECT_EQ(aircraft.get_type(), evtol::AircraftType::ALPHA);
    EXPECT_EQ(aircraft.get_manufacturer(), "Alpha");
    EXPECT_EQ(aircraft.get_passenger_count(), 4);
    EXPECT_NEAR_TOLERANCE(aircraft.get_charge_time_hours(), 0.6);
    EXPECT_NEAR_TOLERANCE(aircraft.get_battery_level(), 1.0);
}

// Test AlphaAircraft specifications
TEST_F(AircraftBaseTest, AlphaAircraftSpecifications) {
    evtol::AlphaAircraft aircraft(test_id_);
    const auto& spec = aircraft.get_spec();
    
    EXPECT_EQ(spec.manufacturer, "Alpha");
    EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 120.0);
    EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 320.0);
    EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.6);
    EXPECT_EQ(spec.passenger_count, 4);
    EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.25);
}

// Test AlphaAircraft flight calculations
TEST_F(AircraftBaseTest, AlphaAircraftFlightCalculations) {
    evtol::AlphaAircraft aircraft(test_id_);
    
    // Flight time calculation: battery_capacity / (cruise_speed * energy_consumption)
    // energy_consumption = 1.6 kWh/mile for Alpha
    // flight_time = 320 / (120 * 1.6) = 320 / 192 = 1.6667 hours
    double expected_flight_time = 320.0 / (120.0 * 1.6);
    EXPECT_NEAR_TOLERANCE(aircraft.get_flight_time_hours(), expected_flight_time);
    
    // Flight distance = flight_time * cruise_speed
    double expected_distance = expected_flight_time * 120.0;
    EXPECT_NEAR_TOLERANCE(aircraft.get_flight_distance_miles(), expected_distance);
}

// Test BetaAircraft construction and basic properties
TEST_F(AircraftBaseTest, BetaAircraftConstruction) {
    evtol::BetaAircraft aircraft(test_id_);
    
    EXPECT_EQ(aircraft.get_id(), test_id_);
    EXPECT_EQ(aircraft.get_type(), evtol::AircraftType::BETA);
    EXPECT_EQ(aircraft.get_manufacturer(), "Beta");
    EXPECT_EQ(aircraft.get_passenger_count(), 5);
    EXPECT_NEAR_TOLERANCE(aircraft.get_charge_time_hours(), 0.2);
    EXPECT_NEAR_TOLERANCE(aircraft.get_battery_level(), 1.0);
}

// Test BetaAircraft specifications
TEST_F(AircraftBaseTest, BetaAircraftSpecifications) {
    evtol::BetaAircraft aircraft(test_id_);
    const auto& spec = aircraft.get_spec();
    
    EXPECT_EQ(spec.manufacturer, "Beta");
    EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 100.0);
    EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 100.0);
    EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.2);
    EXPECT_EQ(spec.passenger_count, 5);
    EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.10);
}

// Test BetaAircraft flight calculations
TEST_F(AircraftBaseTest, BetaAircraftFlightCalculations) {
    evtol::BetaAircraft aircraft(test_id_);
    
    // Flight time calculation: battery_capacity / (cruise_speed * energy_consumption)
    // energy_consumption = 1.5 kWh/mile for Beta
    // flight_time = 100 / (100 * 1.5) = 100 / 150 = 0.6667 hours
    double expected_flight_time = 100.0 / (100.0 * 1.5);
    EXPECT_NEAR_TOLERANCE(aircraft.get_flight_time_hours(), expected_flight_time);
    
    // Flight distance = flight_time * cruise_speed
    double expected_distance = expected_flight_time * 100.0;
    EXPECT_NEAR_TOLERANCE(aircraft.get_flight_distance_miles(), expected_distance);
}

// Test CharlieAircraft construction and basic properties
TEST_F(AircraftBaseTest, CharlieAircraftConstruction) {
    evtol::CharlieAircraft aircraft(test_id_);
    
    EXPECT_EQ(aircraft.get_id(), test_id_);
    EXPECT_EQ(aircraft.get_type(), evtol::AircraftType::CHARLIE);
    EXPECT_EQ(aircraft.get_manufacturer(), "Charlie");
    EXPECT_EQ(aircraft.get_passenger_count(), 3);
    EXPECT_NEAR_TOLERANCE(aircraft.get_charge_time_hours(), 0.8);
    EXPECT_NEAR_TOLERANCE(aircraft.get_battery_level(), 1.0);
}

// Test CharlieAircraft specifications
TEST_F(AircraftBaseTest, CharlieAircraftSpecifications) {
    evtol::CharlieAircraft aircraft(test_id_);
    const auto& spec = aircraft.get_spec();
    
    EXPECT_EQ(spec.manufacturer, "Charlie");
    EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 160.0);
    EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 220.0);
    EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.8);
    EXPECT_EQ(spec.passenger_count, 3);
    EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.05);
}

// Test CharlieAircraft flight calculations
TEST_F(AircraftBaseTest, CharlieAircraftFlightCalculations) {
    evtol::CharlieAircraft aircraft(test_id_);
    
    // Flight time calculation: battery_capacity / (cruise_speed * energy_consumption)
    // energy_consumption = 2.2 kWh/mile for Charlie
    // flight_time = 220 / (160 * 2.2) = 220 / 352 = 0.625 hours
    double expected_flight_time = 220.0 / (160.0 * 2.2);
    EXPECT_NEAR_TOLERANCE(aircraft.get_flight_time_hours(), expected_flight_time);
    
    // Flight distance = flight_time * cruise_speed
    double expected_distance = expected_flight_time * 160.0;
    EXPECT_NEAR_TOLERANCE(aircraft.get_flight_distance_miles(), expected_distance);
}

// Test DeltaAircraft construction and basic properties
TEST_F(AircraftBaseTest, DeltaAircraftConstruction) {
    evtol::DeltaAircraft aircraft(test_id_);
    
    EXPECT_EQ(aircraft.get_id(), test_id_);
    EXPECT_EQ(aircraft.get_type(), evtol::AircraftType::DELTA);
    EXPECT_EQ(aircraft.get_manufacturer(), "Delta");
    EXPECT_EQ(aircraft.get_passenger_count(), 2);
    EXPECT_NEAR_TOLERANCE(aircraft.get_charge_time_hours(), 0.62);
    EXPECT_NEAR_TOLERANCE(aircraft.get_battery_level(), 1.0);
}

// Test DeltaAircraft specifications
TEST_F(AircraftBaseTest, DeltaAircraftSpecifications) {
    evtol::DeltaAircraft aircraft(test_id_);
    const auto& spec = aircraft.get_spec();
    
    EXPECT_EQ(spec.manufacturer, "Delta");
    EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 90.0);
    EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 120.0);
    EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.62);
    EXPECT_EQ(spec.passenger_count, 2);
    EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.22);
}

// Test DeltaAircraft flight calculations
TEST_F(AircraftBaseTest, DeltaAircraftFlightCalculations) {
    evtol::DeltaAircraft aircraft(test_id_);
    
    // Flight time calculation: battery_capacity / (cruise_speed * energy_consumption)
    // energy_consumption = 0.8 kWh/mile for Delta
    // flight_time = 120 / (90 * 0.8) = 120 / 72 = 1.6667 hours
    double expected_flight_time = 120.0 / (90.0 * 0.8);
    EXPECT_NEAR_TOLERANCE(aircraft.get_flight_time_hours(), expected_flight_time);
    
    // Flight distance = flight_time * cruise_speed
    double expected_distance = expected_flight_time * 90.0;
    EXPECT_NEAR_TOLERANCE(aircraft.get_flight_distance_miles(), expected_distance);
}

// Test EchoAircraft construction and basic properties
TEST_F(AircraftBaseTest, EchoAircraftConstruction) {
    evtol::EchoAircraft aircraft(test_id_);
    
    EXPECT_EQ(aircraft.get_id(), test_id_);
    EXPECT_EQ(aircraft.get_type(), evtol::AircraftType::ECHO);
    EXPECT_EQ(aircraft.get_manufacturer(), "Echo");
    EXPECT_EQ(aircraft.get_passenger_count(), 2);
    EXPECT_NEAR_TOLERANCE(aircraft.get_charge_time_hours(), 0.3);
    EXPECT_NEAR_TOLERANCE(aircraft.get_battery_level(), 1.0);
}

// Test EchoAircraft specifications
TEST_F(AircraftBaseTest, EchoAircraftSpecifications) {
    evtol::EchoAircraft aircraft(test_id_);
    const auto& spec = aircraft.get_spec();
    
    EXPECT_EQ(spec.manufacturer, "Echo");
    EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 30.0);
    EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 150.0);
    EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.3);
    EXPECT_EQ(spec.passenger_count, 2);
    EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.61);
}

// Test EchoAircraft flight calculations
TEST_F(AircraftBaseTest, EchoAircraftFlightCalculations) {
    evtol::EchoAircraft aircraft(test_id_);
    
    // Flight time calculation: battery_capacity / (cruise_speed * energy_consumption)
    // energy_consumption = 5.8 kWh/mile for Echo
    // flight_time = 150 / (30 * 5.8) = 150 / 174 = 0.8621 hours
    double expected_flight_time = 150.0 / (30.0 * 5.8);
    EXPECT_NEAR_TOLERANCE(aircraft.get_flight_time_hours(), expected_flight_time);
    
    // Flight distance = flight_time * cruise_speed
    double expected_distance = expected_flight_time * 30.0;
    EXPECT_NEAR_TOLERANCE(aircraft.get_flight_distance_miles(), expected_distance);
}

// Test battery discharge functionality for all aircraft types
TEST_F(AircraftBaseTest, BatteryDischargeFunctionality) {
    std::vector<std::unique_ptr<evtol::AircraftBase>> aircraft_list;
    aircraft_list.emplace_back(std::make_unique<evtol::AlphaAircraft>(0));
    aircraft_list.emplace_back(std::make_unique<evtol::BetaAircraft>(1));
    aircraft_list.emplace_back(std::make_unique<evtol::CharlieAircraft>(2));
    aircraft_list.emplace_back(std::make_unique<evtol::DeltaAircraft>(3));
    aircraft_list.emplace_back(std::make_unique<evtol::EchoAircraft>(4));
    
    for (auto& aircraft : aircraft_list) {
        // Initially fully charged
        EXPECT_NEAR_TOLERANCE(aircraft->get_battery_level(), 1.0);
        
        // Discharge battery
        aircraft->discharge_battery();
        EXPECT_NEAR_TOLERANCE(aircraft->get_battery_level(), 0.0);
        
        // Charge battery
        aircraft->charge_battery();
        EXPECT_NEAR_TOLERANCE(aircraft->get_battery_level(), 1.0);
    }
}

// Test fault checking functionality for all aircraft types
TEST_F(AircraftBaseTest, FaultCheckingFunctionality) {
    std::vector<std::unique_ptr<evtol::AircraftBase>> aircraft_list;
    aircraft_list.emplace_back(std::make_unique<evtol::AlphaAircraft>(0));
    aircraft_list.emplace_back(std::make_unique<evtol::BetaAircraft>(1));
    aircraft_list.emplace_back(std::make_unique<evtol::CharlieAircraft>(2));
    aircraft_list.emplace_back(std::make_unique<evtol::DeltaAircraft>(3));
    aircraft_list.emplace_back(std::make_unique<evtol::EchoAircraft>(4));
    
    for (auto& aircraft : aircraft_list) {
        // Test fault checking - should be probabilistic
        // With very short flight time, fault probability should be very low
        double short_flight_time = 0.001;
        
        // Run multiple checks with short flight time
        int fault_count = 0;
        const int num_checks = 1000;
        
        for (int i = 0; i < num_checks; ++i) {
            if (aircraft->check_fault_during_flight(short_flight_time)) {
                fault_count++;
            }
        }
        
        // With very short flight time, fault rate should be very low
        double fault_rate = static_cast<double>(fault_count) / num_checks;
        EXPECT_LT(fault_rate, 0.1); // Should be less than 10%
    }
}

} // namespace evtol_test