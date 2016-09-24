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

# Tools
CXX = g++
RM = rm -rf

# Compiler options
CPPFLAGS = -std=c++0x -Wall -Werror -fmessage-length=0

# Linker options
LIB = /usr/lib/x86_64-linux-gnu
LFLAGS = -lz -lzip -lboost_system -lboost_filesystem

# Source path
SRC = src/

# Release or debug, binary dir and specific comppile options
DEBUG ?= 0
ifeq ($(DEBUG),1)
	CPPFLAGS += -O0 -g3 -DDEBUG
	BIN = Debug/
else
	CPPFLAGS += -O3 -DNDEBUG
	BIN = Release/
endif
$(shell mkdir -p $(BIN) >/dev/null)

# Option to avoid using libzip, just for compiling anyaway in case of older zip.h
NOZIP ?= 0
ifeq ($(NOZIP),1)
	CPPFLAGS += -DNOZIP
endif

# Dependencies dir
DEPDIR = $(BIN).d/
$(shell mkdir -p $(DEPDIR) >/dev/null)

# Dependencies flags
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)$*.Td

# List of C++ source files
CPPFILES =              \
	Airspace.cpp          \
	AirspaceConverter.cpp \
	Geometry.cpp          \
	KMLwriter.cpp         \
	main.cpp              \
	OpenAIPreader.cpp     \
	OpenAir.cpp           \
	PFMwriter.cpp         \
	RasterMap.cpp

# List of object files
OBJS = $(patsubst %.cpp, $(BIN)%.o, $(CPPFILES))

# Build all
all: $(BIN)AirspaceConverter

# Link
$(BIN)AirspaceConverter: $(OBJS)
	@echo Linking: $@
	@$(CXX) -L$(LIB) $(LFLAGS) $(OBJS) -o $@

# Compile
$(BIN)%.o: $(SRC)%.cpp
$(BIN)%.o: $(SRC)%.cpp $(DEPDIR)%.d
	@echo 'Compiling: $<'
	@$(CXX) $(DEPFLAGS) $(CPPFLAGS) -c $< -o $@
	@mv -f $(DEPDIR)$*.Td $(DEPDIR)$*.d

# Compile dependencies
$(DEPDIR)%.d: ;
.PRECIOUS: $(DEPDIR)%.d

-include $(patsubst %,$(DEPDIR)%.d,$(basename $(CPPFILES)))

# Clean
clean:
	@echo Cleaning all
	@$(RM) $(BIN)*

