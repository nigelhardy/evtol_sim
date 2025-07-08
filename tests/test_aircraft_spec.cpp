#include "test_utilities.h"

namespace evtol_test
{

    class AircraftSpecTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Common test setup
        }
    };

    // Test AircraftSpec construction with valid parameters
    TEST_F(AircraftSpecTest, ValidConstruction)
    {
        evtol::AircraftSpec spec("TestMfg", 100.0, 200.0, 1.5, 4, 0.15);

        EXPECT_EQ(spec.manufacturer, "TestMfg");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 100.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 200.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 1.5);
        EXPECT_EQ(spec.passenger_count, 4);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.15);
    }

    // Test AircraftSpec with minimum values
    TEST_F(AircraftSpecTest, MinimumValues)
    {
        evtol::AircraftSpec spec("", 0.0, 0.0, 0.0, 0, 0.0);

        EXPECT_EQ(spec.manufacturer, "");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 0.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 0.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.0);
        EXPECT_EQ(spec.passenger_count, 0);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.0);
    }

    // Test AircraftSpec with maximum realistic values
    TEST_F(AircraftSpecTest, MaximumValues)
    {
        evtol::AircraftSpec spec("LongManufacturerName", 1000.0, 10000.0, 100.0, 100, 1.0);

        EXPECT_EQ(spec.manufacturer, "LongManufacturerName");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 1000.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 10000.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 100.0);
        EXPECT_EQ(spec.passenger_count, 100);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 1.0);
    }

    // Test AircraftSpec with fractional values
    TEST_F(AircraftSpecTest, FractionalValues)
    {
        evtol::AircraftSpec spec("TestMfg", 123.456, 234.567, 1.234, 3, 0.123);

        EXPECT_EQ(spec.manufacturer, "TestMfg");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 123.456);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 234.567);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 1.234);
        EXPECT_EQ(spec.passenger_count, 3);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.123);
    }

} // namespace evtol_test