CC = g++
CFLAGS = -Wall
TARGETS = main hook.dll

.PHONY: all
all: $(TARGETS)

main: main.o hook.dll
	$(CC) $< -o $@ -L./ -lhook

hook.dll: hookdll.o
	$(CC) -shared $< -o $@

%.o: %.cc hook.h
	$(CC) $(CFLAGS) -c $< -std=c++11

.PHONY: clean
clean:
	rm *.o

