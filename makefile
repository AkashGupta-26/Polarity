# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -std=c++17 -fno-sized-deallocation
OPTFLAGS := -O3 -march=native -flto
DEBUGFLAGS := -g
DEPFLAGS := -MMD -MP

# Static linking flags
STATICFLAGS := -static -static-libgcc -static-libstdc++

# Directories
SRC_DIR := src
UTIL_DIR := utilities
BUILD_DIR := build

# Output name (override with: make engine EXE=myengine)
EXE := engine

# Source and object files
ENGINE_SRC := $(SRC_DIR)/engine.cpp
ENGINE_OBJ := $(BUILD_DIR)/engine.o

PERFT_SRC := $(UTIL_DIR)/perftValidate.cpp
PERFT_OBJ := $(BUILD_DIR)/perftValidate.o

MATCH_SRC := $(UTIL_DIR)/match.cpp
MATCH_OBJ := $(BUILD_DIR)/match.o

TUNER_SRC := $(SRC_DIR)/tuner.cpp
TUNER_OBJ := $(BUILD_DIR)/tuner.o

# Default target
all: CXXFLAGS += $(OPTFLAGS)
all: LDFLAGS += $(STATICFLAGS)
all: engine perftValidate match

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: LDFLAGS += $(STATICFLAGS)
debug: engine perftValidate match

# Ensure build directory exists
$(BUILD_DIR):
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

# Build targets
engine: $(ENGINE_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(EXE) $^

perftValidate: $(PERFT_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

match: $(MATCH_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -pthread -o $@ $^

tuner: CXXFLAGS += $(OPTFLAGS) -DTUNING_MODE
tuner: LDFLAGS += $(STATICFLAGS)
tuner: $(TUNER_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

# Compile src/ files
$(BUILD_DIR)/engine.o: $(ENGINE_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/tuner.o: $(TUNER_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Compile utilities/ files
$(BUILD_DIR)/perftValidate.o: $(PERFT_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/match.o: $(MATCH_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Include dependency files if they exist
-include $(BUILD_DIR)/*.d

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(EXE) perftValidate match tuner

.PHONY: all debug clean engine perftValidate match tuner
