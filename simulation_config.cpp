#include "simulation_config.h"
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace evtol
{
    void SimulationConfig::parse_args(int argc, char *argv[])
    {
        for (int i = 1; i < argc; ++i)
        {
            if (strcmp(argv[i], "--frame-based") == 0)
            {
                mode = SimulationMode::FRAME_BASED;
            }
            else if (strcmp(argv[i], "--event-driven") == 0)
            {
                mode = SimulationMode::EVENT_DRIVEN;
            }
            else if (strcmp(argv[i], "--duration") == 0 && i + 1 < argc)
            {
                simulation_duration_hours = std::stod(argv[++i]);
            }
            else if (strcmp(argv[i], "--frame-time") == 0 && i + 1 < argc)
            {
                frame_time_seconds = std::stod(argv[++i]);
            }
            else if (strcmp(argv[i], "--detailed-logging") == 0)
            {
                enable_detailed_logging = true;
            }
            else if (strcmp(argv[i], "--no-partial-flights") == 0)
            {
                enable_partial_flights = false;
            }
            else if (strcmp(argv[i], "--help") == 0)
            {
                std::cout << "eVTOL Simulation Options:" << std::endl;
                std::cout << "  --frame-based              Use frame-based simulation" << std::endl;
                std::cout << "  --event-driven             Use event-driven simulation (default)" << std::endl;
                std::cout << "  --duration <hours>         Simulation duration in hours (default: 3.0)" << std::endl;
                std::cout << "  --frame-time <seconds>     Frame time in seconds (default: 60.0)" << std::endl;
                std::cout << "  --detailed-logging         Enable detailed logging" << std::endl;
                std::cout << "  --no-partial-flights       Disable partial flights/charging at simulation end" << std::endl;
                std::cout << "  --help                     Show this help message" << std::endl;
                exit(0);
            }
            else
            {
                std::cerr << "Unknown option: " << argv[i] << std::endl;
                std::cerr << "Use --help for available options" << std::endl;
                exit(1);
            }
        }
    }

    bool SimulationConfig::validate() const
    {
        if (simulation_duration_hours <= 0.0)
        {
            std::cerr << "Error: Simulation duration must be positive" << std::endl;
            return false;
        }

        if (frame_time_seconds <= 0.0)
        {
            std::cerr << "Error: Frame time must be positive" << std::endl;
            return false;
        }

        // Warn about potentially problematic settings
        if (frame_time_seconds > 300.0) // 5 minutes
        {
            std::cerr << "Warning: Frame time is quite large (" << frame_time_seconds
                      << " seconds). This may reduce simulation accuracy." << std::endl;
        }

        return true;
    }
}