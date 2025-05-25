# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17

# Source and target
SRC := engine.cpp
HEADERS := board.h constants.h precalculated_move_tables.h random.h moves.h
OBJ := $(SRC:.cpp=.o)
TARGET := engine
TARGET_WIN := engine.exe

# Optimized build
all: CXXFLAGS += -O3 -march=native -flto
all: $(TARGET) $(TARGET_WIN)

# Debug build
debug: CXXFLAGS += -g
debug: $(TARGET) $(TARGET_WIN)

# Targets
$(TARGET): $(OBJ) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

$(TARGET_WIN): $(OBJ) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

# Clean
clean:
	rm -f *.o $(TARGET) $(TARGET_WIN)
