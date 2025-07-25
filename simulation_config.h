#pragma once
#include <thread>
#include "simulation_interface.h"

namespace evtol
{
    /**
     * Configuration for simulation engines
     */
    struct SimulationConfig
    {
        SimulationMode mode = SimulationMode::EVENT_DRIVEN;
        double simulation_duration_hours = 3.0;
        
        // Frame-based specific settings
        double frame_time_seconds = 60.0;  // 1 minute frames
        
        // Performance settings
        bool enable_detailed_logging = false;
        bool enable_partial_flights = true;
        
        /**
         * Parse configuration from command line arguments
         * @param argc Argument count
         * @param argv Argument values
         */
        void parse_args(int argc, char* argv[]);
        
        /**
         * Validate configuration settings
         * @return True if configuration is valid
         */
        bool validate() const;
    };
}