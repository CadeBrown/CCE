# makefile - build system for cce (Cade's Chess Engine)
#
# Just run `make` to build `cce`, which is the UCI-compatible binary
#
# @author: Cade Brown <cade@cade.site>

# -*- Programs -*-

# C++ compiler
CXX          ?= c++

CXXFLAGS     += -std=c++11
LDFLAGS      += 

# debug
CXXFLAGS += -g


# -*- Files -*-

src_CC       := $(wildcard src/*.cc)
src_HH       := $(wildcard include/*.hh)



# -*- Output -*-

# Output binary which can be ran
cce_BIN      := cce

# -*- Generated -*-

src_O        := $(patsubst %.cc,%.o,$(src_CC))


# -*- Rules -*-

.PHONY: default clean FORCE

default: $(cce_BIN)

clean: FORCE
	rm -f $(wildcard $(src_O) $(cce_BIN))

FORCE: 

$(cce_BIN): $(src_O)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

%.o: %.cc $(src_HH)
	$(CXX) $(CXXFLAGS) -Iinclude -fPIC -c $< -o $@

