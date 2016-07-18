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
CPPFLAGS := -std=c++0x -Wall -Werror -fmessage-length=0

# Linker options
LIB := /usr/lib/x86_64-linux-gnu
LFLAGS := -lz -lzip -lboost_system -lboost_filesystem

# Source path
SRC = src/

# Release or debug, binary dir and specific comppile options
DEBUG ?= 0
ifeq ($(DEBUG),1)
	CPPFLAGS += -O0 -g3 -DDEBUG
	BIN := Debug/
else
	CPPFLAGS += -O3
	BIN := Release/
endif

# Dependency dir
DEPDIR := $(BIN).d
$(shell mkdir -p $(DEPDIR) >/dev/null)

# Dependency flags
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

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
$(BIN)%.o: $(SRC)%.cpp $(DEPDIR)/%.d
	@echo 'Compiling: $<'
	@$(CXX) $(DEPFLAGS) $(CPPFLAGS) -c $< -o $@
	@mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

# Dependencies dependencies
$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(CPPFILES)))

### Clean dependencies
clean:
	@echo Cleaning: objects, dependencies and executable
	@$(RM) $(BIN)*
