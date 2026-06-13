# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -std=c++17 -fno-sized-deallocation
OPTFLAGS := -O3 -march=native -flto
DEBUGFLAGS := -g
DEPFLAGS := -MMD -MP

# Static linking flags
STATICFLAGS := -static -static-libgcc -static-libstdc++

# Source and object files
SRCS := engine.cpp perftValidate.cpp match.cpp
OBJS := $(SRCS:.cpp=.o)
DEPS := $(SRCS:.cpp=.d)

TARGETS := engine perftValidate match

TUNER_SRC := tuner.cpp
TUNER_OBJ := tuner.o
TUNER_DEP := tuner.d

# Default target
all: CXXFLAGS += $(OPTFLAGS)
all: LDFLAGS += $(STATICFLAGS)
all: $(TARGETS)

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: LDFLAGS += $(STATICFLAGS)
debug: $(TARGETS)

# Build targets
engine: engine.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

perftValidate: perftValidate.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

match: match.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -pthread -o $@ $^

tuner: CXXFLAGS += $(OPTFLAGS) -DTUNING_MODE
tuner: LDFLAGS += $(STATICFLAGS)
tuner: $(TUNER_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

# Compile .cpp into .o and generate .d files for header tracking
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Include dependency files if they exist
-include $(DEPS)
-include $(TUNER_DEP)

# Clean build artifacts
clean:
	rm -f *.o *.d $(TARGETS) tuner

.PHONY: all debug clean
