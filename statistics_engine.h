#pragma once
#include "aircraft.h"
#include <unordered_map>
#include <iomanip>
#include <sstream>
#include <type_traits>
#include <vector>

namespace evtol
{
    /**
     * Summary statistics structure
     */
    struct SummaryStats
    {
        double total_flight_time = 0.0;
        double total_distance = 0.0;
        double total_charging_time = 0.0;
        double total_waiting_time = 0.0;
        int total_faults = 0;
        double total_passenger_miles = 0.0;
        int total_flights = 0;
        int total_charges = 0;
        
        // Partial activities
        double partial_flight_time = 0.0;
        double partial_distance = 0.0;
        double partial_charging_time = 0.0;
        double partial_passenger_miles = 0.0;
        int partial_flights = 0;
        int partial_charges = 0;
    };

    class StatisticsCollector
    {
    private:
        std::unordered_map<AircraftType, FlightStats> stats_;
        std::unordered_map<AircraftType, int> aircraft_counts_;

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
            
            // Initialize aircraft counts to 0
            aircraft_counts_[AircraftType::ALPHA] = 0;
            aircraft_counts_[AircraftType::BETA] = 0;
            aircraft_counts_[AircraftType::CHARLIE] = 0;
            aircraft_counts_[AircraftType::DELTA] = 0;
            aircraft_counts_[AircraftType::ECHO] = 0;
        }

        virtual ~StatisticsCollector() = default;

        /**
         * Set the count of aircraft for each type
         * @param fleet The fleet to count aircraft types from
         */
        template <typename Fleet>
        void set_aircraft_counts(const Fleet& fleet)
        {
            // Reset counts
            for (auto& [type, count] : aircraft_counts_)
            {
                count = 0;
            }
            
            // Count aircraft by type
            for (const auto& aircraft : fleet)
            {
                aircraft_counts_[aircraft->get_type()]++;
            }
        }

        // allow mock classes to override (since not allowed with template methods)
        virtual void record_flight(AircraftType type, double flight_time, double distance, int passengers)
        {
            stats_[type].add_flight(flight_time, distance, passengers);
        }

        template <typename... MetricArgs>
        void record_flight(AircraftType type, double flight_time, double distance, int passengers, MetricArgs &&...args)
        {
            stats_[type].add_flight(flight_time, distance, passengers);

            if constexpr (sizeof...(args) > 0)
            {
                record_additional_metrics(type, std::forward<MetricArgs>(args)...);
            }
        }
        // allow mock classes to override (since not allowed with template methods)
        virtual void record_charge_session(AircraftType type, double charge_time)
        {
            stats_[type].add_charge_session(charge_time);
        }

        virtual void record_charge_session(AircraftType type, double charge_time, double waiting_time)
        {
            stats_[type].add_charge_session(charge_time, waiting_time);
        }

        virtual void record_waiting_time(AircraftType type, double waiting_time)
        {
            stats_[type].add_waiting_time(waiting_time);
        }

        template <typename... MetricArgs>
        void record_charge_session(AircraftType type, double charge_time, MetricArgs &&...args)
        {
            stats_[type].add_charge_session(charge_time);

            if constexpr (sizeof...(args) > 0)
            {
                record_additional_metrics(type, std::forward<MetricArgs>(args)...);
            }
        }

        virtual void record_fault(AircraftType type)
        {
            stats_[type].add_fault();
        }

        virtual void record_partial_flight(AircraftType type, double flight_time, double distance, int passengers)
        {
            stats_[type].add_partial_flight(flight_time, distance, passengers);
        }

        virtual void record_partial_charge(AircraftType type, double charge_time)
        {
            stats_[type].add_partial_charge(charge_time);
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
            return stats_.at(type);
        }

        template <typename Predicate>
        auto get_filtered_stats(Predicate pred) const
        {
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
            return agg(stats_);
        }

        std::string generate_report() const
        {
            std::ostringstream oss;

            oss << std::fixed << std::setprecision(2);
            oss << "\n========== eVTOL Simulation Results ==========\n\n";

            for (const auto &[type, stats] : stats_)
            {
                int count = aircraft_counts_.at(type);
                oss << aircraft_type_names[static_cast<int>(type)] << " Aircraft(" << count << "):\n";
                oss << "  Average Flight Time: " << stats.avg_flight_time() << " hours\n";
                oss << "  Average Distance: " << stats.avg_distance() << " miles\n";
                oss << "  Average Charging Time: " << stats.avg_charging_time() << " hours\n";
                oss << "  Average Waiting Time: " << stats.avg_waiting_time() << " hours\n";
                oss << "  Average Total Charge Time (including waiting): " << stats.avg_total_charge_time() << " hours\n";
                oss << "  Total Faults: " << stats.total_faults << "\n";
                oss << "  Total Passenger Miles: " << stats.total_passenger_miles << "\n";
                oss << "  Total Flights: " << stats.flight_count << "\n";
                oss << "  Total Charge Sessions: " << stats.charge_count << "\n";
                
                // Add partial activities reporting
                if (stats.partial_flight_count > 0 || stats.partial_charge_count > 0) {
                    oss << "  --- Partial Activities (when simulation ended) ---\n";
                    if (stats.partial_flight_count > 0) {
                        oss << "  Partial Flights: " << stats.partial_flight_count << "\n";
                        oss << "  Partial Flight Time: " << stats.partial_flight_time_hours << " hours\n";
                        oss << "  Partial Distance: " << stats.partial_distance_miles << " miles\n";
                        oss << "  Partial Passenger Miles: " << stats.partial_passenger_miles << "\n";
                    }
                    if (stats.partial_charge_count > 0) {
                        oss << "  Partial Charges: " << stats.partial_charge_count << "\n";
                        oss << "  Partial Charging Time: " << stats.partial_charging_time_hours << " hours\n";
                    }
                }
                
                oss << "\n";
            }

            return oss.str();
        }

        SummaryStats get_summary_stats() const
        {
            SummaryStats summary;

            for (const auto &[type, stats] : stats_)
            {
                summary.total_flight_time += stats.total_flight_time_hours;
                summary.total_distance += stats.total_distance_miles;
                summary.total_charging_time += stats.total_charging_time_hours;
                summary.total_waiting_time += stats.total_waiting_time_hours;
                summary.total_faults += stats.total_faults;
                summary.total_passenger_miles += stats.total_passenger_miles;
                summary.total_flights += stats.flight_count;
                summary.total_charges += stats.charge_count;
                
                // Add partial activities to summary
                summary.partial_flight_time += stats.partial_flight_time_hours;
                summary.partial_distance += stats.partial_distance_miles;
                summary.partial_charging_time += stats.partial_charging_time_hours;
                summary.partial_passenger_miles += stats.partial_passenger_miles;
                summary.partial_flights += stats.partial_flight_count;
                summary.partial_charges += stats.partial_charge_count;
            }

            return summary;
        }

        template <typename ComparisonFunc>
        AircraftType get_best_performing(ComparisonFunc comp) const
        {
            auto best_it = stats_.begin();
            for (auto it = stats_.begin(); it != stats_.end(); ++it)
            {
                if (comp(it->second, best_it->second))
                {
                    best_it = it;
                }
            }

            return best_it->first;
        }

        void reset_stats()
        {
            for (auto &[type, stats] : stats_)
            {
                stats = FlightStats{};
            }
        }
    };

}