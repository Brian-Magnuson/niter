
run: bin/niterc
	./bin/niterc

test: test/bin/test
	./test/bin/test

# Recipes

bin/niterc: src/*.cpp src/*/*.cpp
	mkdir -p bin
	clang++ -std=c++17 -O3 -Wall -Wextra -g -o bin/niterc src/*.cpp $(SOURCE_FILES)

test/bin/test: src/*/*.cpp test/*.cpp test/bin/catch_amalgamated.o
	mkdir -p test/bin
	clang++ -std=c++17 -O3 -Wall -Wextra -g -o test/bin/test src/*/*.cpp test/*.cpp test/bin/catch_amalgamated.o

test/bin/catch_amalgamated.o: test/catch/catch_amalgamated.cpp test/catch/catch_amalgamated.hpp
	mkdir -p test/bin
	clang++ -std=c++17 -O3 -Wall -Wextra -g -c -o test/bin/catch_amalgamated.o test/catch/catch_amalgamated.cpp

clean:
	rm -rf bin
	rm -rf test/bin
