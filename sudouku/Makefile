CFLAGS = -g -fopenmp -Wall -Wextra -Wpedantic
LDFLAGS = -lm

SRCS = bruteForcesud.c helper_functions.c
TARGET = sudoku_solver

all: $(TARGET)

$(TARGET): $(SRCS)
	gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) txt.txt

