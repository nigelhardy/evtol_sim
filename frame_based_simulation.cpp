#include "frame_based_simulation.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace evtol
{
    FrameBasedSimulationEngine::FrameBasedSimulationEngine(StatisticsCollector &stats, const SimulationConfig &config)
        : SimulationEngineBase(stats, config.simulation_duration_hours), config_(config), frame_time_seconds_(config.frame_time_seconds)
    {
        // Validate configuration
        if (!config_.validate())
        {
            throw std::invalid_argument("Invalid simulation configuration");
        }
    }

    void FrameBasedSimulationEngine::run_simulation_impl(ChargerManager &charger_mgr, void *fleet_ptr)
    {
        // Type-erase back to template - cast to the expected fleet type
        auto *fleet = static_cast<std::vector<std::unique_ptr<AircraftBase>> *>(fleet_ptr);
        run_frame_based_simulation(charger_mgr, *fleet);
    }

    // Need to check the existing charger manager interface to make sure these methods exist
    // For now, I'll implement placeholder methods that would need to be added to ChargerManager
    // TODO check this one is good
    void FrameBasedSimulationEngine::handle_partial_flight(std::unique_ptr<AircraftBase> &aircraft, AircraftFrameData &frame_data)
    {
        // Calculate how much of the flight was completed
        double total_flight_time = frame_data.current_flight_time_hrs;
        double remaining_time_seconds = frame_data.time_remaining_sec;
        double completed_flight_time = total_flight_time - (remaining_time_seconds / 3600.0); // Convert to hours

        // Calculate partial distance
        double partial_distance = (completed_flight_time / total_flight_time) * frame_data.current_flight_distance;

        if (config_.enable_detailed_logging)
        {
            log_event("Processing partial flight for aircraft " + std::to_string(aircraft->get_id()) +
                      " (flew " + std::to_string(completed_flight_time) + "h/" + std::to_string(total_flight_time) +
                      "h, " + std::to_string(partial_distance) + "/" + std::to_string(frame_data.current_flight_distance) + " miles)");
        }

        // Record partial flight statistics
        stats_collector_.record_partial_flight(aircraft->get_type(), completed_flight_time, partial_distance, aircraft->get_passenger_count());
    }
    // TODO check this
    void FrameBasedSimulationEngine::handle_partial_charging(std::unique_ptr<AircraftBase> &aircraft, AircraftFrameData &frame_data)
    {
        // Calculate how much charging was completed
        double total_charge_time = aircraft->get_charge_time_hours();
        double remaining_time_seconds = frame_data.time_remaining_sec;
        double completed_charge_time = total_charge_time - (remaining_time_seconds / 3600.0); // Convert to hours

        if (config_.enable_detailed_logging)
        {
            log_event("Processing partial charge for aircraft " + std::to_string(aircraft->get_id()) +
                      " (charged " + std::to_string(completed_charge_time) + "h/" + std::to_string(total_charge_time) +
                      "h, waited: " + std::to_string(frame_data.accumulated_waiting_time_sec / 3600.0) + "h)");
        }

        // Record partial charging statistics
        stats_collector_.record_partial_charge(aircraft->get_type(), completed_charge_time);
    }

    std::string FrameBasedSimulationEngine::aircraft_type_to_string(AircraftType type)
    {
        static const char *aircraft_type_names[] = {
            "Alpha", "Beta", "Charlie", "Delta", "Echo"};

        int index = static_cast<int>(type);
        if (index >= 0 && index < 5)
        {
            return std::string(aircraft_type_names[index]);
        }
        return "Unknown";
    }
}