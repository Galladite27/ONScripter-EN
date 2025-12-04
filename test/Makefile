# -*- makefile-gmake -*-
#
# test/Makefile
#

TOPSRC := $(abspath ..)
OBJSUFFIX ?= .o
LIBSUFFIX ?= .a

CXX ?= g++
CXXSTD = -std=c++98
CXXFLAGS = -g -Wall -Wextra -DGTEST_LANG_CXX11=0 $(DEFS) -pthread

ifeq ($(COVERAGE),1)
CXXFLAGS += -fprofile-arcs -ftest-coverage -O0
endif

ifeq (,$(findstring $(VERBOSE),true))
Q = @
endif

BZIP2_CPPFLAGS=

ifneq (,$(wildcard $(TOPSRC)/extlib/lib/libbz2$(LIBSUFFIX)))
	BZIP2_CPPFLAGS=-I$(TOPSRC)/extlib/include
	LIBS_bz2=$(TOPSRC)/extlib/lib/libbz2$(LIBSUFFIX)
else
	BZIP2_CPPFLAGS=
	LIBS_bz2=$(shell pkg-config --libs bzip2 || echo -lbz2)
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

test_BaseReader$(EXESUFFIX): test_BaseReader.cpp libgtest$(LIBSUFFIX)
	$(Q)$(CXX) $(CXXSTD) -isystem $(GTEST_INCDIR) -I$(TOPSRC) $(CXXFLAGS) $^ -o $@
	./$@

test_DirPaths$(EXESUFFIX): test_DirPaths.cpp $(TOPSRC)/DirPaths.cpp libgtest$(LIBSUFFIX)
	$(Q)$(CXX) $(CXXSTD) -isystem $(GTEST_INCDIR) -I$(TOPSRC) $(CXXFLAGS) $^ -o $@
	./$@

test_DirectReader$(EXESUFFIX): test_DirectReader.cpp $(TOPSRC)/DirectReader.cpp libgtest$(LIBSUFFIX) libgmock$(LIBSUFFIX)
	$(Q)$(CXX) $(CXXSTD) -isystem $(GTEST_INCDIR) -isystem $(GMOCK_INCDIR) -I$(TOPSRC) $(CXXFLAGS) $(BZIP2_CPPFLAGS) $^ $(LIBS_bz2) -o $@
	./$@

test_ShiftJISData$(EXESUFFIX): test_ShiftJISData.cpp $(TOPSRC)/ShiftJISData.cpp libgtest$(LIBSUFFIX) libgmock$(LIBSUFFIX)
	$(Q)$(CXX) $(CXXSTD) -isystem $(GTEST_INCDIR) -isystem $(GMOCK_INCDIR) -I$(TOPSRC) $(CXXFLAGS) $^ -o $@
	./$@

TESTEXE := test_Encoding$(EXESUFFIX) test_BaseReader$(EXESUFFIX) test_DirPaths$(EXESUFFIX) test_DirectReader$(EXESUFFIX) test_ShiftJISData$(EXESUFFIX)

test: $(TESTEXE)

#CLEAN_PKG += googletest

clean::
	$(RM) $(CLEAN_OBJ) $(TESTEXE)
	$(RM) -R $(CLEAN_PKG)

#distclean:: clean
#	$(RM) $(DISTCLEAN_PKG)

.PHONY: all clean test $(TESTEXE)

.DELETE_ON_ERROR:
