CC      := gcc
CFLAGS  := -Wall -O2 -g

SRC     := brainfuck.c
TARGET  := bin/brainfuck
TESTS   := $(wildcard tests/*.bf)

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^

test: $(TARGET)
	@echo "Running tests..."
	@for bf in $(TESTS); do \
		prog=$$(basename "$$bf" .bf); \
		asmb=./bin/$${prog}.s; \
		exec=./bin/$${prog}; \
		inpt=./tests/$${prog}.input; \
		expt=./tests/$${prog}.expected; \
		actu=./bin/$${prog}.actual; \
		./$(TARGET) $$bf -o $$asmb || exit 1; \
		$(CC) $(CFLAGS) $$asmb -o $$exec || exit 1; \
		if [ -f "$$inpt" ]; then \
			./$$exec < $$inpt > $$actu; \
		else \
			./$$exec > $$actu; \
		fi; \
		if diff -u $$expt $$actu; then \
			echo "[PASS] $$bf"; \
		else \
			echo "[FAIL] $$bf"; \
		fi; \
	done

clean:
	rm -rf bin/

.PHONY: all clean
