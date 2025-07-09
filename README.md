# eVTOL Aircraft Sim

A C++ simulation engine for electric vertical takeoff and landing (eVTOL) aircraft flights.
Includes simple charging infrastructure, per vehicle characteristics, and basic statistics recording.

## Overview

The simulation models a fleet of 20 eVTOL aircraft with 5 different aircraft types, 3 charging stations, and defaults to a 3.0 hour duration.

Assumptions: 
1. When a fault occurs, the flight is able to complete then the aircraft is grounded.
   - For event based simulation, I assuemd it is okay to calculate chance of fault before flight is complete

## Basic Features
- Two simulation modes: Event-driven and Frame-based
- 5 aircraft types with unique attributes
- Charging infrastructure with queuing system
- Simple fault modeling
- Simple statistics and reporting
- Command-line configuration options
- Basic test suite with 20 core tests

## Project Structure

### Core Files

Main Application:
- `evtol_sim.cpp` - Main simulation entry point with CLI interface

Aircraft System:
- `aircraft.h` - Base aircraft classes and flight calculations
  - `AircraftBase` - Abstract base class for all aircraft
  - `Aircraft<T>` - Template base class with CRTP pattern
  - `AircraftSpec` - Aircraft specification structure
- `aircraft_types.h` - Concrete aircraft implementations
  - `AircraftFactory<T>` - Factory for creating aircraft fleets
- `aircraft_state.h/.cpp` - Aircraft state management and transitions

Sim Engines:
- `simulation_interface.h` - Abstract interfaces and simulation modes
- `event_driven_simulation.h/.cpp` - Priority queue-based event simulation
  - `EventDrivenSimulation` - Core event-driven simulation logic
- `frame_based_simulation.h/.cpp` - Time-stepped frame simulation
  - `FrameBasedSimulation` - Core frame-based simulation logic
  - `FrameBasedSimulationEngine` - Interface-compliant wrapper

Infrastructure with OOP Style:
- `charger_manager.h` - Charging station management
- `statistics_engine.h` - Data collection and reporting
- `simulation_config.h/.cpp` - CLI Configuration
- `simulation_factory.h` - Factory pattern for sim engines
- `simulation_runner.h` - High-level simulation handler

### Test Structure

Core Test Suite (20 tests):
- `test_core_functionality.cpp`
- `test_system_behavior.cpp`
- `test_edge_cases.cpp`
- `test_utilities.h` - includes some mock classes

Some AI Generated Tests (experimental)
- `tests/ai_tests/` - Lots of tests, but not all the most useful

### Build System

- `Makefile` - Build configuration with multiple targets

## Command Line Interface

The simulation supports various command-line options for configuration:

### Usage
./evtolsim [OPTIONS]

### Available Options

Simulation Mode:
- `--event-driven` - Use event-driven simulation (default)
- `--frame-based` - Use frame-based simulation

Timing Configuration:
- `--duration <hours>` - Simulation duration in hours (default: 3.0)
- `--frame-time <seconds>` - Frame time for frame-based mode (default: 60.0)

Logging and Output:
- `--detailed-logging` - Enable detailed simulation logging
- `--no-partial-flights` - Disable partial flights/charging at simulation end

Usage:
- `--help` - Show help message with all options

### Example Commands

```bash
# Run default event-driven simulation
./evtolsim

# Run frame-based simulation with 1-hour duration and default frame time of 60 seconds
./evtolsim --frame-based --duration 1.0

# Run with detailed logging enabled
./evtolsim --detailed-logging

# Run frame-based with 30-second frames
./evtolsim --frame-based --frame-time 30.0
```

## Aircraft Specifications

| Aircraft | Speed (mph) | Battery (kWh) | Charge Time (h) | Passengers | Energy (kWh/mi) | Fault Rate (/h) |
|----------|-------------|---------------|-----------------|------------|-----------------|-----------------|
| Alpha    | 120         | 320           | 0.6             | 4          | 1.6             | 0.25            |
| Beta     | 100         | 100           | 0.2             | 5          | 1.5             | 0.10            |
| Charlie  | 160         | 220           | 0.8             | 3          | 2.2             | 0.05            |
| Delta    | 90          | 120           | 0.62            | 2          | 0.8             | 0.22            |
| Echo     | 30          | 150           | 0.3             | 2          | 5.8             | 0.61            |

## Building and Running

### Prerequisites
- C++20 compatible compiler (clang++ recommended)
- Google Test framework for testing
- Make build system

### Build Commands

```bash
# Build debug version with sanitizers
make debug

# Build and run all tests
make test

# Build tests only
make test-build

# Run simulation
make run-debug
```

### Test Commands

```bash
# Run all tests
make test

# Run specific test categories
make test-aircraft    # Aircraft-related tests
make test-charger     # Charger manager tests
make test-stats       # Statistics tests
make test-simulation  # Simulation engine tests
```

## Simulation Modes

### Event-Driven Simulation
- Uses priority queue for precise event scheduling
- Handles events: flight completion, charging completion, fault occurrence
- Optimal for speed and accuracy (as long as there are no complicated contigency modes for faults)

### Frame-Based Simulation
- Fixed time-step simulation with configurable frame duration
- Could be a good structure for a visualization (I did attempt one, but decided it was too much work)
- Configurable frame time (default: 60 seconds)

## Sample Log Files

The repository contains sample log files demonstrating different simulation outputs:

- `basic_event_driven.log` - Event-driven simulation output
- `basic_frame_based.log` - Frame-based simulation output
- `verbose_event_driven.log` - Detailed event-driven logging
- `verbose_frame_based.log` - Detailed frame-based logging
- `test_suite_results.log` - Test execution results

These logs show typical simulation behavior and can serve as reference outputs.

## Potential Improvements and General Dev Story

1. Multi-threading would be possible for frame-based approach
   - I originally started adding mutexes and atomic variables, but decided it would be more time that I wanted to spend
2. Monitor/Visualization
   - Only really feasible for frame-based

My basic design started as purely an event-based simulation. It felt like the most efficient way to approach this problem.
Then, I started to feel like that approach was too basic, so I added a frame-based implementation configurable at runtime.
I got a little carried away with making everything a template function, so that filled up my header files quite a bit.

I'm used to using Visual Studio, so debugging and writing code was interesting.
I experimented with writing tests with AI, but would have needed to spend more time with my prompts to make them helpful.

TLDR; I spent more focus on OOP/C++ bells and whistles than I should have.
I think focusing on one type of simulation would have been better for the scope of this assignment.
