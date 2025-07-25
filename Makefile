# eVTOL Aircraft Simulation Makefile

# Compiler configuration
CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -Wshadow
DEBUG_FLAGS = -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined
RELEASE_FLAGS = -O3 -DNDEBUG -flto
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
RELEASE_DIR = $(BUILD_DIR)/release

# Default target
.PHONY: all
all: debug

# Debug build (with sanitizers)
.PHONY: debug
debug: $(DEBUG_DIR)/$(TARGET)

$(DEBUG_DIR)/$(TARGET): $(SOURCES) $(HEADERS) | $(DEBUG_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $(SOURCES)

# Release build (optimized)
.PHONY: release
release: $(RELEASE_DIR)/$(TARGET)

$(RELEASE_DIR)/$(TARGET): $(SOURCES) $(HEADERS) | $(RELEASE_DIR)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -o $@ $(SOURCES)

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
.PHONY: test-core
test-core: test-build
	./$(TEST_BUILD_DIR)/$(TEST_TARGET) --gtest_filter="CoreFunctionalityTest*"

.PHONY: test-behavior
test-behavior: test-build
	./$(TEST_BUILD_DIR)/$(TEST_TARGET) --gtest_filter="SystemBehaviorTest*"

.PHONY: test-edge
test-edge: test-build
	./$(TEST_BUILD_DIR)/$(TEST_TARGET) --gtest_filter="EdgeCasesTest*"

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(DEBUG_DIR): | $(BUILD_DIR)
	mkdir -p $(DEBUG_DIR)

$(RELEASE_DIR): | $(BUILD_DIR)
	mkdir -p $(RELEASE_DIR)

$(TEST_BUILD_DIR): | $(BUILD_DIR)
	mkdir -p $(TEST_BUILD_DIR)

.PHONY: run-debug
run-debug: debug
	./$(DEBUG_DIR)/$(TARGET)

.PHONY: run-release
run-release: release
	./$(RELEASE_DIR)/$(TARGET)

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
	@echo "  release        - Build optimized release version"
	@echo "  test           - Build and run all tests"
	@echo "  test-build     - Build test executable only"
	@echo "  test-core      - Run core functionality tests (8 tests)"
	@echo "  test-behavior  - Run system behavior tests (6 tests)"
	@echo "  test-edge      - Run edge case tests (6 tests)"
	@echo "  run-debug      - Run debug build"
	@echo "  run-release    - Run release build"
	@echo "  clean          - Remove build files"
	@echo "  clean-all      - Remove all generated files"

