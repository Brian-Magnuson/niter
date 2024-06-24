
run:
	cmake --build ../build
	mkdir -p bin
	mkdir -p debug
	../build/niterc src/main.nit -o bin/main -dump-ir debug/main.ll
	./bin/main
