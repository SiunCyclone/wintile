CC = g++
CFLAGS = -Wall
TARGETS = main

.PHONY: all
all: $(TARGETS)

main: main.o
	$(CC) $< -o $@

%.o: %.cc
	$(CC) $(CFLAGS) -c $< -std=c++11

.PHONY: clean
clean:
	rm *.o

