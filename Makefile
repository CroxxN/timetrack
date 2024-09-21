CC = clang
CFLAGS=-Wall -Wpedantic -Wextra -std=c99

timetrack: timetrack.c
	$(CC) timetrack.c $(CFLAGS) -ggdb -o timetrack

release: timetrack.c	
	$(CC) timetrack.c $(CFLAGS)  -o timetrack

run: timetrack
	./timetrack

clean: timetrack
	rm ./timetrack
