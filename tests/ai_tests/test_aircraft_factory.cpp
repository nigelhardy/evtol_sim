#include "test_utilities.h"

namespace evtol_test {

class AircraftFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for factory tests
    }
};

// Test basic fleet creation
TEST_F(AircraftFactoryTest, BasicFleetCreation) {
    const int fleet_size = 5;
    auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);
    
    EXPECT_EQ(fleet.size(), static_cast<size_t>(fleet_size));
    
    // Verify each aircraft is created and has unique ID
    for (size_t i = 0; i < fleet.size(); ++i) {
        EXPECT_NE(fleet[i], nullptr);
        EXPECT_EQ(fleet[i]->get_id(), static_cast<int>(i));
    }
}

// Test fleet creation with different sizes
TEST_F(AircraftFactoryTest, DifferentFleetSizes) {
    std::vector<int> test_sizes = {1, 5, 10, 15, 20, 25, 50};
    
    for (int size : test_sizes) {
        auto fleet = evtol::AircraftFactory<>::create_fleet(size);
        EXPECT_EQ(fleet.size(), static_cast<size_t>(size));
        
        // Verify all aircraft are created
        for (const auto& aircraft : fleet) {
            EXPECT_NE(aircraft, nullptr);
        }
    }
}

// Test aircraft type distribution (round-robin pattern)
TEST_F(AircraftFactoryTest, AircraftTypeDistribution) {
    const int fleet_size = 15; // Multiple of 5 for even distribution
    auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);
    
    // Expected pattern: Alpha, Beta, Charlie, Delta, Echo, Alpha, Beta, ...
    std::vector<evtol::AircraftType> expected_types = {
        evtol::AircraftType::ALPHA,   // i % 5 == 0
        evtol::AircraftType::BETA,    // i % 5 == 1
        evtol::AircraftType::CHARLIE, // i % 5 == 2
        evtol::AircraftType::DELTA,   // i % 5 == 3
        evtol::AircraftType::ECHO,    // i % 5 == 4
        evtol::AircraftType::ALPHA,   // i % 5 == 0
        evtol::AircraftType::BETA,    // i % 5 == 1
        evtol::AircraftType::CHARLIE, // i % 5 == 2
        evtol::AircraftType::DELTA,   // i % 5 == 3
        evtol::AircraftType::ECHO,    // i % 5 == 4
        evtol::AircraftType::ALPHA,   // i % 5 == 0
        evtol::AircraftType::BETA,    // i % 5 == 1
        evtol::AircraftType::CHARLIE, // i % 5 == 2
        evtol::AircraftType::DELTA,   // i % 5 == 3
        evtol::AircraftType::ECHO     // i % 5 == 4
    };
    
    for (size_t i = 0; i < fleet.size(); ++i) {
        EXPECT_EQ(fleet[i]->get_type(), expected_types[i]);
    }
}

// Test aircraft type distribution with non-multiple of 5
TEST_F(AircraftFactoryTest, AircraftTypeDistributionNonMultiple) {
    const int fleet_size = 13;
    auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);
    
    // Count each aircraft type
    std::map<evtol::AircraftType, int> type_counts;
    for (const auto& aircraft : fleet) {
        type_counts[aircraft->get_type()]++;
    }
    
    // For 13 aircraft: Alpha=3, Beta=3, Charlie=3, Delta=2, Echo=2
    EXPECT_EQ(type_counts[evtol::AircraftType::ALPHA], 3);
    EXPECT_EQ(type_counts[evtol::AircraftType::BETA], 3);
    EXPECT_EQ(type_counts[evtol::AircraftType::CHARLIE], 3);
    EXPECT_EQ(type_counts[evtol::AircraftType::DELTA], 2);
    EXPECT_EQ(type_counts[evtol::AircraftType::ECHO], 2);
}

// Test empty fleet creation
TEST_F(AircraftFactoryTest, EmptyFleetCreation) {
    auto fleet = evtol::AircraftFactory<>::create_fleet(0);
    
    EXPECT_EQ(fleet.size(), 0);
    EXPECT_TRUE(fleet.empty());
}

// Test single aircraft fleet
TEST_F(AircraftFactoryTest, SingleAircraftFleet) {
    auto fleet = evtol::AircraftFactory<>::create_fleet(1);
    
    EXPECT_EQ(fleet.size(), 1);
    EXPECT_NE(fleet[0], nullptr);
    EXPECT_EQ(fleet[0]->get_id(), 0);
    EXPECT_EQ(fleet[0]->get_type(), evtol::AircraftType::ALPHA);
}

// Test large fleet creation
TEST_F(AircraftFactoryTest, LargeFleetCreation) {
    const int large_fleet_size = 1000;
    auto fleet = evtol::AircraftFactory<>::create_fleet(large_fleet_size);
    
    EXPECT_EQ(fleet.size(), static_cast<size_t>(large_fleet_size));
    
    // Verify all aircraft are created with correct IDs
    for (size_t i = 0; i < fleet.size(); ++i) {
        EXPECT_NE(fleet[i], nullptr);
        EXPECT_EQ(fleet[i]->get_id(), static_cast<int>(i));
    }
    
    // Verify type distribution is even for large fleet
    std::map<evtol::AircraftType, int> type_counts;
    for (const auto& aircraft : fleet) {
        type_counts[aircraft->get_type()]++;
    }
    
    // Each type should have exactly 200 aircraft (1000 / 5)
    EXPECT_EQ(type_counts[evtol::AircraftType::ALPHA], 200);
    EXPECT_EQ(type_counts[evtol::AircraftType::BETA], 200);
    EXPECT_EQ(type_counts[evtol::AircraftType::CHARLIE], 200);
    EXPECT_EQ(type_counts[evtol::AircraftType::DELTA], 200);
    EXPECT_EQ(type_counts[evtol::AircraftType::ECHO], 200);
}

// Test aircraft independence (each aircraft is separate object)
TEST_F(AircraftFactoryTest, AircraftIndependence) {
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

// Test aircraft functionality after creation
TEST_F(AircraftFactoryTest, AircraftFunctionalityAfterCreation) {
    auto fleet = evtol::AircraftFactory<>::create_fleet(5);
    
    for (auto& aircraft : fleet) {
        // Test basic functionality
        EXPECT_GT(aircraft->get_flight_time_hours(), 0.0);
        EXPECT_GT(aircraft->get_flight_distance_miles(), 0.0);
        EXPECT_GT(aircraft->get_charge_time_hours(), 0.0);
        EXPECT_GT(aircraft->get_passenger_count(), 0);
        EXPECT_NEAR_TOLERANCE(aircraft->get_battery_level(), 1.0);
        
        // Test battery operations
        aircraft->discharge_battery();
        EXPECT_NEAR_TOLERANCE(aircraft->get_battery_level(), 0.0);
        
        aircraft->charge_battery();
        EXPECT_NEAR_TOLERANCE(aircraft->get_battery_level(), 1.0);
        
        // Test manufacturer name
        EXPECT_FALSE(aircraft->get_manufacturer().empty());
    }
}

// Test polymorphic behavior of created fleet
TEST_F(AircraftFactoryTest, PolymorphicBehavior) {
    auto fleet = evtol::AircraftFactory<>::create_fleet(5);
    
    // All aircraft should be accessible through base class interface
    for (auto& aircraft : fleet) {
        evtol::AircraftBase* base_ptr = aircraft.get();
        EXPECT_NE(base_ptr, nullptr);
        
        // Virtual function calls should work correctly
        EXPECT_GE(base_ptr->get_id(), 0);
        EXPECT_GT(base_ptr->get_flight_time_hours(), 0.0);
        EXPECT_GT(base_ptr->get_flight_distance_miles(), 0.0);
        EXPECT_FALSE(base_ptr->get_manufacturer().empty());
    }
}

// Test fleet memory management
TEST_F(AircraftFactoryTest, FleetMemoryManagement) {
    {
        auto fleet = evtol::AircraftFactory<>::create_fleet(100);
        EXPECT_EQ(fleet.size(), 100);
        
        // Fleet contains unique_ptr objects, so they should be automatically managed
        for (const auto& aircraft : fleet) {
            EXPECT_NE(aircraft.get(), nullptr);
        }
    } // Fleet goes out of scope here - all aircraft should be automatically destroyed
    
    // No explicit memory cleanup needed - RAII should handle it
    SUCCEED(); // If we reach here without crashes, memory management is working
}

// Test fleet type consistency within aircraft groups
TEST_F(AircraftFactoryTest, TypeConsistencyWithinGroups) {
    auto fleet = evtol::AircraftFactory<>::create_fleet(25); // 5 groups of 5
    
    // Group aircraft by type and verify specifications are consistent
    std::map<evtol::AircraftType, std::vector<evtol::AircraftBase*>> type_groups;
    for (auto& aircraft : fleet) {
        type_groups[aircraft->get_type()].push_back(aircraft.get());
    }
    
    // Each type should have exactly 5 aircraft
    for (const auto& [type, aircraft_list] : type_groups) {
        EXPECT_EQ(aircraft_list.size(), 5);
        
        // All aircraft of same type should have same specifications
        const auto& first_spec = aircraft_list[0]->get_spec();
        for (size_t i = 1; i < aircraft_list.size(); ++i) {
            const auto& spec = aircraft_list[i]->get_spec();
            
            EXPECT_EQ(spec.manufacturer, first_spec.manufacturer);
            EXPECT_NEAR_TOLERANCE(spec.cruise_speed_mph, first_spec.cruise_speed_mph);
            EXPECT_NEAR_TOLERANCE(spec.battery_capacity_kwh, first_spec.battery_capacity_kwh);
            EXPECT_NEAR_TOLERANCE(spec.time_to_charge_hours, first_spec.time_to_charge_hours);
            EXPECT_EQ(spec.passenger_count, first_spec.passenger_count);
            EXPECT_NEAR_TOLERANCE(spec.fault_probability_per_hour, first_spec.fault_probability_per_hour);
        }
    }
}

// Test fleet creation with extremely large size
TEST_F(AircraftFactoryTest, ExtremelyLargeFleet) {
    const int extreme_size = 10000;
    
    // This test verifies the factory can handle large fleets efficiently
    auto start_time = std::chrono::high_resolution_clock::now();
    auto fleet = evtol::AircraftFactory<>::create_fleet(extreme_size);
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    EXPECT_EQ(fleet.size(), static_cast<size_t>(extreme_size));
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
    
    // Verify first and last aircraft are correctly created
    EXPECT_EQ(fleet[0]->get_id(), 0);
    EXPECT_EQ(fleet[0]->get_type(), evtol::AircraftType::ALPHA);
    EXPECT_EQ(fleet[extreme_size - 1]->get_id(), extreme_size - 1);
    // (10000-1) % 5 = 9999 % 5 = 4, which corresponds to ECHO (index 4)
    EXPECT_EQ(fleet[extreme_size - 1]->get_type(), evtol::AircraftType::ECHO);
}

// Test fleet move semantics
TEST_F(AircraftFactoryTest, FleetMoveSemantics) {
    auto original_fleet = evtol::AircraftFactory<>::create_fleet(10);
    EXPECT_EQ(original_fleet.size(), 10);
    
    // Move the fleet
    auto moved_fleet = std::move(original_fleet);
    
    EXPECT_EQ(moved_fleet.size(), 10);
    EXPECT_EQ(original_fleet.size(), 0); // Original should be empty after move
    
    // Moved fleet should be fully functional
    for (size_t i = 0; i < moved_fleet.size(); ++i) {
        EXPECT_NE(moved_fleet[i], nullptr);
        EXPECT_EQ(moved_fleet[i]->get_id(), static_cast<int>(i));
    }
}

// Test fleet copy behavior
TEST_F(AircraftFactoryTest, FleetCopyBehavior) {
    auto original_fleet = evtol::AircraftFactory<>::create_fleet(5);
    
    // Modify first aircraft in original fleet
    original_fleet[0]->discharge_battery();
    EXPECT_NEAR_TOLERANCE(original_fleet[0]->get_battery_level(), 0.0);
    
    // The fleet contains unique_ptr, so it's move-only (cannot be copied)
    // This test just verifies that the design correctly uses unique_ptr
    
    // We can create a copy by creating a new fleet and copying data
    auto new_fleet = evtol::AircraftFactory<>::create_fleet(5);
    EXPECT_EQ(new_fleet.size(), original_fleet.size());
    
    // New fleet should have all aircraft fully charged
    for (const auto& aircraft : new_fleet) {
        EXPECT_NEAR_TOLERANCE(aircraft->get_battery_level(), 1.0);
    }
}

// Test factory with template parameters (even though not used in current implementation)
TEST_F(AircraftFactoryTest, FactoryTemplateUsage) {
    // Test that the factory template can be instantiated with different parameters
    auto fleet1 = evtol::AircraftFactory<>::create_fleet(5);
    auto fleet2 = evtol::AircraftFactory<evtol::AlphaAircraft>::create_fleet(5);
    
    EXPECT_EQ(fleet1.size(), 5);
    EXPECT_EQ(fleet2.size(), 5);
    
    // Both should create the same distribution since the template parameters
    // don't affect the current implementation
    for (size_t i = 0; i < 5; ++i) {
        EXPECT_EQ(fleet1[i]->get_type(), fleet2[i]->get_type());
    }
}

// Test negative fleet size (edge case)
TEST_F(AircraftFactoryTest, NegativeFleetSize) {
    // Most implementations would handle negative size as 0 or throw an exception
    // Let's test what happens with negative input
    try {
        auto fleet = evtol::AircraftFactory<>::create_fleet(-5);
        // If no exception thrown, fleet should be empty
        EXPECT_EQ(fleet.size(), 0);
    } catch (...) {
        // If exception is thrown, that's also acceptable behavior
        SUCCEED();
    }
}

} // namespace evtol_test