#include "simulation_config.h"
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace evtol
{
    void SimulationConfig::parse_args(int argc, char* argv[])
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
            else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc)
            {
                num_threads = std::stoi(argv[++i]);
            }
            else if (strcmp(argv[i], "--speed") == 0 && i + 1 < argc)
            {
                speed_multiplier = std::stod(argv[++i]);
            }
            else if (strcmp(argv[i], "--visualization") == 0)
            {
                enable_visualization = true;
            }
            else if (strcmp(argv[i], "--no-visualization") == 0)
            {
                enable_visualization = false;
            }
            else if (strcmp(argv[i], "--real-time") == 0)
            {
                enable_real_time = true;
            }
            else if (strcmp(argv[i], "--no-real-time") == 0)
            {
                enable_real_time = false;
            }
            else if (strcmp(argv[i], "--pause-on-fault") == 0)
            {
                pause_on_fault = true;
            }
            else if (strcmp(argv[i], "--no-multithreading") == 0)
            {
                enable_multithreading = false;
            }
            else if (strcmp(argv[i], "--detailed-logging") == 0)
            {
                enable_detailed_logging = true;
            }
            else if (strcmp(argv[i], "--help") == 0)
            {
                std::cout << "eVTOL Simulation Options:" << std::endl;
                std::cout << "  --frame-based              Use frame-based simulation" << std::endl;
                std::cout << "  --event-driven             Use event-driven simulation (default)" << std::endl;
                std::cout << "  --duration <hours>         Simulation duration in hours (default: 3.0)" << std::endl;
                std::cout << "  --frame-time <seconds>     Frame time in seconds (default: 60.0)" << std::endl;
                std::cout << "  --threads <count>          Number of threads (default: auto)" << std::endl;
                std::cout << "  --speed <multiplier>       Speed multiplier (default: 1.0)" << std::endl;
                std::cout << "  --visualization            Enable visualization" << std::endl;
                std::cout << "  --no-visualization         Disable visualization" << std::endl;
                std::cout << "  --real-time                Enable real-time simulation" << std::endl;
                std::cout << "  --no-real-time             Disable real-time simulation" << std::endl;
                std::cout << "  --pause-on-fault           Pause simulation on fault" << std::endl;
                std::cout << "  --no-multithreading        Disable multithreading" << std::endl;
                std::cout << "  --detailed-logging         Enable detailed logging" << std::endl;
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

    void SimulationConfig::load_from_env()
    {
        // Load from environment variables
        const char* env_mode = std::getenv("EVTOL_MODE");
        if (env_mode)
        {
            if (strcmp(env_mode, "frame_based") == 0)
            {
                mode = SimulationMode::FRAME_BASED;
            }
            else if (strcmp(env_mode, "event_driven") == 0)
            {
                mode = SimulationMode::EVENT_DRIVEN;
            }
        }
        
        const char* env_duration = std::getenv("EVTOL_DURATION");
        if (env_duration)
        {
            simulation_duration_hours = std::stod(env_duration);
        }
        
        const char* env_frame_time = std::getenv("EVTOL_FRAME_TIME");
        if (env_frame_time)
        {
            frame_time_seconds = std::stod(env_frame_time);
        }
        
        const char* env_threads = std::getenv("EVTOL_THREADS");
        if (env_threads)
        {
            num_threads = std::stoi(env_threads);
        }
        
        const char* env_speed = std::getenv("EVTOL_SPEED");
        if (env_speed)
        {
            speed_multiplier = std::stod(env_speed);
        }
        
        const char* env_visualization = std::getenv("EVTOL_VISUALIZATION");
        if (env_visualization)
        {
            enable_visualization = (strcmp(env_visualization, "true") == 0 || 
                                   strcmp(env_visualization, "1") == 0);
        }
        
        const char* env_real_time = std::getenv("EVTOL_REAL_TIME");
        if (env_real_time)
        {
            enable_real_time = (strcmp(env_real_time, "true") == 0 || 
                               strcmp(env_real_time, "1") == 0);
        }
        
        const char* env_pause_on_fault = std::getenv("EVTOL_PAUSE_ON_FAULT");
        if (env_pause_on_fault)
        {
            pause_on_fault = (strcmp(env_pause_on_fault, "true") == 0 || 
                             strcmp(env_pause_on_fault, "1") == 0);
        }
        
        const char* env_multithreading = std::getenv("EVTOL_MULTITHREADING");
        if (env_multithreading)
        {
            enable_multithreading = (strcmp(env_multithreading, "true") == 0 || 
                                    strcmp(env_multithreading, "1") == 0);
        }
        
        const char* env_detailed_logging = std::getenv("EVTOL_DETAILED_LOGGING");
        if (env_detailed_logging)
        {
            enable_detailed_logging = (strcmp(env_detailed_logging, "true") == 0 || 
                                      strcmp(env_detailed_logging, "1") == 0);
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
        
        if (num_threads <= 0)
        {
            std::cerr << "Error: Number of threads must be positive" << std::endl;
            return false;
        }
        
        if (speed_multiplier <= 0.0)
        {
            std::cerr << "Error: Speed multiplier must be positive" << std::endl;
            return false;
        }
        
        // Warn about potentially problematic settings
        if (frame_time_seconds > 300.0)  // 5 minutes
        {
            std::cerr << "Warning: Frame time is quite large (" << frame_time_seconds 
                      << " seconds). This may reduce simulation accuracy." << std::endl;
        }
        
        if (mode == SimulationMode::FRAME_BASED && num_threads > 32)
        {
            std::cerr << "Warning: Large number of threads (" << num_threads 
                      << ") may not improve performance." << std::endl;
        }
        
        if (enable_real_time && speed_multiplier > 10.0)
        {
            std::cerr << "Warning: High speed multiplier (" << speed_multiplier 
                      << ") with real-time mode may not work as expected." << std::endl;
        }
        
        return true;
    }
}