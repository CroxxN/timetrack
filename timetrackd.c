#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 100
#define FIFO_PATH "/tmp/timetrack"
#define FIFO_MODE 0666

int handle_pipe_message(int fd, char *buf, int size) {
  unsigned char type = buf[0];
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
    int status = inotify_add_watch(fd, path_name, mask);
    return status;
  } else if (type == 0) {
    // TODO: implement removing
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

// TODO: handle inotify events
int inotify_loop(int fd) {
  // read(fd);
  return 0;
}

int main(void) {

  int wd_fd = initialize_watchdog(); // file_descriptor of the ipc pipe

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

  return 0;
}
