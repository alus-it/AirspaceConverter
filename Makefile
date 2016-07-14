#============================================================================
# AirspaceConverter
# Since       : 14/6/2016
# Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
# Web         : http://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This source file is part of AirspaceConverter project
#============================================================================

# AirspaceConverter version string
VERSION := 0.0.0.9

# Tools
CXX := g++
RM := rm -rf

# Compiler options
CPPFLAGS := -std=c++0x -Wall -Werror -fmessage-length=0 -MMD -MP

# Linker options
LIB := /usr/lib/x86_64-linux-gnu
LFLAGS := -lz -lzip -lboost_system -lboost_filesystem

# Source paths
SRC = src/

# Determine if it is release or debug
DEBUG ?= 0
ifeq ($(DEBUG),1)
	CPPFLAGS += -O0 -g3 -DDEBUG
	BIN := Debug/
else
	CPPFLAGS += -O3
	BIN := Release/
endif

# List of C++ source files
CPPFILES =              \
	Airspace.cpp          \
	AirspaceConverter.cpp \
	KMLwriter.cpp         \
	main.cpp              \
	OpenAIPreader.cpp     \
	OpenAirReader.cpp     \
	PFMwriter.cpp         \
	RasterMap.cpp

# List of object files
OBJS = $(patsubst %.cpp, $(BIN)%.o, $(CPPFILES))

### Build dependencies
all: $(BIN)AirspaceConverter

# Link
$(BIN)AirspaceConverter: $(OBJS)
	@echo Linking: $@
	@$(CXX) -L$(LIB) $(LFLAGS) $(OBJS) -o $@

# Create output directory if missing
$(OBJS): | $(BIN)
$(BIN):
	@mkdir -p $(BIN)

# Compile
$(BIN)%.o: $(SRC)%.cpp
	@echo 'Compiling: $<'
	@$(CXX) $(CPPFLAGS) -c $< -o $@

### Clean dependencies
clean:
	@echo Cleaning: objects and  executable
	@$(RM) $(BIN)*

