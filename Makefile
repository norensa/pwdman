# 
#  Copyright (c) 2023 Noah Orensa.
#  Licensed under the MIT license. See LICENSE file in the project root for details.
# 

# module name
MODULE = pwdman

################################################################################

BUILD_DIR = build/$(shell uname -s)-$(shell uname -m)
LIB_DIR = lib/$(shell uname -s)-$(shell uname -m)
BIN_DIR = bin/$(shell uname -s)-$(shell uname -m)

INCLUDES = -Iinclude -Ilibspl/include

LIB_DIRS = -Llib/$(shell uname -s)-$(shell uname -m)

LIBS = -lspl -lcryptopp -lreadline
LIB_DEPEND = \
	libspl/lib/$(shell uname -s)-$(shell uname -m)/libspl.a

CXX = g++
CPPFLAGS = -Werror -Wall -Winline -Wpedantic
CXXFLAGS = -std=c++17 -march=native -fopenmp -pthread -fPIC

AR = ar
ARFLAGS = rc

LDFLAGS = -Wl,-E -Wl,-export-dynamic
DEPFLAGS = -MM

SOURCES = $(filter-out src/main.cpp, $(wildcard src/*.cpp))
OBJ_FILES = $(SOURCES:src/%.cpp=$(BUILD_DIR)/%.o)

.PHONY : all libspl test install uninstall clean clean-dep

all : pwdman

test : libspl $(OBJ_FILES)
	@$(MAKE) -C test --no-print-directory EXTRACXXFLAGS="$(EXTRACXXFLAGS)" nodep="$(nodep)"
	@./test/dtest/dtest-cxx17

test-build-only : libspl $(OBJ_FILES)
	@$(MAKE) -C test --no-print-directory EXTRACXXFLAGS="$(EXTRACXXFLAGS)" nodep="$(nodep)"

pwdman : $(BIN_DIR)/pwdman

libspl : | $(LIB_DIR)
	@$(MAKE) -C libspl --no-print-directory nodep="$(nodep)"
	@ln -f libspl/lib/$(shell uname -s)-$(shell uname -m)/libspl.a $(LIB_DIR)/libspl.a

install : pwdman
	@echo "CP        $(MODULE)"
	@cp $(BIN_DIR)/pwdman /usr/bin/pwdman

uninstall :
	@echo "RM        $(MODULE)"
	@rm -f /usr/bin/pwdman

ifndef nodep
include $(SOURCES:src/%.cpp=.dep/%.d)
else
ifneq ($(nodep), true)
include $(SOURCES:src/%.cpp=.dep/%.d)
endif
endif

# cleanup

clean :
	@rm -rf build lib
	@echo "Cleaned $(MODULE)/build/"
	@echo "Cleaned $(MODULE)/lib/"
	@$(MAKE) -C test --no-print-directory clean nodep="$(nodep)"
	@$(MAKE) -C libspl --no-print-directory clean nodep="$(nodep)"

clean-dep :
	@rm -rf .dep
	@echo "Cleaned $(MODULE)/.dep/"
	@$(MAKE) -C test --no-print-directory clean-dep nodep="$(nodep)"
	@$(MAKE) -C libspl --no-print-directory clean-dep nodep="$(nodep)"

# dirs

.dep $(BUILD_DIR) $(LIB_DIR) $(BIN_DIR) :
	@echo "MKDIR     $(MODULE)/$@/"
	@mkdir -p $@

# core

libspl/lib/$(shell uname -s)-$(shell uname -m)/libspl.a : libspl

$(BIN_DIR)/pwdman : $(BUILD_DIR)/main.o $(LIB_DEPEND) $(OBJ_FILES) | $(BIN_DIR)
	@echo "LD        $(MODULE)/$@"
	@$(CXX) $(CXXFLAGS) $(EXTRACXXFLAGS) $(OBJ_FILES) $(BUILD_DIR)/main.o $(LIB_DIRS) $(LIBS) -o $@

.dep/%.d : src/%.cpp | .dep
	@echo "DEP       $(MODULE)/$@"
	@set -e; rm -f $@; \
	$(CXX) $(DEPFLAGS) $(INCLUDES) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(BUILD_DIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(BUILD_DIR)/%.o : src/%.cpp $(LIB_DEPEND) | $(BUILD_DIR)
	@echo "CXX       $(MODULE)/$@"
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(EXTRACXXFLAGS) $(INCLUDES) $< $(LD_FLAGS) $(LIB_DIRS) $(LIBS) -o $@
