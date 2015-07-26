CC = g++
CFLAGS = -Wall -std=c++11
TARGETS = wintile

.PHONY: all
all: $(TARGETS)

wintile: wintile.o
	$(CC) $< -o $@

%.o: %.cc
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm *.o

