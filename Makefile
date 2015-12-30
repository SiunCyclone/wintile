CC = clang++
CFLAGS = -Wall -std=c++11
EXE = wintile.exe

.PHONY: all
all: $(EXE)

wintile.exe: wintile.o wndhook.dll
	$(CC) $(CFLAGS) -L./ -lwndhook $< -o $@ -mwindows

wintile.o: wintile.cc wintile.h wndhookdll.h
	$(CC) $(CFLAGS) -c $<

wndhook.dll: wndhookdll.o
	$(CC) $(CFLAGS) -shared $< -o $@

wndhookdll.o: wndhookdll.cc wndhookdll.h
	$(CC) $(CFLAGS) -c $<

debug: wintile.o wndhook.dll
	$(CC) $(CFLAGS) -S -g wintile.cc wndhookdll.cc
	$(CC) $(CFLAGS) -c -g wintile.cc wndhookdll.cc
	$(CC) $(CFLAGS) -shared wndhookdll.o -o wndhook.dll
	$(CC) $(CFLAGS) -L./ -lwndhook wintile.o -o $(EXE) -mwindows

release: wintile.o wndhook.dll
	$(CC) $(CFLAGS) -shared -s -DNDEBUG wndhookdll.o -o wndhook.dll
	$(CC) $(CFLAGS) -L./ -lwndhook -s -DNDEBUG wintile.cc -o $(EXE) -mwindows

.PHONY: clean
clean:
	rm *.o *.s

