#============================================================================
# AirspaceConverter
# Since       : 14/6/2016
# Authors     : Alberto Realis-Luc <alberto.realisluc@gmail.com>
#               Valerio Messina <efa@iol.it>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2021 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This source file is part of AirspaceConverter project
#============================================================================

# Compiler options
CPPFLAGS = -std=c++11 -Wall -Werror -fmessage-length=0

# Product name
APPNAME = airspaceconverter

# Platform
PLATFORM=$(shell uname -s)

# Linker and strip options
LFLAGS = -lzip -lboost_system -lboost_filesystem '-Wl,-rpath,$$ORIGIN'
STRIP = -S

ifeq ($(PLATFORM),Linux)
	LIB = /usr/lib/x86_64-linux-gnu
	LIBFILE = lib$(APPNAME).so
	LFLAGS += -lboost_locale
	DYNLIBFLAGS = $(LFLAGS) -Wl,-soname,$(LIBFILE)
	STRIP += --strip-unneeded
	DEST = /usr/
else ifeq ($(PLATFORM),Darwin)
	LIB = /usr/local/lib
	LIBFILE = lib$(APPNAME).dylib
	LFLAGS += -lboost_locale-mt
	DEST = /usr/local/
	DYNLIBFLAGS = $(LFLAGS) -Wl,-install_name,$(DEST)lib/$(LIBFILE)
else
	#ERROR: Unknown platform!
endif

# Source path
SRC = src/

# Release or debug, binary dir and specific compile options
DEBUG ?= 0
ifeq ($(DEBUG),1)
	CPPFLAGS += -O0 -g3 -DDEBUG
	BIN = Debug/
else
	CPPFLAGS += -O3 -DNDEBUG
	BIN = Release/
endif
$(shell mkdir -p $(BIN) >/dev/null)

# Dependencies dir
DEPDIR = $(BIN).d/
$(shell mkdir -p $(DEPDIR) >/dev/null)

# Dependencies flags
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)$*.Td

# List of C++ source files
CPPFILES =                \
	Airfield.cpp          \
	Airspace.cpp          \
	AirspaceConverter.cpp \
	SeeYou.cpp            \
	Geometry.cpp          \
	KML.cpp               \
	OpenAIP.cpp           \
	OpenAir.cpp           \
	Polish.cpp            \
	RasterMap.cpp         \
	Waypoint.cpp          \
	CSV.cpp               \
	IGC.cpp

# List of object files
OBJS = $(patsubst %.cpp, $(BIN)%.o, $(CPPFILES))

# Build all
all: $(BIN)$(APPNAME)

# Build the command line program
$(BIN)$(APPNAME): $(BIN)$(LIBFILE) $(SRC)main.cpp
	@echo Building executable: $@
	@g++ $(CPPFLAGS) -L$(BIN) $(SRC)main.cpp -l$(APPNAME) $(LFLAGS) -o $@
ifeq ($(DEBUG),0)
	@strip $(STRIP) $@
endif

# Build the shared library
$(BIN)$(LIBFILE): $(OBJS)
	@echo Building shared library: $@
	@g++ -L$(LIB) $(DYNLIBFLAGS) -shared $(OBJS) -o $@
ifeq ($(DEBUG),0)
	@strip $(STRIP) $@
endif
	@chmod a-x $@

# Compile the sources of common shared library
$(BIN)%.o: $(SRC)%.cpp
$(BIN)%.o: $(SRC)%.cpp $(DEPDIR)%.d
	@echo 'Compiling: $<'
	@g++ $(DEPFLAGS) $(CPPFLAGS) -fPIC -c $< -o $@
	@mv -f $(DEPDIR)$*.Td $(DEPDIR)$*.d

# Compile dependencies
$(DEPDIR)%.d: ;
.PRECIOUS: $(DEPDIR)%.d

-include $(patsubst %,$(DEPDIR)%.d,$(basename $(CPPFILES)))

# Clean
.PHONY: clean
clean:
	@echo Cleaning all
	@rm -rf $(BIN)

# Install
.PHONY: install
install: $(BIN)airspaceconverter
	@echo Installing AirspaceConverter CLI ...
	@cp $< $(DEST)bin
	@cp $(BIN)$(LIBFILE) $(DEST)lib
	@mkdir -p $(DEST)share/airspaceconverter/icons
	@cp icons/* $(DEST)share/airspaceconverter/icons
	@gzip -9 < airspaceconverter.1 > airspaceconverter.1.gz
	@mv airspaceconverter.1.gz $(DEST)share/man/man1/
	@echo Done.

# Uninstall
.PHONY: uninstall
uninstall:
	@echo Uninstalling AirspaceConverter CLI ...
	@rm -f $(DEST)bin/airspaceconverter
	@rm -f $(DEST)lib/$(LIBFILE)
	@rm -rf $(DEST)share/airspaceconverter
	@rm -rf $(DEST)share/man/man1/airspaceconverter.1.gz
	@echo Done.
