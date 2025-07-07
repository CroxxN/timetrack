#include <fcntl.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "hashmap.h"

#define BUF_SIZE 100
#define FIFO_PATH "/tmp/timetrack"
#define FIFO_MODE 0666

struct Table *map;
// global mutex lock
pthread_mutex_t tex;

// TODO: implement
void logging(char *s, ...) {
  va_list v;
  va_start(v, s);
}

int handle_pipe_message(int fd, char *buf, int size) {
  unsigned char type = buf[0];
  if (type < 0 || type > 1)
    return -1;
  int path_size;

  memcpy(&path_size, buf + 1, sizeof(int));

  buf += 5; // now at the start of the path name

  char *path_name = malloc(path_size + 1);

  if (NULL == path_name)
    return -1;

  strncpy(path_name, buf, path_size);
  path_name[path_size] = '\0';

  // TODO: add masks
  uint32_t mask = 0x0;

  if (type == 1) {
    int wd = inotify_add_watch(fd, path_name, mask);
    // TODO: add error checking
    pthread_mutex_lock(&tex);
    int status = hashmap_insert(map, wd, path_name);
    pthread_mutex_unlock(&tex);
    if (-1 == status)
      return -1;
  } else if (type == 0) {
    // TODO: implement removing
    // int status = inotify_rm_watch(int fd, int wd);
  }
  return 0;
}

int listen(int wd_fd) {
  char read_buf[BUF_SIZE];

  mkfifo(FIFO_PATH, FIFO_MODE);
  int fd;

  if ((fd = open(FIFO_PATH, O_RDONLY)) < 0) {
    return -1;
  }

  int n;

  // read is blocking so this waits until there is data to read
  while ((n = read(fd, read_buf, BUF_SIZE)) > 0) {
    handle_pipe_message(wd_fd, read_buf, n);
  }
  return 0;
}

// using inotify
int initialize_watchdog(void) {
  int inotify_fd = inotify_init(); // inotify

  if (inotify_fd == -1) {
    perror("Failed to create a inotify instance");
    return -1;
  }

  return inotify_fd;
}

// TODO: Implement
int watchdog_act(struct inotify_event *event) {
  // --------
  // There are three inotify events are fundamental to timetrack
  // IN_OPEN: This event is triggered when a file is opened, whether for
  // reading, writing or executing is not specified;
  // --------
  // We use two other events
  // types with IN_OPEN to determine if the user actually edited the file and if
  // so, for how long
  // --------
  // IN_CLOSE_WRITE: This event is triggered when a file opened
  // for writing is close. Paired with IN_OPEN, this event type will be used to
  // filter files opened for writing from files that were not for files returned
  // by IN_OPEN;
  // --------
  // IN_CLOSE_NOWRITE: This even is triggered when a file opened for any OTHER
  // purpose than writing is closed. This event is used to filter files returned
  // by IN_OPEN that we can ignore.
  // --------
  // Other events types that are, though as a supplement, still used are:
  // IN_CREATE
  // IN_DELETE
  // IN_MODIFY
  // --------

  return 0;
}

int inotify_loop(int fd) {
  struct inotify_event ievnt;
  // we allocate an additional 100 bytes because the size of
  // each inotify event is the size of the struct + the size of the name
  // field
  // See more: https://man7.org/linux/man-pages/man7/inotify.7.html

  int event_size = sizeof(struct inotify_event) + 100;

  int n;

  while ((n = read(fd, &ievnt, sizeof(struct inotify_event))) > 0) {
    // TODO: implement performing actions on the received events
    int wd = ievnt.wd;
    pthread_mutex_lock(&tex);
    char *pathname = hashmap_get(map, wd);
    pthread_mutex_unlock(&tex);

    int status = watchdog_act(&ievnt);

    // TODO: add logging
    if (0 > status)
      continue;
  }
  return 0;
}

int main(void) {

  int wd_fd = initialize_watchdog(); // file_descriptor of the ipc pipe
  map = hashmap();
  pthread_mutex_init(&tex, NULL);

  if (-1 == wd_fd)
    return -1;

  pid_t wd_pid = fork();
  if (-1 == wd_pid) {
    perror("fork");
    return -1;

  } else if (0 == wd_pid) {
    int status = listen(wd_fd);
    return status;
  }

  inotify_loop(wd_fd);

  pthread_mutex_destroy(&tex);

  return 0;
}
