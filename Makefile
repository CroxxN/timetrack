CC = clang
CFLAGS=-Wall -Wpedantic -Wextra
LIBS=-lgit2

timetrack: timetrack.c
	$(CC) timetrack.c $(CFLAGS) $(LIBS) -ggdb -o timetrack

release: timetrack.c	
	$(CC) timetrack.c $(CFLAGS) $(LIBS) -o timetrack

run: timetrack
	./timetrack

clean: timetrack
	rm ./timetrack
