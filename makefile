# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -std=c++17 -fno-sized-deallocation
OPTFLAGS := -O3 -march=native -flto
DEBUGFLAGS := -g
DEPFLAGS := -MMD -MP

# Source and object files
SRCS := engine.cpp perftValidate.cpp
OBJS := $(SRCS:.cpp=.o)
DEPS := $(SRCS:.cpp=.d)

TARGETS := engine perftValidate

# Default target
all: CXXFLAGS += $(OPTFLAGS)
all: $(TARGETS)

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: $(TARGETS)

# Build targets
engine: engine.o
	$(CXX) $(CXXFLAGS) -o $@ $^

perftValidate: perftValidate.o
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile .cpp into .o and generate .d files for header tracking
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Include dependency files if they exist
-include $(DEPS)

# Clean build artifacts
clean:
	rm -f *.o *.d $(TARGETS)

.PHONY: all debug clean
