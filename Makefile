CC = g++
CFLAGS = -Wall
TARGETS = wintile

.PHONY: all
all: $(TARGETS)

wintile: wintile.o
	$(CC) $< -o $@

%.o: %.cc
	$(CC) $(CFLAGS) -c $< -std=c++11

.PHONY: clean
clean:
	rm *.o

