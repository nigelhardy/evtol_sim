#include "test_utilities.h"

namespace evtol_test
{

    class PerformanceTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            TestDataGenerator::seed_random(42); // Consistent random results
        }
    };

    // Test large fleet creation performance
    TEST_F(PerformanceTest, LargeFleetCreationPerformance)
    {
        const int large_fleet_size = 10000;

        PerformanceTestHelper::run_performance_test(
            "Large Fleet Creation",
            [&]()
            {
                auto fleet = evtol::AircraftFactory<>::create_fleet(large_fleet_size);
                EXPECT_EQ(fleet.size(), static_cast<size_t>(large_fleet_size));
            },
            5.0 // Should complete within 5 seconds
        );
    }

    // Test simulation performance with large fleet
    TEST_F(PerformanceTest, LargeFleetSimulationPerformance)
    {
        const int large_fleet_size = 1000;
        auto fleet = evtol::AircraftFactory<>::create_fleet(large_fleet_size);

        evtol::StatisticsCollector stats_collector;
        evtol::ChargerManager charger_manager;
        evtol::EventDrivenSimulation sim_engine(stats_collector, 3.0);

        PerformanceTestHelper::run_performance_test(
            "Large Fleet Simulation",
            [&]()
            {
                sim_engine.run_simulation(charger_manager, fleet);
                auto summary = stats_collector.get_summary_stats();
                EXPECT_GT(summary.total_flights, large_fleet_size);
            },
            15.0 // Should complete within 15 seconds
        );
    }

    // Test long duration simulation performance
    TEST_F(PerformanceTest, LongDurationSimulationPerformance)
    {
        const int fleet_size = 100;
        const double long_duration = 24.0; // 24 hour simulation

        auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);
        evtol::StatisticsCollector stats_collector;
        evtol::ChargerManager charger_manager;
        evtol::EventDrivenSimulation sim_engine(stats_collector, long_duration);

        PerformanceTestHelper::run_performance_test(
            "Long Duration Simulation",
            [&]()
            {
                sim_engine.run_simulation(charger_manager, fleet);
                auto summary = stats_collector.get_summary_stats();
                // Because of faults, this isn't deterministic, but for this project, I will leave it dangerous
                EXPECT_GT(summary.total_flights, fleet_size * 2); // Multiple flights per aircraft
            },
            10.0 // Should complete within 10 seconds despite long simulated time
        );
    }

    // Test memory usage with repeated simulations
    TEST_F(PerformanceTest, RepeatedSimulationsMemoryUsage)
    {
        const int num_iterations = 100;
        const int fleet_size = 50;

        PerformanceTestHelper::run_performance_test(
            "Repeated Simulations Memory Usage",
            [&]()
            {
                for (int i = 0; i < num_iterations; ++i)
                {
                    auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);
                    evtol::StatisticsCollector stats_collector;
                    evtol::ChargerManager charger_manager;
                    evtol::EventDrivenSimulation sim_engine(stats_collector, 1.0);

                    sim_engine.run_simulation(charger_manager, fleet);

                    auto summary = stats_collector.get_summary_stats();
                    EXPECT_GT(summary.total_flights, 0);

                    // Fleet and other objects should be cleaned up automatically
                }
            },
            20.0 // Should complete within 20 seconds
        );
    }

    // Test statistics collection performance
    TEST_F(PerformanceTest, StatisticsCollectionPerformance)
    {
        evtol::StatisticsCollector stats_collector;
        const int num_operations = 100000;

        PerformanceTestHelper::run_performance_test(
            "Statistics Collection Performance",
            [&]()
            {
                for (int i = 0; i < num_operations; ++i)
                {
                    evtol::AircraftType type = static_cast<evtol::AircraftType>(i % 5);
                    stats_collector.record_flight(type, 1.0 + i * 0.001, 50.0 + i * 0.1, 2 + (i % 3));
                    stats_collector.record_charge_session(type, 0.5 + i * 0.0001);

                    if (i % 100 == 0)
                    {
                        stats_collector.record_fault(type);
                    }
                }

                auto summary = stats_collector.get_summary_stats();
                EXPECT_EQ(summary.total_flights, num_operations);
                EXPECT_EQ(summary.total_charges, num_operations);
                EXPECT_EQ(summary.total_faults, 1000);
            },
            3.0 // Should complete within 3 seconds
        );
    }

    // Test charger manager performance under load
    TEST_F(PerformanceTest, ChargerManagerPerformanceUnderLoad)
    {
        evtol::ChargerManager charger_manager;
        const int num_operations = 10000;

        PerformanceTestHelper::run_performance_test(
            "Charger Manager Performance Under Load",
            [&]()
            {
                std::vector<int> active_aircraft;

                for (int i = 0; i < num_operations; ++i)
                {
                    if (charger_manager.request_charger(i))
                    {
                        active_aircraft.push_back(i);

                        // If we have too many active, release some
                        if (active_aircraft.size() > 2)
                        {
                            int to_release = active_aircraft.front();
                            active_aircraft.erase(active_aircraft.begin());
                            charger_manager.release_charger(to_release);
                        }
                    }
                    else
                    {
                        charger_manager.add_to_queue(i);

                        // Try to process queue
                        if (!active_aircraft.empty())
                        {
                            int to_release = active_aircraft.front();
                            active_aircraft.erase(active_aircraft.begin());
                            charger_manager.release_charger(to_release);

                            int next_aircraft = charger_manager.get_next_from_queue();
                            if (next_aircraft != -1)
                            {
                                charger_manager.assign_charger(next_aircraft);
                                active_aircraft.push_back(next_aircraft);
                            }
                        }
                    }
                }

                EXPECT_EQ(charger_manager.get_total_chargers(), 3);
            },
            5.0 // Should complete within 5 seconds
        );
    }

    // Test aircraft operations performance
    TEST_F(PerformanceTest, AircraftOperationsPerformance)
    {
        const int num_aircraft = 10000;
        const int operations_per_aircraft = 100;

        PerformanceTestHelper::run_performance_test(
            "Aircraft Operations Performance",
            [&]()
            {
                auto fleet = evtol::AircraftFactory<>::create_fleet(num_aircraft);

                for (auto &aircraft : fleet)
                {
                    for (int op = 0; op < operations_per_aircraft; ++op)
                    {
                        // Test various aircraft operations
                        double flight_time = aircraft->get_flight_time_hours();
                        double distance = aircraft->get_flight_distance_miles();
                        (void)aircraft->check_fault_during_flight(0.1); // Suppress unused variable warning
                        double battery = aircraft->get_battery_level();

                        aircraft->discharge_battery();
                        aircraft->charge_battery();

                        // Basic validation
                        EXPECT_GT(flight_time, 0.0);
                        EXPECT_GT(distance, 0.0);
                        EXPECT_GE(battery, 0.0);
                        EXPECT_LE(battery, 1.0);
                    }
                }

                EXPECT_EQ(fleet.size(), static_cast<size_t>(num_aircraft));
            },
            10.0 // Should complete within 10 seconds
        );
    }

    // Test report generation performance
    TEST_F(PerformanceTest, ReportGenerationPerformance)
    {
        evtol::StatisticsCollector stats_collector;
        const int num_data_points = 1000;

        // Add substantial amount of data
        for (int i = 0; i < num_data_points; ++i)
        {
            for (int type_int = 0; type_int < 5; ++type_int)
            {
                evtol::AircraftType type = static_cast<evtol::AircraftType>(type_int);
                stats_collector.record_flight(type, 1.0 + i * 0.01, 50.0 + i * 0.5, 2 + (i % 3));
                stats_collector.record_charge_session(type, 0.5 + i * 0.005);

                if (i % 50 == 0)
                {
                    stats_collector.record_fault(type);
                }
            }
        }

        PerformanceTestHelper::run_performance_test(
            "Report Generation Performance",
            [&]()
            {
                std::string report = stats_collector.generate_report();
                EXPECT_FALSE(report.empty());
                EXPECT_GT(report.length(), 1000); // Substantial report

                // Generate multiple times to test consistency
                for (int i = 0; i < 10; ++i)
                {
                    std::string repeat_report = stats_collector.generate_report();
                    EXPECT_EQ(report, repeat_report);
                }
            },
            2.0 // Should complete within 2 seconds
        );
    }

    // Test concurrent-style operations performance
    TEST_F(PerformanceTest, ConcurrentStyleOperationsPerformance)
    {
        const int num_threads_simulated = 10;
        const int operations_per_thread = 100;

        PerformanceTestHelper::run_performance_test(
            "Concurrent-Style Operations Performance",
            [&]()
            {
                std::vector<std::unique_ptr<evtol::StatisticsCollector>> collectors;
                std::vector<std::unique_ptr<evtol::ChargerManager>> managers;

                // Simulate multiple independent simulations
                for (int thread = 0; thread < num_threads_simulated; ++thread)
                {
                    collectors.emplace_back(std::make_unique<evtol::StatisticsCollector>());
                    managers.emplace_back(std::make_unique<evtol::ChargerManager>());

                    auto &stats = *collectors.back();
                    auto &charger_mgr = *managers.back();

                    for (int op = 0; op < operations_per_thread; ++op)
                    {
                        evtol::AircraftType type = static_cast<evtol::AircraftType>(op % 5);
                        stats.record_flight(type, 1.0, 50.0, 2);

                        int aircraft_id = thread * operations_per_thread + op;
                        if (!charger_mgr.request_charger(aircraft_id))
                        {
                            charger_mgr.add_to_queue(aircraft_id);
                        }
                    }
                }

                // Verify all operations completed
                EXPECT_EQ(collectors.size(), static_cast<size_t>(num_threads_simulated));
                EXPECT_EQ(managers.size(), static_cast<size_t>(num_threads_simulated));
            },
            5.0 // Should complete within 5 seconds
        );
    }

    // Test simulation scaling with different fleet sizes
    TEST_F(PerformanceTest, SimulationScalingWithFleetSizes)
    {
        std::vector<int> fleet_sizes = {10, 50, 100, 500, 1000};
        std::vector<double> execution_times;

        for (int fleet_size : fleet_sizes)
        {
            auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);
            evtol::StatisticsCollector stats_collector;
            evtol::ChargerManager charger_manager;
            evtol::EventDrivenSimulation sim_engine(stats_collector, 1.0);

            auto start_time = std::chrono::high_resolution_clock::now();
            sim_engine.run_simulation(charger_manager, fleet);
            auto end_time = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            execution_times.push_back(duration.count());

            auto summary = stats_collector.get_summary_stats();
            EXPECT_GT(summary.total_flights, 0);

            std::cout << "Fleet size " << fleet_size << ": " << duration.count() << "ms\n";
        }

        // Verify reasonable scaling (execution time shouldn't grow exponentially)
        for (size_t i = 0; i < execution_times.size(); ++i)
        {
            EXPECT_LT(execution_times[i], 10000); // All should complete within 10 seconds
        }

        // Later tests shouldn't be dramatically slower than earlier ones
        if (execution_times.size() >= 2 && execution_times[0] > 0)
        {
            double ratio = execution_times.back() / execution_times[0];
            EXPECT_LT(ratio, 100.0); // No more than 100x slower for 100x more aircraft
        }
    }

    // Test memory efficiency with large datasets
    TEST_F(PerformanceTest, MemoryEfficiencyWithLargeDatasets)
    {
        const int large_dataset_size = 100000;

        PerformanceTestHelper::run_performance_test(
            "Memory Efficiency with Large Datasets",
            [&]()
            {
                evtol::StatisticsCollector stats_collector;

                // Add large amount of data
                for (int i = 0; i < large_dataset_size; ++i)
                {
                    evtol::AircraftType type = static_cast<evtol::AircraftType>(i % 5);
                    stats_collector.record_flight(type,
                                                  1.0 + (i % 100) * 0.01,
                                                  50.0 + (i % 200) * 0.5,
                                                  1 + (i % 5));

                    if (i % 10 == 0)
                    {
                        stats_collector.record_charge_session(type, 0.5 + (i % 50) * 0.01);
                    }

                    if (i % 1000 == 0)
                    {
                        stats_collector.record_fault(type);
                    }
                }

                // Verify data integrity
                auto summary = stats_collector.get_summary_stats();
                EXPECT_EQ(summary.total_flights, large_dataset_size);
                EXPECT_EQ(summary.total_charges, large_dataset_size / 10);
                EXPECT_EQ(summary.total_faults, large_dataset_size / 1000);

                // Test operations on large dataset
                std::string report = stats_collector.generate_report();
                EXPECT_FALSE(report.empty());

                auto filtered = stats_collector.get_filtered_stats(
                    [](evtol::AircraftType /*type*/, const evtol::FlightStats &stats)
                    {
                        return stats.flight_count > 10000;
                    });
                EXPECT_EQ(filtered.size(), 5); // All types should have > 10000 flights
            },
            10.0 // Should complete within 10 seconds
        );
    }

    // Test performance timer accuracy
    TEST_F(PerformanceTest, PerformanceTimerAccuracy)
    {
        const int num_measurements = 2000;
        std::vector<long long> measurements;

        evtol_test::PerformanceTimer<std::chrono::microseconds> timer;

        for (int i = 0; i < num_measurements; ++i)
        {
            timer.reset();

            // Do some consistent work
            volatile int sum = 0;
            for (int j = 0; j < 5000; ++j)
            {
                sum += j;
            }

            auto elapsed = timer.elapsed();
            measurements.push_back(elapsed.count());
        }

        // Calculate statistics
        double total = 0.0;
        for (auto measurement : measurements)
        {
            total += measurement;
        }
        double mean = total / measurements.size();

        double variance = 0.0;
        for (auto measurement : measurements)
        {
            variance += (measurement - mean) * (measurement - mean);
        }
        variance /= measurements.size();
        double std_dev = std::sqrt(variance);

        // Measurements should be reasonably consistent
        EXPECT_GT(mean, 0.0);
        EXPECT_LT(std_dev / mean, 10.0); // Coefficient of variation < 1000% (very small measurements can vary)

        std::cout << "Timer measurements - Mean: " << mean << "μs, StdDev: " << std_dev << "μs\n";
    }

    // Test end-to-end performance with realistic scenario
    TEST_F(PerformanceTest, EndToEndPerformanceRealisticScenario)
    {
        // Simulate realistic eVTOL operation scenario
        const int fleet_size = 100;      // Reasonable fleet size
        const double sim_duration = 8.0; // 8-hour operation day

        PerformanceTestHelper::run_performance_test(
            "End-to-End Realistic Scenario",
            [&]()
            {
                // Create realistic fleet
                auto fleet = evtol::AircraftFactory<>::create_fleet(fleet_size);

                // Set up simulation
                evtol::StatisticsCollector stats_collector;
                evtol::ChargerManager charger_manager;
                evtol::EventDrivenSimulation sim_engine(stats_collector, sim_duration);

                // Run simulation
                sim_engine.run_simulation(charger_manager, fleet);

                // Analyze results
                auto summary = stats_collector.get_summary_stats();
                EXPECT_GT(summary.total_flights, fleet_size);     // At least one flight per aircraft
                EXPECT_GT(summary.total_charges, 0);              // Some charges should occur
                EXPECT_GT(summary.total_passenger_miles, 1000.0); // Some passenger miles

                // Generate comprehensive report
                std::string report = stats_collector.generate_report();
                EXPECT_GT(report.length(), 500); // Substantial report

                // Verify final system state
                EXPECT_EQ(charger_manager.get_total_chargers(), 3);
                EXPECT_LE(charger_manager.get_active_chargers(), 3);

                std::cout << "Realistic scenario results:\n";
                std::cout << "Total flights: " << summary.total_flights << "\n";
                std::cout << "Total charges: " << summary.total_charges << "\n";
                std::cout << "Total passenger miles: " << summary.total_passenger_miles << "\n";
            },
            30.0 // Should complete within 30 seconds for realistic scenario
        );
    }

} // namespace evtol_test