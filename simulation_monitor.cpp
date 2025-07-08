#include "simulation_monitor.h"
#include "simulation_config.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>

namespace evtol
{
    ConsoleSimulationMonitor::ConsoleSimulationMonitor(bool show_aircraft, bool show_chargers, 
                                                     bool show_stats, bool pause_on_fault,
                                                     double update_interval)
        : show_aircraft_states_(show_aircraft)
        , show_charger_status_(show_chargers)
        , show_statistics_(show_stats)
        , pause_on_fault_(pause_on_fault)
        , last_update_time_(0.0)
        , update_interval_(update_interval)
    {
    }

    void ConsoleSimulationMonitor::on_simulation_start(const SimulationConfig& config)
    {
        clear_screen();
        
        std::cout << "=== eVTOL Simulation Started ===" << std::endl;
        std::cout << "Mode: " << (config.mode == SimulationMode::FRAME_BASED ? "Frame-Based" : "Event-Driven") << std::endl;
        std::cout << "Duration: " << config.simulation_duration_hours << " hours" << std::endl;
        
        if (config.mode == SimulationMode::FRAME_BASED)
        {
            std::cout << "Frame Time: " << config.frame_time_seconds << " seconds" << std::endl;
            std::cout << "Threads: " << config.num_threads << std::endl;
            std::cout << "Real-time: " << (config.enable_real_time ? "Yes" : "No") << std::endl;
            std::cout << "Speed Multiplier: " << config.speed_multiplier << "x" << std::endl;
        }
        
        std::cout << "================================" << std::endl << std::endl;
    }

    void ConsoleSimulationMonitor::on_simulation_end(const SummaryStats& final_stats)
    {
        std::cout << std::endl << "=== Simulation Completed ===" << std::endl;
        print_statistics(final_stats);
        std::cout << "============================" << std::endl;
    }

    void ConsoleSimulationMonitor::on_frame_update(const SimulationSnapshot& snapshot)
    {
        // Only update display at specified intervals
        if (snapshot.current_time_hours - last_update_time_ < update_interval_ / 3600.0)
        {
            return;
        }
        
        last_update_time_ = snapshot.current_time_hours;
        
        // Clear screen for live updates
        clear_screen();
        
        // Show header
        std::cout << "eVTOL Simulation - Time: " << std::fixed << std::setprecision(2) 
                  << snapshot.current_time_hours << " / " << snapshot.simulation_duration_hours << " hours";
        
        if (snapshot.is_paused)
        {
            std::cout << " [PAUSED]";
        }
        
        if (snapshot.speed_multiplier != 1.0)
        {
            std::cout << " (Speed: " << snapshot.speed_multiplier << "x)";
        }
        
        std::cout << std::endl;
        
        // Progress bar
        double progress = snapshot.current_time_hours / snapshot.simulation_duration_hours;
        int bar_width = 50;
        int filled = static_cast<int>(progress * bar_width);
        
        std::cout << "[";
        for (int i = 0; i < bar_width; ++i)
        {
            if (i < filled)
                std::cout << "=";
            else if (i == filled)
                std::cout << ">";
            else
                std::cout << " ";
        }
        std::cout << "] " << std::fixed << std::setprecision(1) << (progress * 100.0) << "%" << std::endl;
        std::cout << std::endl;
        
        // Show aircraft states
        if (show_aircraft_states_)
        {
            print_aircraft_states(snapshot.aircraft_states);
        }
        
        // Show charger status
        if (show_charger_status_)
        {
            print_charger_status(snapshot.charger_states);
        }
        
        // Show statistics
        if (show_statistics_)
        {
            print_statistics(snapshot.current_stats);
        }
        
        std::cout << std::endl << "Press Ctrl+C to stop simulation" << std::endl;
    }

    void ConsoleSimulationMonitor::on_aircraft_state_change(int aircraft_id, AircraftState old_state, AircraftState new_state)
    {
        if (show_aircraft_states_)
        {
            std::cout << "Aircraft " << aircraft_id << ": " 
                      << AircraftStateMachine::state_to_string(old_state) << " -> " 
                      << AircraftStateMachine::state_to_string(new_state) << std::endl;
        }
    }

    void ConsoleSimulationMonitor::on_fault_occurred(int aircraft_id, double fault_time)
    {
        std::cout << "*** FAULT *** Aircraft " << aircraft_id << " at time " 
                  << std::fixed << std::setprecision(2) << fault_time << " hours" << std::endl;
    }

    bool ConsoleSimulationMonitor::should_pause()
    {
        // Could implement keyboard input checking here
        return false;
    }

    bool ConsoleSimulationMonitor::should_stop()
    {
        // Could implement keyboard input checking here
        return false;
    }

    void ConsoleSimulationMonitor::print_aircraft_states(const std::vector<SimulationSnapshot::AircraftInfo>& aircraft)
    {
        std::cout << "Aircraft States:" << std::endl;
        std::cout << "ID   Type     State                 Time Remaining  Battery  Charger  Fault" << std::endl;
        std::cout << "---- -------- --------------------- --------------- -------- -------- -----" << std::endl;
        
        for (const auto& info : aircraft)
        {
            std::cout << std::setw(4) << info.id << " "
                      << std::setw(8) << info.type << " "
                      << std::setw(21) << AircraftStateMachine::state_to_string(info.state) << " "
                      << std::setw(15) << std::fixed << std::setprecision(1) << info.time_remaining << " "
                      << std::setw(8) << std::fixed << std::setprecision(1) << info.battery_level << " "
                      << std::setw(8) << (info.charger_id >= 0 ? std::to_string(info.charger_id) : "N/A") << " "
                      << std::setw(5) << (info.fault_occurred ? "YES" : "NO") << std::endl;
        }
        std::cout << std::endl;
    }

    void ConsoleSimulationMonitor::print_charger_status(const std::vector<SimulationSnapshot::ChargerInfo>& chargers)
    {
        std::cout << "Charger Status:" << std::endl;
        std::cout << "ID   Status     Aircraft   Queue Length" << std::endl;
        std::cout << "---- ---------- ---------- ------------" << std::endl;
        
        for (const auto& info : chargers)
        {
            std::cout << std::setw(4) << info.id << " "
                      << std::setw(10) << (info.is_occupied ? "OCCUPIED" : "AVAILABLE") << " "
                      << std::setw(10) << (info.aircraft_id >= 0 ? std::to_string(info.aircraft_id) : "N/A") << " "
                      << std::setw(12) << info.waiting_queue.size() << std::endl;
        }
        
        // Show waiting queue if not empty
        if (!chargers.empty() && !chargers[0].waiting_queue.empty())
        {
            std::cout << std::endl << "Waiting Queue: ";
            for (size_t i = 0; i < chargers[0].waiting_queue.size(); ++i)
            {
                if (i > 0) std::cout << ", ";
                std::cout << chargers[0].waiting_queue[i];
            }
            std::cout << std::endl;
        }
        
        std::cout << std::endl;
    }

    void ConsoleSimulationMonitor::print_statistics(const SummaryStats& stats)
    {
        std::cout << "Current Statistics:" << std::endl;
        std::cout << "Total Flights: " << stats.total_flights << std::endl;
        std::cout << "Total Charge Sessions: " << stats.total_charges << std::endl;
        std::cout << "Total Faults: " << stats.total_faults << std::endl;
        std::cout << "Total Passenger Miles: " << std::fixed << std::setprecision(1) << stats.total_passenger_miles << std::endl;
        
        if (stats.total_flights > 0)
        {
            std::cout << "Average Flight Time: " << std::fixed << std::setprecision(2) 
                      << stats.total_flight_time / stats.total_flights << " hours" << std::endl;
        }
        
        if (stats.total_charges > 0)
        {
            std::cout << "Average Charge Time: " << std::fixed << std::setprecision(2) 
                      << stats.total_charging_time / stats.total_charges << " hours" << std::endl;
        }
        
        std::cout << std::endl;
    }

    void ConsoleSimulationMonitor::clear_screen()
    {
        // Clear screen using ANSI escape codes (works on most terminals)
        std::cout << "\033[2J\033[H";
        std::cout.flush();
    }
}