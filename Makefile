CC = clang
CFLAGS=-Wall -Wpedantic -Wextra
LIBS=-lgit2

timetrack: ./src/timetrack.c
	$(CC) ./src/timetrack.c $(CFLAGS) $(LIBS) -ggdb -o timetrack

release: ./src/timetrack.c	
	$(CC) ./src/timetrack.c $(CFLAGS) $(LIBS) -o timetrack

daemon: ./src/timetrackd.c
	$(CC) ./src/timetrackd.c $(CFLAGS)  -o timetrackd

run: timetrack
	./timetrack

clean: timetrack
	rm ./timetrack ./timetrackd
