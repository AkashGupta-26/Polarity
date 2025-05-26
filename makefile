CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -fno-sized-deallocation
OPTFLAGS := -O3 -march=native -flto
DEBUGFLAGS := -g
DEPFLAGS := -MMD -MP

SRCS := engine.cpp perftValidate.cpp
OBJS := $(SRCS:.cpp=.o)
DEPS := $(SRCS:.cpp=.d)
TARGETS := engine perftValidate

all: CXXFLAGS += $(OPTFLAGS)
all: $(TARGETS)

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: $(TARGETS)

engine: engine.o
	$(CXX) $(CXXFLAGS) -o $@ $^

perftValidate: perftValidate.o
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -f *.o *.d $(TARGETS)

.PHONY: all debug clean
