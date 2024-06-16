
# This Makefile is used to build the niterc compiler and run tests.

# Compiler and compiler options
CC = clang++-18
CC_OPTIONS = -std=c++17 -O0 -Wall -g
# Do not set -c or -o here; these are set per recipe

# LLVM libraries to link against
LLVM_INCLUDE = -I`llvm-config-18 --includedir`
LLVM_LIBS = `llvm-config-18 --cxxflags --ldflags --system-libs --libs core orcjit native` -fexceptions


# The core source files, i.e. files of the form src/*/*.cpp
OBJ_DIR = build
CORE_SRCS = $(wildcard src/*/*.cpp)
CORE_OBJS = $(patsubst src/%.cpp, $(OBJ_DIR)/%.o, $(CORE_SRCS))

# The core header files, i.e. files of the form src/*/*.h
CORE_HEADERS = $(wildcard src/*/*.h)
# If any header file changes, rebuild all source files

# The test source files, i.e. files of the form test/*.cpp
TEST_OBJ_DIR = test/build
TEST_SRCS = $(wildcard test/*.cpp)
TEST_OBJS = $(patsubst test/%.cpp, $(TEST_OBJ_DIR)/%.o, $(TEST_SRCS))

# Main rules

# The default rule builds the niterc compiler
default: bin/niterc

# The test rule builds the test runner and runs the tests
test: test/bin/test
	test/bin/test

# The clean rule removes all build artifacts
clean:
	rm -rf bin build test/bin test/build test/temp sandbox/bin sandbox/debug

# Recipes

# Builds the core source files
$(OBJ_DIR)/%.o: src/%.cpp $(CORE_HEADERS)
	@echo "\033[0;35mCompiling $<\033[0m"
	mkdir -p $(@D)
	$(CC) $(CC_OPTIONS) $(LLVM_INCLUDE) -c -o $@ $<

# Builds the niterc compiler
bin/niterc: $(CORE_OBJS) src/main.cpp
	@echo "\033[0;35mLinking $@\033[0m"
	mkdir -p bin
	$(CC) $(CC_OPTIONS) $(LLVM_INCLUDE) -o bin/niterc $(CORE_OBJS) src/main.cpp $(LLVM_LIBS)
	@echo "\033[0;35mFinished building $@\033[0m"

# Builds the catch_amalgamated object file
test/build/catch/catch_amalgamated.o: test/catch/catch_amalgamated.cpp test/catch/catch_amalgamated.hpp
	@echo "\033[0;36mCompiling $<\033[0m"
	mkdir -p test/build/catch
	$(CC) $(CC_OPTIONS) -c -o test/build/catch/catch_amalgamated.o test/catch/catch_amalgamated.cpp

# Builds the test source files
$(TEST_OBJ_DIR)/%.o: test/%.cpp $(CORE_HEADERS)
	@echo "\033[0;36mCompiling $<\033[0m"
	mkdir -p $(@D)
	$(CC) $(CC_OPTIONS) $(LLVM_INCLUDE) -c -o $@ $<

# Builds the test runner
test/bin/test: $(CORE_OBJS) $(TEST_OBJS) test/build/catch/catch_amalgamated.o
	@echo "\033[0;36mLinking $@\033[0m"
	mkdir -p test/bin
	$(CC) $(CC_OPTIONS) $(LLVM_INCLUDE) -o test/bin/test $(CORE_OBJS) $(TEST_OBJS) test/build/catch/catch_amalgamated.o $(LLVM_LIBS)
	@echo "\033[0;36mFinished building $@\033[0m"
