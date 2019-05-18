SRC_DIR := .
OBJ_DIR := bin
ENGINE_SRC_DIR := ..
#need to add a line here for every subfoler in GameEngine
GAMESTATE_SRC_DIR := ../GameState
COMMANDS_SRC_DIR := ../Commands

EXENAME=unit_tests

INCLUDE+=-I$(SRC_DIR)
INCLUDE+=-I$(ENGINE_SRC_DIR)
INCLUDE+=-I$(GAMESTATE_SRC_DIR)
INCLUDE+=-I$(COMMANDS_SRC_DIR)

CXX=g++
#these are the flags that I always want
CXXFLAGS+=-Wall -std=c++14 -fPIC

#grab all source files of interest
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
SRC_FILES += $(wildcard $(ENGINE_SRC_DIR)/*.cpp)
SRC_FILES += $(wildcard $(GAMESTATE_SRC_DIR)/*.cpp)
SRC_FILES += $(wildcard $(COMMANDS_SRC_DIR)/*.cpp)

#obj files is list of all source files (without their directories) with .o appended
#$(patsubst pattern,replacement,text) Finds whitespace-separated words in text that match pattern and replaces them with replacement
OBJ_FILES := $(patsubst $(COMMANDS_SRC_DIR)/%.cpp,%.o,$(SRC_FILES))
OBJ_FILES := $(patsubst $(GAMESTATE_SRC_DIR)/%.cpp,%.o,$(OBJ_FILES))
OBJ_FILES := $(patsubst $(ENGINE_SRC_DIR)/%.cpp,%.o,$(OBJ_FILES))
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,%.o,$(OBJ_FILES))

debug: CXXFLAGS+=-g
debug: all

all: $(OBJ_DIR) $(OBJ_DIR)/$(EXENAME) run

$(OBJ_DIR)/$(EXENAME): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@

$(OBJ_FILES): $(SRC_FILES)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $(SRC_FILES)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

run:
	./$(OBJ_DIR)/$(EXENAME) $(TEST)
	
clean:
	rm -f ./bin/*
	rm -f *.o
