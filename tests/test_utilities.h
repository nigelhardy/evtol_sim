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

    // Mock Aircraft class for controlled testing
    class MockAircraft : public evtol::AircraftBase
    {
    private:
        int id_;
        double battery_level_;
        evtol::AircraftType type_;
        std::string manufacturer_;
        evtol::AircraftSpec spec_;
        bool should_fault_;
        bool is_faulty_;

    public:
        MockAircraft(int id, evtol::AircraftType type = evtol::AircraftType::ALPHA,
                     bool should_fault = false)
            : id_(id), battery_level_(1.0), type_(type), manufacturer_("TestMfg"),
              spec_("TestMfg", 100.0, 100.0, 0.5, 2, 0.1), should_fault_(should_fault) {}

        virtual ~MockAircraft() = default;

        double get_flight_time_hours() const override { return 0.5; }
        double get_flight_distance_miles() const override { return 50.0; }
        double check_fault_during_flight(double /*flight_time_hours*/) override
        {
            // -1 means no fault, anything over 0.0 will be a fault
            // using .1 for fault arbitrarily to be short and less than flight time
            return should_fault_ ? 0.1 : -1;
        }
        void discharge_battery() override { battery_level_ = 0.0; }
        void charge_battery() override { battery_level_ = 1.0; }
        double get_battery_level() const override { return battery_level_; }
        int get_id() const override { return id_; }
        evtol::AircraftType get_type() const override { return type_; }
        std::string get_manufacturer() const override { return manufacturer_; }
        const evtol::AircraftSpec &get_spec() const override { return spec_; }
        int get_passenger_count() const override { return 2; }
        double get_charge_time_hours() const override { return 0.5; }
        bool is_faulty() const override { return is_faulty_; }
        void set_faulty(bool faulty) override { is_faulty_ = faulty; }

        // Test utilities
        void set_battery_level(double level) { battery_level_ = level; }
        void set_should_fault(bool fault) { should_fault_ = fault; }
    };

    // Mock Statistics Collector for testing simulation engine
    class MockStatisticsCollector : public evtol::StatisticsCollector
    {
    private:
        int flight_count_ = 0;
        int charge_count_ = 0;
        int fault_count_ = 0;

    public:
        // Override the specific signatures used by SimulationEngine
        void record_flight(evtol::AircraftType type, double flight_time,
                           double distance, int passengers) override
        {
            evtol::StatisticsCollector::record_flight(type, flight_time, distance, passengers);
            flight_count_++;
        }

        void record_charge_session(evtol::AircraftType type, double charge_time) override
        {
            evtol::StatisticsCollector::record_charge_session(type, charge_time);
            charge_count_++;
        }

        void record_fault(evtol::AircraftType type) override
        {
            evtol::StatisticsCollector::record_fault(type);
            fault_count_++;
        }

        // For compatibility with template calls, still provide template versions
        template <typename... MetricArgs>
        void record_flight(evtol::AircraftType type, double flight_time,
                           double distance, int passengers, MetricArgs &&...args)
        {
            evtol::StatisticsCollector::record_flight(type, flight_time, distance, passengers, std::forward<MetricArgs>(args)...);
            flight_count_++;
        }

        template <typename... MetricArgs>
        void record_charge_session(evtol::AircraftType type, double charge_time, MetricArgs &&...args)
        {
            evtol::StatisticsCollector::record_charge_session(type, charge_time, std::forward<MetricArgs>(args)...);
            charge_count_++;
        }

        // Test accessors
        int get_flight_count() const { return flight_count_; }
        int get_charge_count() const { return charge_count_; }
        int get_fault_count() const { return fault_count_; }

        void reset_counts()
        {
            flight_count_ = 0;
            charge_count_ = 0;
            fault_count_ = 0;
        }
    };

    // Test data generators
    class TestDataGenerator
    {
    private:
        static inline std::mt19937 rng_{std::random_device{}()};

    public:
        static void seed_random(unsigned int seed)
        {
            rng_.seed(seed);
        }

        static double random_double(double min = 0.0, double max = 1.0)
        {
            std::uniform_real_distribution<double> dist(min, max);
            return dist(rng_);
        }

        static int random_int(int min = 0, int max = 100)
        {
            std::uniform_int_distribution<int> dist(min, max);
            return dist(rng_);
        }

        static std::vector<std::unique_ptr<evtol::AircraftBase>> create_test_fleet(int size)
        {
            std::vector<std::unique_ptr<evtol::AircraftBase>> fleet;
            fleet.reserve(static_cast<size_t>(size));

            for (int i = 0; i < size; ++i)
            {
                evtol::AircraftType type = static_cast<evtol::AircraftType>(i % 5);
                fleet.emplace_back(std::make_unique<MockAircraft>(i, type));
            }

            return fleet;
        }

        static std::vector<std::unique_ptr<evtol::AircraftBase>> create_real_test_fleet(int size)
        {
            return evtol::AircraftFactory<>::create_fleet(size);
        }
    };

    // Time utilities for testing
    class TestTimeUtils
    {
    public:
        static void sleep_milliseconds(int ms)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }

        static auto get_current_time()
        {
            return std::chrono::high_resolution_clock::now();
        }

        static double get_elapsed_seconds(const std::chrono::high_resolution_clock::time_point &start)
        {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            return duration.count() / 1000000.0;
        }
    };

    // Test fixtures for common setup
    class FlightStatsTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            stats_ = evtol::FlightStats{};
        }

        evtol::FlightStats stats_;
    };

    class ChargerManagerTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            charger_manager_ = std::make_unique<evtol::ChargerManager>();
        }

        std::unique_ptr<evtol::ChargerManager> charger_manager_;
    };

    class StatisticsCollectorTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            stats_collector_ = std::make_unique<evtol::StatisticsCollector>();
        }

        std::unique_ptr<evtol::StatisticsCollector> stats_collector_;
    };
    // TODO Do we want these setup functions in test utilities? Or in the test themselves??
    class SimulationEngineTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            mock_stats_ = std::make_unique<MockStatisticsCollector>();
            sim_engine_ = std::make_unique<evtol::SimulationEngine>(*mock_stats_, TEST_SIMULATION_DURATION);
            charger_manager_ = std::make_unique<evtol::ChargerManager>();
        }

        std::unique_ptr<MockStatisticsCollector> mock_stats_;
        std::unique_ptr<evtol::SimulationEngine> sim_engine_;
        std::unique_ptr<evtol::ChargerManager> charger_manager_;
    };

    // Aircraft type test utilities
    template <typename AircraftType>
    class AircraftTypeTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            aircraft_ = std::make_unique<AircraftType>(DEFAULT_AIRCRAFT_ID);
        }

        std::unique_ptr<AircraftType> aircraft_;
    };

    // Performance test utilities
    class PerformanceTestHelper
    {
    public:
        static void run_performance_test(const std::string &test_name,
                                         std::function<void()> test_func,
                                         double expected_max_seconds = 1.0)
        {
            auto start = std::chrono::high_resolution_clock::now();
            test_func();
            auto end = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            double seconds = duration.count() / 1000000.0;

            std::cout << test_name << " completed in " << seconds << " seconds\n";
            EXPECT_LT(seconds, expected_max_seconds) << test_name << " took too long: " << seconds << "s";
        }
    };

    // Performance Timer class (from evtol_sim.cpp)
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

} // namespace evtol_test