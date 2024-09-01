# -*- makefile-gmake -*-
#
# test/Makefile
#

TOPSRC := $(abspath ..)
OBJSUFFIX ?= .o
LIBSUFFIX ?= .a

CXX ?= g++
CXXSTD = -std=c++98
CXXFLAGS = -g -Wall -Wextra -DGTEST_LANG_CXX11=0 -pthread

ifeq ($(COVERAGE),1)
CXXFLAGS += -fprofile-arcs -ftest-coverage -O0
endif

ifeq (,$(findstring $(VERBOSE),true))
Q = @
endif

GTEST_DIR=googletest/googletest
GTEST_INCDIR=$(GTEST_DIR)/include
GMOCK_DIR=googletest/googlemock
GMOCK_INCDIR=$(GMOCK_DIR)/include

all: test

include Makefile.testlibs

.cpp$(OBJSUFFIX):
	$(CXX) $(CXXSTD) -isystem $(GTEST_INCDIR) -I$(GTEST_DIR) $(CXXFLAGS) -c $< -o $@

.cc$(OBJSUFFIX):
	$(CXX) $(CXXSTD) -isystem $(GTEST_INCDIR) -I$(GTEST_DIR) $(CXXFLAGS) -c $< -o $@

test_%$(EXESUFFIX): test_%.cpp $(TOPSRC)/%.cpp libgtest$(LIBSUFFIX)
	$(Q)$(CXX) $(CXXSTD) -isystem $(GTEST_INCDIR) -I$(TOPSRC) $(CXXFLAGS) $^ -o $@
	./$@

test_Encoding$(EXESUFFIX): test_Encoding.cpp $(TOPSRC)/Encoding.cpp $(TOPSRC)/sjis2utf16.cpp libgtest$(LIBSUFFIX)
	$(Q)$(CXX) $(CXXSTD) -isystem $(GTEST_INCDIR) -I$(TOPSRC) $(CXXFLAGS) $^ -o $@
	./$@

test_BaseReader$(EXESUFFIX): test_BaseReader.cpp $(TOPSRC)/BaseReader.h libgtest$(LIBSUFFIX)
	$(Q)$(CXX) $(CXXSTD) -isystem $(GTEST_INCDIR) -I$(TOPSRC) $(CXXFLAGS) $^ -o $@
	./$@

TESTEXE := test_Encoding$(EXESUFFIX) test_BaseReader$(EXESUFFIX)

test: $(TESTEXE)

#CLEAN_PKG += googletest

clean::
	$(RM) $(CLEAN_OBJ) $(TESTEXE)
	$(RM) -R $(CLEAN_PKG)

#distclean:: clean
#	$(RM) $(DISTCLEAN_PKG)

.PHONY: all clean test $(TESTEXE)

.DELETE_ON_ERROR:
