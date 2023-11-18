build: *.c
	gcc *.c -o brainfuck

run_hello: build
	./brainfuck hello.bf

clean: 
	rm -rf brainfuck