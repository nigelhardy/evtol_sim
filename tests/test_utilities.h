#pragma once

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <random>
#include <chrono>
#include <thread>

#include "../aircraft.h"
#include "../aircraft_types.h"
#include "../simulation_engine.h"
#include "../charger_manager.h"
#include "../statistics_engine.h"

namespace evtol_test
{

    // Test constants for consistent testing
    constexpr double FLOAT_TOLERANCE = 1e-6;
    constexpr int DEFAULT_AIRCRAFT_ID = 42;
    constexpr int TEST_FLEET_SIZE = 5;
    constexpr double TEST_SIMULATION_DURATION = 2.0;

// Custom matcher for floating point comparison
#define EXPECT_NEAR_TOLERANCE(val1, val2) EXPECT_NEAR(val1, val2, FLOAT_TOLERANCE)

} // namespace evtol_test