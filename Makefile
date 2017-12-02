#============================================================================
# AirspaceConverter
# Since       : 14/6/2016
# Authors     : Alberto Realis-Luc <alberto.realisluc@gmail.com>
#               Valerio Messina <efa@iol.it>
# Web         : http://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2017 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This source file is part of AirspaceConverter project
#============================================================================

# Compiler options
CPPFLAGS = -std=c++0x -Wall -Werror -fmessage-length=0

# Linker options
LIB = /usr/lib/x86_64-linux-gnu
LFLAGS = -lzip -lboost_system -lboost_filesystem -lboost_locale

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
	Waypoint.cpp

# List of object files
OBJS = $(patsubst %.cpp, $(BIN)%.o, $(CPPFILES))

# Build all
all: $(BIN)airspaceconverter

# Build the command line program
$(BIN)airspaceconverter: $(BIN)libairspaceconverter.so $(SRC)main.cpp
	@echo Building executable: $@
	@g++ $(CPPFLAGS) -L$(BIN) $(SRC)main.cpp -lairspaceconverter $(LFLAGS) -o $@
ifeq ($(DEBUG),0)
	@strip -S --strip-unneeded $@
endif

# Build the shared library
$(BIN)libairspaceconverter.so: $(OBJS)
	@echo Building shared library: $@
	@g++ -L$(LIB) $(LFLAGS) -Wl,-soname,libairspaceconverter.so -shared $(OBJS) -o $@
ifeq ($(DEBUG),0)
	@strip -S --strip-unneeded $@
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
	@rm -rf $(BIN)*

# Install
.PHONY: install
install: $(BIN)airspaceconverter
	@echo Installing AirspaceConverter...
	@cp $< /usr/bin
	@cp $(BIN)libairspaceconverter.so /usr/lib
	@mkdir -p /usr/share/airspaceconverter/icons
	@cp icons/* /usr/share/airspaceconverter/icons
	@gzip -9 < airspaceconverter.1 > airspaceconverter.1.gz
	@mv airspaceconverter.1.gz /usr/share/man/man1/
	@echo Done.

# Uninstall
.PHONY: uninstall
uninstall:
	@echo Uninstalling AirspaceConverter...
	@rm -f /usr/bin/airspaceconverter
	@rm -f /usr/lib/libairspaceconverter.so
	@rm -rf /usr/share/airspaceconverter
	@rm -rf /usr/share/man/man1/airspaceconverter.1.gz
	@echo Done.
