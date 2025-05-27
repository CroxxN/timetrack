#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 100
#define FIFO_PATH "/tmp/timetrack"
#define FIFO_MODE 1337

int listen(void) {
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
void watchdog(void) {
  // TODO: implement
}

int main(void) {
  pid_t pid = fork();

  if (pid == -1) {
    perror("fork");
    return -1;
  } else if (0 == pid)
    listen();
  return 0;
}
