# eVTOL Aircraft Simulation Makefile

# Compiler configuration
CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -Wshadow
DEBUG_FLAGS = -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined

# Project configuration
TARGET = evtolsim
SOURCE = evtol_sim.cpp
HEADERS = aircraft.h aircraft_types.h simulation_engine.h charger_manager.h

# Build directories
BUILD_DIR = build
DEBUG_DIR = $(BUILD_DIR)/debug

# Default target
.PHONY: all
all: debug

# Debug build (with sanitizers)
.PHONY: debug
debug: $(DEBUG_DIR)/$(TARGET)

$(DEBUG_DIR)/$(TARGET): $(SOURCE) $(HEADERS) | $(DEBUG_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $(SOURCE)

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(DEBUG_DIR): | $(BUILD_DIR)
	mkdir -p $(DEBUG_DIR)

.PHONY: run-debug
run-debug: debug
	./$(DEBUG_DIR)/$(TARGET)

# Clean targets
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET) *.o *.s *.i

.PHONY: clean-all
clean-all: clean
	rm -f evtolsim

