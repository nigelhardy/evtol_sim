#pragma once
#include <memory>
#include "simulation_interface.h"
#include "simulation_config.h"
#include "event_driven_simulation_engine.h"
#include "frame_based_simulation.h"

namespace evtol
{
    /**
     * Factory for creating simulation engines
     */
    class SimulationFactory
    {
    public:
        /**
         * Create a simulation engine based on configuration
         * @param config Simulation configuration
         * @param stats Statistics collector
         * @return Unique pointer to simulation engine
         */
        static std::unique_ptr<ISimulationEngine> create_engine(
            const SimulationConfig &config,
            StatisticsCollector &stats)
        {
            switch (config.mode)
            {
            case SimulationMode::EVENT_DRIVEN:
                return std::make_unique<EventDrivenSimulationEngine>(stats, config.simulation_duration_hours);

            case SimulationMode::FRAME_BASED:
                return std::make_unique<FrameBasedSimulationEngine>(stats, config);

            default:
                throw std::invalid_argument("Unknown simulation mode");
            }
        }

        /**
         * Create a complete simulation setup
         * @param config Simulation configuration
         * @param stats Statistics collector
         * @return engine
         */
        static std::unique_ptr<ISimulationEngine>
        create_simulation_setup(const SimulationConfig &config, StatisticsCollector &stats)
        {
            return create_engine(config, stats);
        }
    };
}