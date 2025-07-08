#pragma once
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "aircraft_state.h"
#include "charger_manager.h"
#include "statistics_engine.h"

// Forward declaration
namespace evtol {
    struct SimulationConfig;
}

namespace evtol
{
    /**
     * Information about current simulation state for monitoring
     */
    struct SimulationSnapshot
    {
        double current_time_hours;
        double simulation_duration_hours;
        bool is_running;
        bool is_paused;
        double speed_multiplier;
        
        struct AircraftInfo
        {
            int id;
            std::string type;
            AircraftState state;
            double time_remaining;
            double battery_level;
            int charger_id;
            bool fault_occurred;
        };
        
        std::vector<AircraftInfo> aircraft_states;
        
        struct ChargerInfo
        {
            int id;
            bool is_occupied;
            int aircraft_id;
            std::vector<int> waiting_queue;
        };
        
        std::vector<ChargerInfo> charger_states;
        
        // Current statistics
        SummaryStats current_stats;
    };

    /**
     * Interface for monitoring simulation progress
     */
    class ISimulationMonitor
    {
    public:
        virtual ~ISimulationMonitor() = default;
        
        /**
         * Called at the start of simulation
         * @param config Simulation configuration
         */
        virtual void on_simulation_start(const SimulationConfig& config) = 0;
        
        /**
         * Called at the end of simulation
         * @param final_stats Final statistics
         */
        virtual void on_simulation_end(const SummaryStats& final_stats) = 0;
        
        /**
         * Called every frame with current simulation state
         * @param snapshot Current simulation snapshot
         */
        virtual void on_frame_update(const SimulationSnapshot& snapshot) = 0;
        
        /**
         * Called when aircraft state changes
         * @param aircraft_id ID of the aircraft
         * @param old_state Previous state
         * @param new_state New state
         */
        virtual void on_aircraft_state_change(int aircraft_id, AircraftState old_state, AircraftState new_state) = 0;
        
        /**
         * Called when a fault occurs
         * @param aircraft_id ID of the aircraft
         * @param fault_time Time when fault occurred
         */
        virtual void on_fault_occurred(int aircraft_id, double fault_time) = 0;
        
        /**
         * Check if monitor wants to pause simulation
         * @return True if simulation should pause
         */
        virtual bool should_pause() = 0;
        
        /**
         * Check if monitor wants to stop simulation
         * @return True if simulation should stop
         */
        virtual bool should_stop() = 0;
    };

    /**
     * Console-based simulation monitor
     */
    class ConsoleSimulationMonitor : public ISimulationMonitor
    {
    private:
        bool show_aircraft_states_;
        bool show_charger_status_;
        bool show_statistics_;
        bool pause_on_fault_;
        double last_update_time_;
        double update_interval_;
        
    public:
        ConsoleSimulationMonitor(bool show_aircraft = true, bool show_chargers = true, 
                               bool show_stats = true, bool pause_on_fault = false,
                               double update_interval = 60.0);
        
        void on_simulation_start(const SimulationConfig& config) override;
        void on_simulation_end(const SummaryStats& final_stats) override;
        void on_frame_update(const SimulationSnapshot& snapshot) override;
        void on_aircraft_state_change(int aircraft_id, AircraftState old_state, AircraftState new_state) override;
        void on_fault_occurred(int aircraft_id, double fault_time) override;
        bool should_pause() override;
        bool should_stop() override;
        
    private:
        void print_aircraft_states(const std::vector<SimulationSnapshot::AircraftInfo>& aircraft);
        void print_charger_status(const std::vector<SimulationSnapshot::ChargerInfo>& chargers);
        void print_statistics(const SummaryStats& stats);
        void clear_screen();
    };

    /**
     * Null monitor that does nothing (for performance)
     */
    class NullSimulationMonitor : public ISimulationMonitor
    {
    public:
        void on_simulation_start(const SimulationConfig& /* config */) override {}
        void on_simulation_end(const SummaryStats& /* final_stats */) override {}
        void on_frame_update(const SimulationSnapshot& /* snapshot */) override {}
        void on_aircraft_state_change(int /* aircraft_id */, AircraftState /* old_state */, AircraftState /* new_state */) override {}
        void on_fault_occurred(int /* aircraft_id */, double /* fault_time */) override {}
        bool should_pause() override { return false; }
        bool should_stop() override { return false; }
    };
}