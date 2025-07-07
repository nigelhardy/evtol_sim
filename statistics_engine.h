#pragma once
#include "aircraft.h"
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <iomanip>
#include <sstream>
#include <type_traits>
#include <vector>

namespace evtol
{

    class StatisticsCollector
    {
    private:
        std::unordered_map<AircraftType, FlightStats> stats_;
        mutable std::mutex stats_mutex_;

        static constexpr const char *aircraft_type_names[] = {
            "Alpha", "Beta", "Charlie", "Delta", "Echo"};

    public:
        StatisticsCollector()
        {
            stats_.emplace(AircraftType::ALPHA, FlightStats{});
            stats_.emplace(AircraftType::BETA, FlightStats{});
            stats_.emplace(AircraftType::CHARLIE, FlightStats{});
            stats_.emplace(AircraftType::DELTA, FlightStats{});
            stats_.emplace(AircraftType::ECHO, FlightStats{});
        }

        template <typename... MetricArgs>
        void record_flight(AircraftType type, double flight_time, double distance, int passengers, MetricArgs &&...args)
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_[type].add_flight(flight_time, distance, passengers);

            if constexpr (sizeof...(args) > 0)
            {
                record_additional_metrics(type, std::forward<MetricArgs>(args)...);
            }
        }

        template <typename... MetricArgs>
        void record_charge_session(AircraftType type, double charge_time, MetricArgs &&...args)
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_[type].add_charge_session(charge_time);

            if constexpr (sizeof...(args) > 0)
            {
                record_additional_metrics(type, std::forward<MetricArgs>(args)...);
            }
        }

        void record_fault(AircraftType type)
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_[type].add_fault();
        }

        template <typename Metric, typename... Rest>
        void record_additional_metrics(AircraftType type, Metric &&metric, Rest &&...rest)
        {
            if constexpr (std::is_arithmetic_v<std::decay_t<Metric>>)
            {
                stats_[type].total_passenger_miles += static_cast<double>(metric);
            }

            if constexpr (sizeof...(rest) > 0)
            {
                record_additional_metrics(type, std::forward<Rest>(rest)...);
            }
        }

        const FlightStats &get_stats(AircraftType type) const
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            return stats_.at(type);
        }

        template <typename Predicate>
        auto get_filtered_stats(Predicate pred) const
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            std::vector<std::pair<AircraftType, FlightStats>> filtered;

            for (const auto &[type, stats] : stats_)
            {
                if (pred(type, stats))
                {
                    filtered.emplace_back(type, stats);
                }
            }

            return filtered;
        }

        template <typename Aggregator>
        auto aggregate_stats(Aggregator agg) const
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            return agg(stats_);
        }

        std::string generate_report() const
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            std::ostringstream oss;

            oss << std::fixed << std::setprecision(2);
            oss << "\n========== eVTOL Simulation Results ==========\n\n";

            for (const auto &[type, stats] : stats_)
            {
                oss << aircraft_type_names[static_cast<int>(type)] << " Aircraft:\n";
                oss << "  Average Flight Time: " << stats.avg_flight_time() << " hours\n";
                oss << "  Average Distance: " << stats.avg_distance() << " miles\n";
                oss << "  Average Charging Time: " << stats.avg_charging_time() << " hours\n";
                oss << "  Total Faults: " << stats.total_faults.load() << "\n";
                oss << "  Total Passenger Miles: " << stats.total_passenger_miles << "\n";
                oss << "  Total Flights: " << stats.flight_count << "\n";
                oss << "  Total Charge Sessions: " << stats.charge_count << "\n\n";
            }

            return oss.str();
        }

        auto get_summary_stats() const
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);

            struct SummaryStats
            {
                double total_flight_time = 0.0;
                double total_distance = 0.0;
                double total_charging_time = 0.0;
                int total_faults = 0;
                double total_passenger_miles = 0.0;
                int total_flights = 0;
                int total_charges = 0;
            };

            SummaryStats summary;

            for (const auto &[type, stats] : stats_)
            {
                summary.total_flight_time += stats.total_flight_time_hours;
                summary.total_distance += stats.total_distance_miles;
                summary.total_charging_time += stats.total_charging_time_hours;
                summary.total_faults += stats.total_faults.load();
                summary.total_passenger_miles += stats.total_passenger_miles;
                summary.total_flights += stats.flight_count;
                summary.total_charges += stats.charge_count;
            }

            return summary;
        }
    };

}