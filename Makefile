CC = gcc
CFLAGS = -Wall
TARGETS = main hook.dll

.PHONY: all
all: $(TARGETS)

main: main.o
	$(CC) $< -o $@

hook.dll: hookdll.o
	$(CC) -shared $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm *.o

