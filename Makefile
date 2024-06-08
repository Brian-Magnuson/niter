
# This Makefile is used to build the niterc compiler and run tests.

# LLVM libraries to link against
LLVM_LIBS = `llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native` -fexceptions


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
	rm -rf bin
	rm -rf build
	rm -rf test/bin
	rm -rf test/build
	rm -rf test/temp
	rm -rf sandbox/bin
	rm -rf sandbox/debug

# Recipes

# Builds the core source files
$(OBJ_DIR)/%.o: src/%.cpp $(CORE_HEADERS)
	@echo "\033[0;35mCompiling $<\033[0m"
	mkdir -p $(@D)
	clang++ -std=c++17 -O0 -Wall -g -c -o $@ $<

# Builds the niterc compiler
bin/niterc: $(CORE_OBJS) src/main.cpp
	@echo "\033[0;35mLinking $@\033[0m"
	mkdir -p bin
	clang++ -std=c++17 -O0 -Wall -g -o bin/niterc $(CORE_OBJS) src/main.cpp $(LLVM_LIBS)

# Builds the catch_amalgamated object file
test/build/catch/catch_amalgamated.o: test/catch/catch_amalgamated.cpp test/catch/catch_amalgamated.hpp
	@echo "\033[0;36mCompiling $<\033[0m"
	mkdir -p test/build/catch
	clang++ -std=c++17 -O0 -Wall -Wextra -g -c -o test/build/catch/catch_amalgamated.o test/catch/catch_amalgamated.cpp

# Builds the test source files
$(TEST_OBJ_DIR)/%.o: test/%.cpp
	@echo "\033[0;36mCompiling $<\033[0m"
	mkdir -p $(@D)
	clang++ -std=c++17 -O0 -Wall -g -c -o $@ $<

# Builds the test runner
test/bin/test: $(CORE_OBJS) $(TEST_OBJS) test/build/catch/catch_amalgamated.o
	@echo "\033[0;36mLinking $@\033[0m"
	mkdir -p test/bin
	clang++ -std=c++17 -O0 -Wall -g -o test/bin/test $(CORE_OBJS) $(TEST_OBJS) test/build/catch/catch_amalgamated.o $(LLVM_LIBS)
