
run:
	cd .. && make bin/niterc
	mkdir -p bin
	mkdir -p debug
	../bin/niterc src/main.nit -o bin/main -dump-ir debug/main.ll
	./bin/main
