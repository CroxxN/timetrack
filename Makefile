CC = clang
CFLAGS=-Wall -Wpedantic -Wextra
LIBS=-lgit2

timetrack: ./src/timetrack.c
	$(CC) ./src/timetrack.c $(CFLAGS) $(LIBS) -ggdb -o timetrack

release: ./src/timetrack.c	
	$(CC) ./src/timetrack.c $(CFLAGS) $(LIBS) -o timetrack

daemon: ./src/timetrackd.c
	$(CC) ./src/timetrackd.c $(CFLAGS)  -o timetrackd

# TODO: one test for all; use cli to indicate what to test
test: ./tests/hashmap.c
	$(CC) ./tests/hashmap.c -o ./tests/hashmap && ./tests/hashmap

run: timetrack
	./timetrack

clean: timetrack
	rm ./timetrack ./timetrackd
