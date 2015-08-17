CC = g++
CFLAGS = -Wall -std=c++11 -mwindows
TARGETS = wintile

.PHONY: all
all: $(TARGETS)

wintile: wintile.o
	$(CC) $< -o $@

debug: wintile.cc wintile.h
	$(CC) $(CFLAGS) -S -g $<

release: wintile.cc wintile.h
	$(CC) $(CFLAGS) -O2 -s $< -o $(TARGETS)

%.o: %.cc %.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm *.o

