#include "test_utilities.h"

namespace evtol_test {

// Test ChargerManager construction and initial state
TEST_F(ChargerManagerTest, Construction) {
    EXPECT_EQ(charger_manager_->get_total_chargers(), 3);
    EXPECT_EQ(charger_manager_->get_available_chargers(), 3);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 0);
    EXPECT_EQ(charger_manager_->get_queue_size(), 0);
}

// Test single charger request
TEST_F(ChargerManagerTest, SingleChargerRequest) {
    int aircraft_id = 100;
    
    bool request_result = charger_manager_->request_charger(aircraft_id);
    
    EXPECT_TRUE(request_result);
    EXPECT_EQ(charger_manager_->get_available_chargers(), 2);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 1);
    EXPECT_EQ(charger_manager_->get_queue_size(), 0);
}

// Test multiple charger requests
TEST_F(ChargerManagerTest, MultipleChargerRequests) {
    int aircraft_id1 = 100;
    int aircraft_id2 = 200;
    int aircraft_id3 = 300;
    
    // Request first charger
    bool request1 = charger_manager_->request_charger(aircraft_id1);
    EXPECT_TRUE(request1);
    EXPECT_EQ(charger_manager_->get_available_chargers(), 2);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 1);
    
    // Request second charger
    bool request2 = charger_manager_->request_charger(aircraft_id2);
    EXPECT_TRUE(request2);
    EXPECT_EQ(charger_manager_->get_available_chargers(), 1);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 2);
    
    // Request third charger
    bool request3 = charger_manager_->request_charger(aircraft_id3);
    EXPECT_TRUE(request3);
    EXPECT_EQ(charger_manager_->get_available_chargers(), 0);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 3);
}

// Test charger request when all chargers are in use
TEST_F(ChargerManagerTest, ChargerRequestWhenFull) {
    // Fill all chargers
    for (int i = 0; i < 3; ++i) {
        bool request = charger_manager_->request_charger(i);
        EXPECT_TRUE(request);
    }
    
    // Try to request one more charger
    bool request_fail = charger_manager_->request_charger(999);
    EXPECT_FALSE(request_fail);
    EXPECT_EQ(charger_manager_->get_available_chargers(), 0);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 3);
}

// Test charger release
TEST_F(ChargerManagerTest, ChargerRelease) {
    int aircraft_id = 100;
    
    // Request charger
    bool request_result = charger_manager_->request_charger(aircraft_id);
    EXPECT_TRUE(request_result);
    EXPECT_EQ(charger_manager_->get_available_chargers(), 2);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 1);
    
    // Release charger
    charger_manager_->release_charger(aircraft_id);
    EXPECT_EQ(charger_manager_->get_available_chargers(), 3);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 0);
}

// Test releasing charger for non-existent aircraft
TEST_F(ChargerManagerTest, ReleaseNonExistentCharger) {
    int non_existent_aircraft = 999;
    
    // Initial state
    EXPECT_EQ(charger_manager_->get_available_chargers(), 3);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 0);
    
    // Release charger for aircraft that doesn't have one
    charger_manager_->release_charger(non_existent_aircraft);
    
    // State should be unchanged
    EXPECT_EQ(charger_manager_->get_available_chargers(), 3);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 0);
}

// Test queue functionality - adding to queue
TEST_F(ChargerManagerTest, AddToQueue) {
    int aircraft_id = 100;
    
    charger_manager_->add_to_queue(aircraft_id);
    
    EXPECT_EQ(charger_manager_->get_queue_size(), 1);
}

// Test queue functionality - multiple adds
TEST_F(ChargerManagerTest, MultipleAddsToQueue) {
    for (int i = 0; i < 5; ++i) {
        charger_manager_->add_to_queue(i);
    }
    
    EXPECT_EQ(charger_manager_->get_queue_size(), 5);
}

// Test queue functionality - get next from queue
TEST_F(ChargerManagerTest, GetNextFromQueue) {
    int aircraft_id1 = 100;
    int aircraft_id2 = 200;
    int aircraft_id3 = 300;
    
    // Add aircraft to queue
    charger_manager_->add_to_queue(aircraft_id1);
    charger_manager_->add_to_queue(aircraft_id2);
    charger_manager_->add_to_queue(aircraft_id3);
    
    EXPECT_EQ(charger_manager_->get_queue_size(), 3);
    
    // Get next from queue (should be FIFO)
    int next1 = charger_manager_->get_next_from_queue();
    EXPECT_EQ(next1, aircraft_id1);
    EXPECT_EQ(charger_manager_->get_queue_size(), 2);
    
    int next2 = charger_manager_->get_next_from_queue();
    EXPECT_EQ(next2, aircraft_id2);
    EXPECT_EQ(charger_manager_->get_queue_size(), 1);
    
    int next3 = charger_manager_->get_next_from_queue();
    EXPECT_EQ(next3, aircraft_id3);
    EXPECT_EQ(charger_manager_->get_queue_size(), 0);
}

// Test get next from empty queue
TEST_F(ChargerManagerTest, GetNextFromEmptyQueue) {
    EXPECT_EQ(charger_manager_->get_queue_size(), 0);
    
    int next = charger_manager_->get_next_from_queue();
    EXPECT_EQ(next, -1);
    EXPECT_EQ(charger_manager_->get_queue_size(), 0);
}

// Test assign_charger functionality
TEST_F(ChargerManagerTest, AssignCharger) {
    int aircraft_id = 100;
    
    bool assign_result = charger_manager_->assign_charger(aircraft_id);
    
    EXPECT_TRUE(assign_result);
    EXPECT_EQ(charger_manager_->get_available_chargers(), 2);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 1);
}

// Test assign_charger when all chargers are busy
TEST_F(ChargerManagerTest, AssignChargerWhenFull) {
    // Fill all chargers
    for (int i = 0; i < 3; ++i) {
        bool assign = charger_manager_->assign_charger(i);
        EXPECT_TRUE(assign);
    }
    
    // Try to assign one more charger
    bool assign_fail = charger_manager_->assign_charger(999);
    EXPECT_FALSE(assign_fail);
    EXPECT_EQ(charger_manager_->get_available_chargers(), 0);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 3);
}

// Test complete workflow: request, release, and queue management
TEST_F(ChargerManagerTest, CompleteWorkflow) {
    // Step 1: Fill all chargers
    for (int i = 0; i < 3; ++i) {
        bool request = charger_manager_->request_charger(i);
        EXPECT_TRUE(request);
    }
    EXPECT_EQ(charger_manager_->get_active_chargers(), 3);
    EXPECT_EQ(charger_manager_->get_available_chargers(), 0);
    
    // Step 2: Add aircraft to queue (chargers are full)
    for (int i = 3; i < 6; ++i) {
        charger_manager_->add_to_queue(i);
    }
    EXPECT_EQ(charger_manager_->get_queue_size(), 3);
    
    // Step 3: Release one charger
    charger_manager_->release_charger(0);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 2);
    EXPECT_EQ(charger_manager_->get_available_chargers(), 1);
    
    // Step 4: Assign charger to next in queue
    int next_aircraft = charger_manager_->get_next_from_queue();
    EXPECT_EQ(next_aircraft, 3);
    EXPECT_EQ(charger_manager_->get_queue_size(), 2);
    
    bool assign_result = charger_manager_->assign_charger(next_aircraft);
    EXPECT_TRUE(assign_result);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 3);
    EXPECT_EQ(charger_manager_->get_available_chargers(), 0);
}

// Test edge case: same aircraft requesting multiple chargers
TEST_F(ChargerManagerTest, SameAircraftMultipleRequests) {
    int aircraft_id = 100;
    
    // First request should succeed
    bool request1 = charger_manager_->request_charger(aircraft_id);
    EXPECT_TRUE(request1);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 1);
    
    // Second request from same aircraft should still succeed
    // (This depends on implementation - current implementation allows it)
    bool request2 = charger_manager_->request_charger(aircraft_id);
    EXPECT_TRUE(request2);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 2);
}

// Test stress test: many aircraft in queue
TEST_F(ChargerManagerTest, StressTestQueue) {
    const int num_aircraft = 100;
    
    // Fill chargers
    for (int i = 0; i < 3; ++i) {
        charger_manager_->request_charger(i);
    }
    
    // Add many aircraft to queue
    for (int i = 3; i < num_aircraft; ++i) {
        charger_manager_->add_to_queue(i);
    }
    
    EXPECT_EQ(charger_manager_->get_queue_size(), num_aircraft - 3);
    
    // Process queue gradually
    for (int i = 0; i < 3; ++i) {
        charger_manager_->release_charger(i);
        
        if (charger_manager_->get_queue_size() > 0) {
            int next_aircraft = charger_manager_->get_next_from_queue();
            charger_manager_->assign_charger(next_aircraft);
        }
    }
    
    EXPECT_EQ(charger_manager_->get_active_chargers(), 3);
    EXPECT_EQ(charger_manager_->get_queue_size(), num_aircraft - 6);
}

// Test queue FIFO behavior
TEST_F(ChargerManagerTest, QueueFIFOBehavior) {
    std::vector<int> aircraft_ids = {100, 200, 300, 400, 500};
    
    // Add aircraft to queue in order
    for (int id : aircraft_ids) {
        charger_manager_->add_to_queue(id);
    }
    
    // Remove aircraft from queue - should be in same order
    for (int expected_id : aircraft_ids) {
        int actual_id = charger_manager_->get_next_from_queue();
        EXPECT_EQ(actual_id, expected_id);
    }
    
    EXPECT_EQ(charger_manager_->get_queue_size(), 0);
}

// Test concurrent operations (simulated)
TEST_F(ChargerManagerTest, SimulatedConcurrentOperations) {
    // Simulate interleaved operations
    charger_manager_->request_charger(1);
    charger_manager_->add_to_queue(4);
    charger_manager_->request_charger(2);
    charger_manager_->add_to_queue(5);
    charger_manager_->request_charger(3);
    charger_manager_->add_to_queue(6);
    
    EXPECT_EQ(charger_manager_->get_active_chargers(), 3);
    EXPECT_EQ(charger_manager_->get_available_chargers(), 0);
    EXPECT_EQ(charger_manager_->get_queue_size(), 3);
    
    // Release and assign in interleaved manner
    charger_manager_->release_charger(1);
    int next1 = charger_manager_->get_next_from_queue();
    EXPECT_EQ(next1, 4);
    charger_manager_->assign_charger(next1);
    
    charger_manager_->release_charger(2);
    int next2 = charger_manager_->get_next_from_queue();
    EXPECT_EQ(next2, 5);
    charger_manager_->assign_charger(next2);
    
    EXPECT_EQ(charger_manager_->get_active_chargers(), 3);
    EXPECT_EQ(charger_manager_->get_queue_size(), 1);
}

// Test boundary conditions
TEST_F(ChargerManagerTest, BoundaryConditions) {
    // Test with aircraft ID 0
    bool request_zero = charger_manager_->request_charger(0);
    EXPECT_TRUE(request_zero);
    
    charger_manager_->release_charger(0);
    
    // Test with negative aircraft ID
    bool request_negative = charger_manager_->request_charger(-1);
    EXPECT_TRUE(request_negative); // Implementation allows negative IDs
    
    charger_manager_->release_charger(-1);
    
    // Test with very large aircraft ID
    int large_id = 2147483647; // MAX_INT
    bool request_large = charger_manager_->request_charger(large_id);
    EXPECT_TRUE(request_large);
    
    charger_manager_->release_charger(large_id);
}

// Test charger state consistency
TEST_F(ChargerManagerTest, StateConsistency) {
    // At any time, active + available should equal total
    auto verify_consistency = [&]() {
        EXPECT_EQ(charger_manager_->get_active_chargers() + charger_manager_->get_available_chargers(),
                  charger_manager_->get_total_chargers());
    };
    
    verify_consistency();
    
    // Request all chargers
    for (int i = 0; i < 3; ++i) {
        charger_manager_->request_charger(i);
        verify_consistency();
    }
    
    // Release all chargers
    for (int i = 0; i < 3; ++i) {
        charger_manager_->release_charger(i);
        verify_consistency();
    }
    
    // Mixed operations
    charger_manager_->request_charger(100);
    verify_consistency();
    charger_manager_->request_charger(200);
    verify_consistency();
    charger_manager_->release_charger(100);
    verify_consistency();
    charger_manager_->request_charger(300);
    verify_consistency();
}

// Test queue with duplicate aircraft IDs
TEST_F(ChargerManagerTest, QueueWithDuplicates) {
    int aircraft_id = 100;
    
    // Add same aircraft to queue multiple times
    charger_manager_->add_to_queue(aircraft_id);
    charger_manager_->add_to_queue(aircraft_id);
    charger_manager_->add_to_queue(aircraft_id);
    
    EXPECT_EQ(charger_manager_->get_queue_size(), 3);
    
    // All should come out as the same ID
    for (int i = 0; i < 3; ++i) {
        int next = charger_manager_->get_next_from_queue();
        EXPECT_EQ(next, aircraft_id);
    }
    
    EXPECT_EQ(charger_manager_->get_queue_size(), 0);
}

// Test large scale operations
TEST_F(ChargerManagerTest, LargeScaleOperations) {
    const int num_operations = 1000;
    
    // Fill chargers first
    for (int i = 0; i < 3; ++i) {
        charger_manager_->request_charger(i);
    }
    
    // Add many aircraft to queue
    for (int i = 3; i < num_operations; ++i) {
        charger_manager_->add_to_queue(i);
    }
    
    EXPECT_EQ(charger_manager_->get_queue_size(), num_operations - 3);
    
    // Process all aircraft through the system
    int processed = 0;
    while (charger_manager_->get_queue_size() > 0) {
        // Release a charger (rotate through active chargers)
        int charger_to_release = processed % 3;
        charger_manager_->release_charger(charger_to_release);
        
        // Get next from queue and assign charger
        int next_aircraft = charger_manager_->get_next_from_queue();
        EXPECT_GE(next_aircraft, 3);
        EXPECT_LT(next_aircraft, num_operations);
        
        charger_manager_->assign_charger(next_aircraft);
        processed++;
    }
    
    EXPECT_EQ(charger_manager_->get_queue_size(), 0);
    EXPECT_EQ(charger_manager_->get_active_chargers(), 3);
}

} // namespace evtol_test