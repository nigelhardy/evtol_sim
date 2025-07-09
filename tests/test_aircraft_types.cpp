#include "test_utilities.h"

namespace evtol_test
{

    // Test fixture for Aircraft base class functionality
    class AircraftBaseTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Common setup for all aircraft tests
            test_id_ = DEFAULT_AIRCRAFT_ID;
        }

        int test_id_;
    };

    // Test AlphaAircraft construction and basic properties
    TEST_F(AircraftBaseTest, AlphaAircraftConstruction)
    {
        evtol::AlphaAircraft aircraft(test_id_);

        EXPECT_EQ(aircraft.get_id(), test_id_);
        EXPECT_EQ(aircraft.get_type(), evtol::AircraftType::ALPHA);
        EXPECT_EQ(aircraft.get_manufacturer(), "Alpha");
        EXPECT_EQ(aircraft.get_passenger_count(), 4);
        EXPECT_NEAR_TOLERANCE(aircraft.get_charge_time_hours(), 0.6);
        EXPECT_NEAR_TOLERANCE(aircraft.get_battery_level(), 1.0);
    }

    // Test AlphaAircraft specifications
    TEST_F(AircraftBaseTest, AlphaAircraftSpecifications)
    {
        evtol::AlphaAircraft aircraft(test_id_);
        const auto &spec = aircraft.get_spec();

        EXPECT_EQ(spec.manufacturer, "Alpha");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 120.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 320.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.6);
        EXPECT_EQ(spec.passenger_count, 4);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.25);
    }

    // Test AlphaAircraft flight calculations
    TEST_F(AircraftBaseTest, AlphaAircraftFlightCalculations)
    {
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
    TEST_F(AircraftBaseTest, BetaAircraftConstruction)
    {
        evtol::BetaAircraft aircraft(test_id_);

        EXPECT_EQ(aircraft.get_id(), test_id_);
        EXPECT_EQ(aircraft.get_type(), evtol::AircraftType::BETA);
        EXPECT_EQ(aircraft.get_manufacturer(), "Beta");
        EXPECT_EQ(aircraft.get_passenger_count(), 5);
        EXPECT_NEAR_TOLERANCE(aircraft.get_charge_time_hours(), 0.2);
        EXPECT_NEAR_TOLERANCE(aircraft.get_battery_level(), 1.0);
    }

    // Test BetaAircraft specifications
    TEST_F(AircraftBaseTest, BetaAircraftSpecifications)
    {
        evtol::BetaAircraft aircraft(test_id_);
        const auto &spec = aircraft.get_spec();

        EXPECT_EQ(spec.manufacturer, "Beta");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 100.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 100.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.2);
        EXPECT_EQ(spec.passenger_count, 5);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.10);
    }

    // Test BetaAircraft flight calculations
    TEST_F(AircraftBaseTest, BetaAircraftFlightCalculations)
    {
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
    TEST_F(AircraftBaseTest, CharlieAircraftConstruction)
    {
        evtol::CharlieAircraft aircraft(test_id_);

        EXPECT_EQ(aircraft.get_id(), test_id_);
        EXPECT_EQ(aircraft.get_type(), evtol::AircraftType::CHARLIE);
        EXPECT_EQ(aircraft.get_manufacturer(), "Charlie");
        EXPECT_EQ(aircraft.get_passenger_count(), 3);
        EXPECT_NEAR_TOLERANCE(aircraft.get_charge_time_hours(), 0.8);
        EXPECT_NEAR_TOLERANCE(aircraft.get_battery_level(), 1.0);
    }

    // Test CharlieAircraft specifications
    TEST_F(AircraftBaseTest, CharlieAircraftSpecifications)
    {
        evtol::CharlieAircraft aircraft(test_id_);
        const auto &spec = aircraft.get_spec();

        EXPECT_EQ(spec.manufacturer, "Charlie");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 160.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 220.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.8);
        EXPECT_EQ(spec.passenger_count, 3);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.05);
    }

    // Test CharlieAircraft flight calculations
    TEST_F(AircraftBaseTest, CharlieAircraftFlightCalculations)
    {
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
    TEST_F(AircraftBaseTest, DeltaAircraftConstruction)
    {
        evtol::DeltaAircraft aircraft(test_id_);

        EXPECT_EQ(aircraft.get_id(), test_id_);
        EXPECT_EQ(aircraft.get_type(), evtol::AircraftType::DELTA);
        EXPECT_EQ(aircraft.get_manufacturer(), "Delta");
        EXPECT_EQ(aircraft.get_passenger_count(), 2);
        EXPECT_NEAR_TOLERANCE(aircraft.get_charge_time_hours(), 0.62);
        EXPECT_NEAR_TOLERANCE(aircraft.get_battery_level(), 1.0);
    }

    // Test DeltaAircraft specifications
    TEST_F(AircraftBaseTest, DeltaAircraftSpecifications)
    {
        evtol::DeltaAircraft aircraft(test_id_);
        const auto &spec = aircraft.get_spec();

        EXPECT_EQ(spec.manufacturer, "Delta");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 90.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 120.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.62);
        EXPECT_EQ(spec.passenger_count, 2);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.22);
    }

    // Test DeltaAircraft flight calculations
    TEST_F(AircraftBaseTest, DeltaAircraftFlightCalculations)
    {
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
    TEST_F(AircraftBaseTest, EchoAircraftConstruction)
    {
        evtol::EchoAircraft aircraft(test_id_);

        EXPECT_EQ(aircraft.get_id(), test_id_);
        EXPECT_EQ(aircraft.get_type(), evtol::AircraftType::ECHO);
        EXPECT_EQ(aircraft.get_manufacturer(), "Echo");
        EXPECT_EQ(aircraft.get_passenger_count(), 2);
        EXPECT_NEAR_TOLERANCE(aircraft.get_charge_time_hours(), 0.3);
        EXPECT_NEAR_TOLERANCE(aircraft.get_battery_level(), 1.0);
    }

    // Test EchoAircraft specifications
    TEST_F(AircraftBaseTest, EchoAircraftSpecifications)
    {
        evtol::EchoAircraft aircraft(test_id_);
        const auto &spec = aircraft.get_spec();

        EXPECT_EQ(spec.manufacturer, "Echo");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 30.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 150.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.3);
        EXPECT_EQ(spec.passenger_count, 2);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.61);
    }

    // Test EchoAircraft flight calculations
    TEST_F(AircraftBaseTest, EchoAircraftFlightCalculations)
    {
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
    TEST_F(AircraftBaseTest, BatteryDischargeFunctionality)
    {
        std::vector<std::unique_ptr<evtol::AircraftBase>> aircraft_list;
        aircraft_list.emplace_back(std::make_unique<evtol::AlphaAircraft>(0));
        aircraft_list.emplace_back(std::make_unique<evtol::BetaAircraft>(1));
        aircraft_list.emplace_back(std::make_unique<evtol::CharlieAircraft>(2));
        aircraft_list.emplace_back(std::make_unique<evtol::DeltaAircraft>(3));
        aircraft_list.emplace_back(std::make_unique<evtol::EchoAircraft>(4));

        for (auto &aircraft : aircraft_list)
        {
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

    // Test polymorphic behavior through base class interface
    TEST_F(AircraftBaseTest, PolymorphicBehavior)
    {
        std::vector<std::unique_ptr<evtol::AircraftBase>> aircraft_list;
        aircraft_list.emplace_back(std::make_unique<evtol::AlphaAircraft>(0));
        aircraft_list.emplace_back(std::make_unique<evtol::BetaAircraft>(1));
        aircraft_list.emplace_back(std::make_unique<evtol::CharlieAircraft>(2));
        aircraft_list.emplace_back(std::make_unique<evtol::DeltaAircraft>(3));
        aircraft_list.emplace_back(std::make_unique<evtol::EchoAircraft>(4));

        // Test that each aircraft type has different characteristics
        std::vector<evtol::AircraftType> expected_types = {
            evtol::AircraftType::ALPHA,
            evtol::AircraftType::BETA,
            evtol::AircraftType::CHARLIE,
            evtol::AircraftType::DELTA,
            evtol::AircraftType::ECHO};

        for (size_t i = 0; i < aircraft_list.size(); ++i)
        {
            auto &aircraft = aircraft_list[i];

            EXPECT_EQ(aircraft->get_type(), expected_types[i]);
            EXPECT_EQ(aircraft->get_id(), static_cast<int>(i));

            // Each aircraft should have different flight characteristics
            double flight_time = aircraft->get_flight_time_hours();
            double flight_distance = aircraft->get_flight_distance_miles();

            EXPECT_GT(flight_time, 0.0);
            EXPECT_GT(flight_distance, 0.0);
        }
    }

    // Test that all aircraft types have unique specifications
    TEST_F(AircraftBaseTest, UniqueSpecifications)
    {
        evtol::AlphaAircraft alpha(0);
        evtol::BetaAircraft beta(1);
        evtol::CharlieAircraft charlie(2);
        evtol::DeltaAircraft delta(3);
        evtol::EchoAircraft echo(4);

        std::vector<evtol::AircraftBase *> aircraft_list = {&alpha, &beta, &charlie, &delta, &echo};

        // Check that each aircraft has unique cruise speed
        std::set<double> cruise_speeds;
        for (auto *aircraft : aircraft_list)
        {
            cruise_speeds.insert(aircraft->get_spec().cruise_speed_mph);
        }
        EXPECT_EQ(cruise_speeds.size(), 5); // All should be unique

        // Check that each aircraft has unique battery capacity
        std::set<double> battery_capacities;
        for (auto *aircraft : aircraft_list)
        {
            battery_capacities.insert(aircraft->get_spec().battery_capacity_kwh);
        }
        EXPECT_EQ(battery_capacities.size(), 5); // All should be unique

        // Check that each aircraft has unique charge time
        std::set<double> charge_times;
        for (auto *aircraft : aircraft_list)
        {
            charge_times.insert(aircraft->get_spec().time_to_charge_hours);
        }
        EXPECT_EQ(charge_times.size(), 5); // All should be unique

        // Check that each aircraft has unique passenger count
        std::set<int> passenger_counts;
        for (auto *aircraft : aircraft_list)
        {
            passenger_counts.insert(aircraft->get_spec().passenger_count);
        }
        EXPECT_GE(passenger_counts.size(), 3); // At least 3 different passenger counts

        // Check that each aircraft has unique fault probability
        std::set<double> fault_probabilities;
        for (auto *aircraft : aircraft_list)
        {
            fault_probabilities.insert(aircraft->get_spec().fault_probability_per_hour);
        }
        EXPECT_EQ(fault_probabilities.size(), 5); // All should be unique
    }

    // Test static methods for aircraft types
    TEST_F(AircraftBaseTest, StaticMethods)
    {
        // Test Alpha static methods
        EXPECT_EQ(evtol::AlphaAircraft::get_aircraft_type(), evtol::AircraftType::ALPHA);
        const auto &alpha_spec = evtol::AlphaAircraft::get_aircraft_spec();
        EXPECT_EQ(alpha_spec.manufacturer, "Alpha");

        // Test Beta static methods
        EXPECT_EQ(evtol::BetaAircraft::get_aircraft_type(), evtol::AircraftType::BETA);
        const auto &beta_spec = evtol::BetaAircraft::get_aircraft_spec();
        EXPECT_EQ(beta_spec.manufacturer, "Beta");

        // Test Charlie static methods
        EXPECT_EQ(evtol::CharlieAircraft::get_aircraft_type(), evtol::AircraftType::CHARLIE);
        const auto &charlie_spec = evtol::CharlieAircraft::get_aircraft_spec();
        EXPECT_EQ(charlie_spec.manufacturer, "Charlie");

        // Test Delta static methods
        EXPECT_EQ(evtol::DeltaAircraft::get_aircraft_type(), evtol::AircraftType::DELTA);
        const auto &delta_spec = evtol::DeltaAircraft::get_aircraft_spec();
        EXPECT_EQ(delta_spec.manufacturer, "Delta");

        // Test Echo static methods
        EXPECT_EQ(evtol::EchoAircraft::get_aircraft_type(), evtol::AircraftType::ECHO);
        const auto &echo_spec = evtol::EchoAircraft::get_aircraft_spec();
        EXPECT_EQ(echo_spec.manufacturer, "Echo");
    }

    // Test aircraft with different IDs
    TEST_F(AircraftBaseTest, DifferentIDs)
    {
        evtol::AlphaAircraft aircraft1(100);
        evtol::AlphaAircraft aircraft2(200);
        evtol::BetaAircraft aircraft3(300);

        EXPECT_EQ(aircraft1.get_id(), 100);
        EXPECT_EQ(aircraft2.get_id(), 200);
        EXPECT_EQ(aircraft3.get_id(), 300);

        // Same type should have same specifications
        EXPECT_EQ(aircraft1.get_type(), aircraft2.get_type());
        EXPECT_EQ(aircraft1.get_spec().manufacturer, aircraft2.get_spec().manufacturer);

        // Different types should have different specifications
        EXPECT_NE(aircraft1.get_type(), aircraft3.get_type());
        EXPECT_NE(aircraft1.get_spec().manufacturer, aircraft3.get_spec().manufacturer);
    }

    // Test battery cycling (discharge and charge multiple times)
    TEST_F(AircraftBaseTest, BatteryCycling)
    {
        evtol::AlphaAircraft aircraft(test_id_);

        for (int cycle = 0; cycle < 10; ++cycle)
        {
            // Should start or return to fully charged
            EXPECT_NEAR_TOLERANCE(aircraft.get_battery_level(), 1.0);

            // Discharge
            aircraft.discharge_battery();
            EXPECT_NEAR_TOLERANCE(aircraft.get_battery_level(), 0.0);

            // Charge
            aircraft.charge_battery();
            EXPECT_NEAR_TOLERANCE(aircraft.get_battery_level(), 1.0);
        }
    }

    // Test flight calculations are consistent across multiple calls
    TEST_F(AircraftBaseTest, FlightCalculationConsistency)
    {
        evtol::AlphaAircraft aircraft(test_id_);

        double first_flight_time = aircraft.get_flight_time_hours();
        double first_flight_distance = aircraft.get_flight_distance_miles();

        // Multiple calls should return same values
        for (int i = 0; i < 10; ++i)
        {
            EXPECT_NEAR_TOLERANCE(aircraft.get_flight_time_hours(), first_flight_time);
            EXPECT_NEAR_TOLERANCE(aircraft.get_flight_distance_miles(), first_flight_distance);
        }
    }

    // Test that specifications are read-only (const)
    TEST_F(AircraftBaseTest, SpecificationsReadOnly)
    {
        evtol::AlphaAircraft aircraft(test_id_);
        const auto &spec1 = aircraft.get_spec();
        const auto &spec2 = aircraft.get_spec();

        // Should be the same object (reference to static)
        EXPECT_EQ(&spec1, &spec2);

        // Values should be identical
        EXPECT_EQ(spec1.manufacturer, spec2.manufacturer);
        EXPECT_NEAR_TOLERANCE(spec1.cruise_speed_mph, spec2.cruise_speed_mph);
        EXPECT_NEAR_TOLERANCE(spec1.battery_capacity_kwh, spec2.battery_capacity_kwh);
        EXPECT_NEAR_TOLERANCE(spec1.time_to_charge_hours, spec2.time_to_charge_hours);
        EXPECT_EQ(spec1.passenger_count, spec2.passenger_count);
        EXPECT_NEAR_TOLERANCE(spec1.fault_probability_per_hour, spec2.fault_probability_per_hour);
    }

    // Test inheritance hierarchy
    TEST_F(AircraftBaseTest, InheritanceHierarchy)
    {
        evtol::AlphaAircraft alpha(0);
        evtol::BetaAircraft beta(1);
        evtol::CharlieAircraft charlie(2);
        evtol::DeltaAircraft delta(3);
        evtol::EchoAircraft echo(4);

        // All should be instances of AircraftBase
        evtol::AircraftBase *alpha_base = &alpha;
        evtol::AircraftBase *beta_base = &beta;
        evtol::AircraftBase *charlie_base = &charlie;
        evtol::AircraftBase *delta_base = &delta;
        evtol::AircraftBase *echo_base = &echo;

        EXPECT_NE(alpha_base, nullptr);
        EXPECT_NE(beta_base, nullptr);
        EXPECT_NE(charlie_base, nullptr);
        EXPECT_NE(delta_base, nullptr);
        EXPECT_NE(echo_base, nullptr);

        // Virtual function calls should work correctly
        EXPECT_EQ(alpha_base->get_type(), evtol::AircraftType::ALPHA);
        EXPECT_EQ(beta_base->get_type(), evtol::AircraftType::BETA);
        EXPECT_EQ(charlie_base->get_type(), evtol::AircraftType::CHARLIE);
        EXPECT_EQ(delta_base->get_type(), evtol::AircraftType::DELTA);
        EXPECT_EQ(echo_base->get_type(), evtol::AircraftType::ECHO);
    }

} // namespace evtol_test