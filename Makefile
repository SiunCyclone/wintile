CC = clang++
CFLAGS = -Wall -std=c++11
EXE = wintile.exe

.PHONY: all
all: $(EXE)

wintile.exe: wintile.o
	$(CC) $< -o $@ -mwindows

wintile.o: wintile.cc wintile.h
	$(CC) $(CFLAGS) -c $<

debug: wintile.cc wintile.h
	$(CC) $(CFLAGS) -S -g $<
	$(CC) $(CFLAGS) -c -g $<

release: wintile.cc wintile.h
	$(CC) $(CFLAGS) -O2 -s $< -o $(EXE)

.PHONY: clean
clean:
	rm *.o *.s

