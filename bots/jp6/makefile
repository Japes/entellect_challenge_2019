SRC_DIR := cppbot
OBJ_DIR := bin
ENGINE_SRC_DIR := ../../GameEngine
#need to add a line here for every subfoler in GameEngine
GAMESTATE_SRC_DIR := $(ENGINE_SRC_DIR)/GameState
COMMANDS_SRC_DIR := $(ENGINE_SRC_DIR)/Commands
RNG_SRC_DIR := ../../pcg-cpp-0.98/include
UTILS_SRC_DIR := ../../Utilities

EXENAME=cppbot.exe

INCLUDE+=-I$(SRC_DIR)
INCLUDE+=-I$(ENGINE_SRC_DIR)
INCLUDE+=-I$(GAMESTATE_SRC_DIR)
INCLUDE+=-I$(COMMANDS_SRC_DIR)
INCLUDE+=-I$(RNG_SRC_DIR)
INCLUDE+=-I$(UTILS_SRC_DIR)

CXX=g++
#these are the flags that I always want
CXXFLAGS+=-Wall -std=c++14 -fPIC -O2 -pthread

#grab all source files of interest
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
SRC_FILES += $(wildcard $(ENGINE_SRC_DIR)/*.cpp)
SRC_FILES += $(wildcard $(GAMESTATE_SRC_DIR)/*.cpp)
SRC_FILES += $(wildcard $(UTILS_SRC_DIR)/*.cpp)
SRC_FILES += $(wildcard $(COMMANDS_SRC_DIR)/*.cpp)

#obj files is list of all source files (without their directories) with .o appended
#$(patsubst pattern,replacement,text) Finds whitespace-separated words in text that match pattern and replaces them with replacement
OBJ_FILES := $(patsubst %.cpp,%.o,$(SRC_FILES))
HEADER_DEPS := $(patsubst %.cpp,%.d,$(SRC_FILES))

all: $(OBJ_DIR) $(OBJ_DIR)/$(EXENAME)

debug: CXXFLAGS+=-g
debug: all

$(OBJ_DIR)/$(EXENAME): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -c -o $@ $<

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

#include dependency stuff generated by the -MMD and -MP flags
-include $(HEADER_DEPS)
	
clean:
	rm -f ./bin/*
	rm -f $(OBJ_FILES)
	rm -f $(HEADER_DEPS)
