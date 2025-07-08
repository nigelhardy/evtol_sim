#pragma once
#include <memory>
#include "simulation_interface.h"
#include "event_driven_simulation.h"

namespace evtol
{
    /**
     * Wrapper for the EventDrivenSimulation class to implement the ISimulationEngine interface
     * This provides a consistent interface for the simulation factory and runner
     */
    class EventDrivenSimulationEngine : public SimulationEngineBase
    {
    private:
        std::unique_ptr<EventDrivenSimulation> simulation_;

    public:
        EventDrivenSimulationEngine(StatisticsCollector &stats, double duration_hours = 3.0)
            : SimulationEngineBase(stats, duration_hours), 
              simulation_(std::make_unique<EventDrivenSimulation>(stats, duration_hours))
        {
        }

    protected:
        void run_simulation_impl(ChargerManager &charger_mgr, void *fleet_ptr) override
        {
            is_running_ = true;

            // Type-erase back to template - this is a limitation of the current design
            // In a real implementation, we might use std::function or type erasure
            // For now, we'll use a simple approach
            auto *fleet = static_cast<std::vector<std::unique_ptr<AircraftBase>> *>(fleet_ptr);
            simulation_->run_simulation(charger_mgr, *fleet);

            // Update our time tracking from the simulation
            current_time_hours_ = simulation_->get_current_time();
            
            is_running_ = false;
        }
    };
}