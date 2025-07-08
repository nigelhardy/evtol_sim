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

    // Test AircraftSpec immutability (const fields)
    TEST_F(AircraftSpecTest, ImmutabilityTest)
    {
        evtol::AircraftSpec spec("TestMfg", 100.0, 200.0, 1.5, 4, 0.15);

        // These should be const and not modifiable
        // (This is more of a compile-time test, but we can verify values don't change)
        EXPECT_EQ(spec.manufacturer, "TestMfg");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 100.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 200.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 1.5);
        EXPECT_EQ(spec.passenger_count, 4);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.15);
    }

    // Test AircraftSpec with special characters in manufacturer name
    TEST_F(AircraftSpecTest, SpecialCharactersInManufacturer)
    {
        evtol::AircraftSpec spec("Test-Mfg & Co. (2023)", 100.0, 200.0, 1.5, 4, 0.15);

        EXPECT_EQ(spec.manufacturer, "Test-Mfg & Co. (2023)");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 100.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 200.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 1.5);
        EXPECT_EQ(spec.passenger_count, 4);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.15);
    }

    // Test AircraftSpec with very small values
    TEST_F(AircraftSpecTest, VerySmallValues)
    {
        evtol::AircraftSpec spec("TestMfg", 0.001, 0.001, 0.001, 1, 0.001);

        EXPECT_EQ(spec.manufacturer, "TestMfg");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 0.001);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 0.001);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.001);
        EXPECT_EQ(spec.passenger_count, 1);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.001);
    }

    // Test AircraftSpec copy semantics
    TEST_F(AircraftSpecTest, CopySemantics)
    {
        evtol::AircraftSpec spec1("TestMfg", 100.0, 200.0, 1.5, 4, 0.15);
        evtol::AircraftSpec spec2 = spec1;

        // Both specs should have the same values
        EXPECT_EQ(spec1.manufacturer, spec2.manufacturer);
        EXPECT_NEAR_TOLERANCE(spec1.cruise_speed_mph, spec2.cruise_speed_mph);
        EXPECT_NEAR_TOLERANCE(spec1.battery_capacity_kwh, spec2.battery_capacity_kwh);
        EXPECT_NEAR_TOLERANCE(spec1.time_to_charge_hours, spec2.time_to_charge_hours);
        EXPECT_EQ(spec1.passenger_count, spec2.passenger_count);
        EXPECT_NEAR_TOLERANCE(spec1.fault_probability_per_hour, spec2.fault_probability_per_hour);
    }

    // Test AircraftSpec comparison (structural)
    TEST_F(AircraftSpecTest, StructuralComparison)
    {
        evtol::AircraftSpec spec1("TestMfg", 100.0, 200.0, 1.5, 4, 0.15);
        evtol::AircraftSpec spec2("TestMfg", 100.0, 200.0, 1.5, 4, 0.15);
        evtol::AircraftSpec spec3("DifferentMfg", 100.0, 200.0, 1.5, 4, 0.15);

        // spec1 and spec2 should be structurally equivalent
        EXPECT_EQ(spec1.manufacturer, spec2.manufacturer);
        EXPECT_NEAR_TOLERANCE(spec1.cruise_speed_mph, spec2.cruise_speed_mph);
        EXPECT_NEAR_TOLERANCE(spec1.battery_capacity_kwh, spec2.battery_capacity_kwh);
        EXPECT_NEAR_TOLERANCE(spec1.time_to_charge_hours, spec2.time_to_charge_hours);
        EXPECT_EQ(spec1.passenger_count, spec2.passenger_count);
        EXPECT_NEAR_TOLERANCE(spec1.fault_probability_per_hour, spec2.fault_probability_per_hour);

        // spec1 and spec3 should differ in manufacturer
        EXPECT_NE(spec1.manufacturer, spec3.manufacturer);
    }

    // Test AircraftSpec with realistic aircraft values (Alpha-like)
    TEST_F(AircraftSpecTest, RealisticAlphaLikeValues)
    {
        evtol::AircraftSpec spec("AlphaCorp", 120.0, 320.0, 0.6, 4, 0.25);

        EXPECT_EQ(spec.manufacturer, "AlphaCorp");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 120.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 320.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.6);
        EXPECT_EQ(spec.passenger_count, 4);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.25);
    }

    // Test AircraftSpec with realistic aircraft values (Beta-like)
    TEST_F(AircraftSpecTest, RealisticBetaLikeValues)
    {
        evtol::AircraftSpec spec("BetaCorp", 100.0, 100.0, 0.2, 5, 0.10);

        EXPECT_EQ(spec.manufacturer, "BetaCorp");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 100.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 100.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.2);
        EXPECT_EQ(spec.passenger_count, 5);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.10);
    }

    // Test AircraftSpec with realistic aircraft values (Charlie-like)
    TEST_F(AircraftSpecTest, RealisticCharlieLikeValues)
    {
        evtol::AircraftSpec spec("CharlieCorp", 160.0, 220.0, 0.8, 3, 0.05);

        EXPECT_EQ(spec.manufacturer, "CharlieCorp");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 160.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 220.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.8);
        EXPECT_EQ(spec.passenger_count, 3);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.05);
    }

    // Test AircraftSpec with realistic aircraft values (Delta-like)
    TEST_F(AircraftSpecTest, RealisticDeltaLikeValues)
    {
        evtol::AircraftSpec spec("DeltaCorp", 90.0, 120.0, 0.62, 2, 0.22);

        EXPECT_EQ(spec.manufacturer, "DeltaCorp");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 90.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 120.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.62);
        EXPECT_EQ(spec.passenger_count, 2);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.22);
    }

    // Test AircraftSpec with realistic aircraft values (Echo-like)
    TEST_F(AircraftSpecTest, RealisticEchoLikeValues)
    {
        evtol::AircraftSpec spec("EchoCorp", 30.0, 150.0, 0.3, 2, 0.61);

        EXPECT_EQ(spec.manufacturer, "EchoCorp");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 30.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 150.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 0.3);
        EXPECT_EQ(spec.passenger_count, 2);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.61);
    }

    // Test AircraftSpec with negative values (edge case)
    TEST_F(AircraftSpecTest, NegativeValues)
    {
        evtol::AircraftSpec spec("TestMfg", -10.0, -50.0, -1.0, -1, -0.1);

        EXPECT_EQ(spec.manufacturer, "TestMfg");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, -10.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, -50.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, -1.0);
        EXPECT_EQ(spec.passenger_count, -1);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, -0.1);
    }

    // Test AircraftSpec with extreme values
    TEST_F(AircraftSpecTest, ExtremeValues)
    {
        evtol::AircraftSpec spec("TestMfg", 1e6, 1e6, 1e6, 1000000, 1e6);

        EXPECT_EQ(spec.manufacturer, "TestMfg");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 1e6);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 1e6);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 1e6);
        EXPECT_EQ(spec.passenger_count, 1000000);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 1e6);
    }

    // Test AircraftSpec with precision values
    TEST_F(AircraftSpecTest, PrecisionValues)
    {
        evtol::AircraftSpec spec("TestMfg", 123.456789, 234.567890, 1.234567, 3, 0.123456);

        EXPECT_EQ(spec.manufacturer, "TestMfg");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 123.456789);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 234.567890);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 1.234567);
        EXPECT_EQ(spec.passenger_count, 3);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.123456);
    }

    // Test AircraftSpec with empty manufacturer name
    TEST_F(AircraftSpecTest, EmptyManufacturerName)
    {
        evtol::AircraftSpec spec("", 100.0, 200.0, 1.5, 4, 0.15);

        EXPECT_EQ(spec.manufacturer, "");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 100.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 200.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 1.5);
        EXPECT_EQ(spec.passenger_count, 4);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.15);
    }

    // Test AircraftSpec with single character manufacturer name
    TEST_F(AircraftSpecTest, SingleCharacterManufacturerName)
    {
        evtol::AircraftSpec spec("X", 100.0, 200.0, 1.5, 4, 0.15);

        EXPECT_EQ(spec.manufacturer, "X");
        EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, 100.0);
        EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, 200.0);
        EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, 1.5);
        EXPECT_EQ(spec.passenger_count, 4);
        EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, 0.15);
    }

} // namespace evtol_test