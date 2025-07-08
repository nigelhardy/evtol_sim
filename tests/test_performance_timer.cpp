#include "test_utilities.h"
#include <chrono>
#include <thread>

// Since PerformanceTimer is defined in evtol_sim.cpp, we need to include it here
// For testing purposes, we'll recreate the template class
template <typename Duration>
class PerformanceTimer
{
private:
    std::chrono::high_resolution_clock::time_point start_time_;

public:
    PerformanceTimer() : start_time_(std::chrono::high_resolution_clock::now()) {}

    auto elapsed() const
    {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<Duration>(end_time - start_time_);
    }

    void reset()
    {
        start_time_ = std::chrono::high_resolution_clock::now();
    }
};

namespace evtol_test {

class PerformanceTimerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup any common test data
    }
};

// Test PerformanceTimer with microseconds
TEST_F(PerformanceTimerTest, MicrosecondsTimerBasicFunctionality) {
    PerformanceTimer<std::chrono::microseconds> timer;
    
    // Sleep for a known duration
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    auto elapsed = timer.elapsed();
    
    // Should be approximately 10000 microseconds (10ms), but allow for variance
    EXPECT_GE(elapsed.count(), 8000);  // At least 8ms
    EXPECT_LE(elapsed.count(), 20000); // At most 20ms (generous for CI systems)
}

// Test PerformanceTimer with milliseconds
TEST_F(PerformanceTimerTest, MillisecondsTimerBasicFunctionality) {
    PerformanceTimer<std::chrono::milliseconds> timer;
    
    // Sleep for a known duration
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    auto elapsed = timer.elapsed();
    
    // Should be approximately 50 milliseconds
    EXPECT_GE(elapsed.count(), 40);  // At least 40ms
    EXPECT_LE(elapsed.count(), 100); // At most 100ms (generous for CI systems)
}

// Test PerformanceTimer with nanoseconds
TEST_F(PerformanceTimerTest, NanosecondsTimerBasicFunctionality) {
    PerformanceTimer<std::chrono::nanoseconds> timer;
    
    // Sleep for a very small duration
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    
    auto elapsed = timer.elapsed();
    
    // Should be approximately 100000 nanoseconds (100μs)
    EXPECT_GE(elapsed.count(), 80000);   // At least 80μs
    EXPECT_LE(elapsed.count(), 200000);  // At most 200μs
}

// Test timer reset functionality
TEST_F(PerformanceTimerTest, TimerResetFunctionality) {
    PerformanceTimer<std::chrono::microseconds> timer;
    
    // Sleep for first duration
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    auto first_elapsed = timer.elapsed();
    
    // Reset timer and sleep for second duration
    timer.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    
    auto second_elapsed = timer.elapsed();
    
    // Second elapsed should be less than first elapsed
    EXPECT_LT(second_elapsed.count(), first_elapsed.count());
    
    // Second elapsed should be approximately 5ms
    EXPECT_GE(second_elapsed.count(), 3000);  // At least 3ms
    EXPECT_LE(second_elapsed.count(), 10000); // At most 10ms
}

// Test timer precision by measuring multiple intervals
TEST_F(PerformanceTimerTest, TimerPrecisionTest) {
    PerformanceTimer<std::chrono::microseconds> timer;
    
    std::vector<long long> measurements;
    
    for (int i = 0; i < 5; ++i) {
        timer.reset();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        measurements.push_back(timer.elapsed().count());
    }
    
    // All measurements should be reasonably close to each other
    // Calculate variance to ensure consistency
    double sum = 0.0;
    for (auto measurement : measurements) {
        sum += measurement;
    }
    double mean = sum / measurements.size();
    
    double variance = 0.0;
    for (auto measurement : measurements) {
        variance += (measurement - mean) * (measurement - mean);
    }
    variance /= measurements.size();
    
    // Standard deviation should be reasonable (less than 50% of mean)
    double std_dev = std::sqrt(variance);
    EXPECT_LT(std_dev, mean * 0.5);
}

// Test timer with very short intervals
TEST_F(PerformanceTimerTest, VeryShortIntervalTest) {
    PerformanceTimer<std::chrono::nanoseconds> timer;
    
    // Do some minimal work
    volatile int sum = 0;
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
    
    auto elapsed = timer.elapsed();
    
    // Should have measurable time, but not too much
    EXPECT_GT(elapsed.count(), 0);
    EXPECT_LT(elapsed.count(), 10000000); // Less than 10ms
}

// Test timer with seconds duration
TEST_F(PerformanceTimerTest, SecondsTimerTest) {
    PerformanceTimer<std::chrono::seconds> timer;
    
    // Sleep for just over 1 second
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    
    auto elapsed = timer.elapsed();
    
    // Should be at least 1 second
    EXPECT_GE(elapsed.count(), 1);
    EXPECT_LE(elapsed.count(), 2); // At most 2 seconds
}

// Test multiple timers can run simultaneously
TEST_F(PerformanceTimerTest, MultipleTimersTest) {
    PerformanceTimer<std::chrono::microseconds> timer1;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    
    PerformanceTimer<std::chrono::microseconds> timer2;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    
    auto elapsed1 = timer1.elapsed();
    auto elapsed2 = timer2.elapsed();
    
    // Timer1 should have approximately twice the elapsed time of timer2
    EXPECT_GT(elapsed1.count(), elapsed2.count());
    
    // Timer1 should be roughly 10ms, timer2 should be roughly 5ms
    EXPECT_GE(elapsed1.count(), 8000);  // At least 8ms
    EXPECT_LE(elapsed1.count(), 20000); // At most 20ms
    
    EXPECT_GE(elapsed2.count(), 3000);  // At least 3ms
    EXPECT_LE(elapsed2.count(), 10000); // At most 10ms
}

// Test timer construction and immediate measurement
TEST_F(PerformanceTimerTest, ImmediateMeasurementTest) {
    PerformanceTimer<std::chrono::nanoseconds> timer;
    
    // Measure immediately after construction
    auto elapsed = timer.elapsed();
    
    // Should be very small but measurable
    EXPECT_GE(elapsed.count(), 0);
    EXPECT_LT(elapsed.count(), 1000000); // Less than 1ms
}

// Test timer behavior with repeated measurements
TEST_F(PerformanceTimerTest, RepeatedMeasurementsTest) {
    PerformanceTimer<std::chrono::microseconds> timer;
    
    auto elapsed1 = timer.elapsed();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    auto elapsed2 = timer.elapsed();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    auto elapsed3 = timer.elapsed();
    
    // Each measurement should be greater than the previous
    EXPECT_LT(elapsed1.count(), elapsed2.count());
    EXPECT_LT(elapsed2.count(), elapsed3.count());
    
    // Final measurement should be approximately 10ms
    EXPECT_GE(elapsed3.count(), 8000);
    EXPECT_LE(elapsed3.count(), 20000);
}

} // namespace evtol_test