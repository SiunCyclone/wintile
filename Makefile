CC = gcc
CFLAGS = -Wall
TARGETS = main hook.dll

.PHONY: all
all: $(TARGETS)

main: main.c
	$(CC) $(CFLAGS) -o $@ $<

hook.dll: hookdll.o
	$(CC) -shared -o $@ $<

hookdll.o: hookdll.c
	$(CC) $(CFLAGS) -c  $<

.PHONY: clean
clean:
	rm *.o

