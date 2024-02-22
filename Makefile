
run: bin/niterc
	./bin/niterc

bin/niterc: src/*.cpp
	mkdir -p bin
	clang++ -std=c++17 -O3 -Wall -Wextra -g -o bin/niterc src/*.cpp

clean:
	rm -rf bin
