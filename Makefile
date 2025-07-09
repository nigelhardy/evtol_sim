# eVTOL Aircraft Simulation Makefile

# Compiler configuration
CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -Wshadow
DEBUG_FLAGS = -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined
TEST_FLAGS = -g -O0 -DDEBUG

# Project configuration
TARGET = evtolsim
SOURCES = evtol_sim.cpp aircraft_state.cpp simulation_config.cpp frame_based_simulation.cpp \
          event_driven_simulation.cpp \
          
HEADERS = aircraft.h aircraft_types.h charger_manager.h statistics_engine.h \
          simulation_interface.h simulation_factory.h simulation_config.h aircraft_state.h \
          frame_based_simulation.h event_driven_simulation.h \
          simulation_runner.h

# Test configuration
TEST_DIR = tests
TEST_BUILD_DIR = $(BUILD_DIR)/test
TEST_TARGET = evtol_tests
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJECTS = $(TEST_SOURCES:%.cpp=$(TEST_BUILD_DIR)/%.o)

# Google Test configuration
GTEST_PREFIX = /opt/homebrew/opt/googletest
GTEST_INCLUDE = -I$(GTEST_PREFIX)/include
GTEST_LIB = -L$(GTEST_PREFIX)/lib
GTEST_FLAGS = $(GTEST_LIB) -lgtest -lgtest_main -pthread

# Build directories
BUILD_DIR = build
DEBUG_DIR = $(BUILD_DIR)/debug

# Default target
.PHONY: all
all: debug

# Debug build (with sanitizers)
.PHONY: debug
debug: $(DEBUG_DIR)/$(TARGET)

$(DEBUG_DIR)/$(TARGET): $(SOURCES) $(HEADERS) | $(DEBUG_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $(SOURCES)

# Test targets
.PHONY: test
test: test-build
	./$(TEST_BUILD_DIR)/$(TEST_TARGET)

.PHONY: test-build
test-build: $(TEST_BUILD_DIR)/$(TEST_TARGET)

$(TEST_BUILD_DIR)/$(TEST_TARGET): $(TEST_OBJECTS) | $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_FLAGS) -o $@ $(TEST_OBJECTS) $(GTEST_FLAGS)

# Test object compilation
$(TEST_BUILD_DIR)/%.o: %.cpp $(HEADERS) | $(TEST_BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(TEST_FLAGS) $(GTEST_INCLUDE) -I. -c $< -o $@

# Individual test targets
.PHONY: test-aircraft
test-aircraft: test-build
	./$(TEST_BUILD_DIR)/$(TEST_TARGET) --gtest_filter="Aircraft*"

.PHONY: test-charger
test-charger: test-build
	./$(TEST_BUILD_DIR)/$(TEST_TARGET) --gtest_filter="ChargerManager*"

.PHONY: test-stats
test-stats: test-build
	./$(TEST_BUILD_DIR)/$(TEST_TARGET) --gtest_filter="Statistics*"

.PHONY: test-simulation
test-simulation: test-build
	./$(TEST_BUILD_DIR)/$(TEST_TARGET) --gtest_filter="Simulation*"

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(DEBUG_DIR): | $(BUILD_DIR)
	mkdir -p $(DEBUG_DIR)

$(TEST_BUILD_DIR): | $(BUILD_DIR)
	mkdir -p $(TEST_BUILD_DIR)

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

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  debug          - Build debug version with sanitizers"
	@echo "  test           - Build and run all tests"
	@echo "  test-build     - Build test executable only"
	@echo "  test-aircraft  - Run only aircraft-related tests"
	@echo "  test-charger   - Run only charger manager tests"
	@echo "  test-stats     - Run only statistics tests"
	@echo "  test-simulation - Run only simulation engine tests"
	@echo "  run-debug      - Run debug build"
	@echo "  clean          - Remove build files"
	@echo "  clean-all      - Remove all generated files"

