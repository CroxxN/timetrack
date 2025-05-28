#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 100
#define FIFO_PATH "/tmp/timetrack"
#define FIFO_MODE 1337

struct Watchdog {
  // The file descriptor associated with the current inotify instance
  int inotify;

  // flags with which the inotify instance is created
  int flags;
};

// clean before exit
void clean(struct Watchdog *wd) { free(wd); }

int listen(int wd_fd) {
  char read_buf[BUF_SIZE];

  mkfifo(FIFO_PATH, FIFO_MODE);
  int fd;

  if ((fd = open(FIFO_PATH, O_RDONLY)) < 0) {
    return -1;
  }

  int n;

  while ((n = read(fd, read_buf, BUF_SIZE)) > 0) {
    // TODO: add the directory in `read_buf` to the file watch dog
  }
  return 0;
}

// using inotify
int initialize_watchdog(void) {

  // inotify
  int inotify_fd = inotify_init();

  if (inotify_fd == -1) {
    perror("Failed to create a inotify instance");
    return -1;
  }

  return inotify_fd;
}

// handle inotify events
int inotify_loop(void) {
  // todo
  return 0;
}

int main(void) {
  pid_t pid = fork();

  if (pid == -1) {
    perror("fork");
    return -1;
  } else if (0 == pid) {
    int wd_fd = initialize_watchdog();

    if (wd_fd == -1)
      return -1;

    listen(wd_fd);
  }
  return 0;
}
