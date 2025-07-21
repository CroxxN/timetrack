#include <asm-generic/errno.h>
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

// Global Logs
FILE *TIMETRACK_LOG = NULL;

// LOGGING:
typedef enum { LOG_INFO, LOG_WARN, LOG_ERROR } LOG_TYPE;
// NOTE: We should probably be careful about whether `s` is null-terminated or
// not.
void logging(LOG_TYPE ltype, char *s, ...) {
  if (NULL == TIMETRACK_LOG)
    return;

  time_t ts = time(NULL);
  struct tm *tsinfo = localtime(&ts);
  char *proper_time = asctime(tsinfo);

  int size = strlen(s);
  char log_buffer[size];

  va_list v;
  va_start(v, s);
  int status = vsnprintf(log_buffer, 100, s, v);
  va_end(v);

  if (0 > status)
    fprintf(TIMETRACK_LOG, "FAILED TO LOG PREVIOUS MESSAGE\n");

  switch (ltype) {
  case LOG_INFO:
    fprintf(TIMETRACK_LOG, "%s [INFO]: %s\n", proper_time, log_buffer);
    break;
  case LOG_WARN:
    fprintf(TIMETRACK_LOG, "%s [WARN]: %s\n", proper_time, log_buffer);
    break;
  case LOG_ERROR:
    fprintf(TIMETRACK_LOG, "%s [ERROR]: %s\n", proper_time, log_buffer);
    break;
  }
  return;
}

int handle_pipe_message(int fd, char *buf, int size) {
  unsigned char type = buf[0];
  if (type < 0 || type > 1)
    return -1;
  char path_size_c[4];

  memcpy(&path_size_c, buf + 1, sizeof(int));
  int path_size = atoi(path_size_c);
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
    if (EDEADLK == pthread_mutex_lock(&tex)) {
      logging(LOG_ERROR, "Deadlock detected. Operation failed.");
      return -1;
    }
    int status = hashmap_insert(map, wd, path_name);
    pthread_mutex_unlock(&tex);
    if (-1 == status)
      return -1;
    logging(LOG_ERROR, "Added path '%s' to watchdog", path_name);
  } else if (type == 0) {
    // TODO: implement removing
    // int status = inotify_rm_watch(int fd, int wd);
    logging(LOG_ERROR, "Removed path '%s' from watchdog", path_name);
  } else {
    logging(LOG_ERROR, "Invalid action type: %d", type);
    return -1;
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
    logging(LOG_INFO, "%s", read_buf);
    if (-1 == handle_pipe_message(wd_fd, read_buf, n)) {
      char *log_str = "Invalid Pipe Message. No Command Executed.";
      logging(LOG_ERROR, log_str);
    }
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

// TODO: implement performing actions on the received events
int watchdog_act(struct inotify_event *event, char *path) {
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
  // IN_CLOSE_NOWRITE: This event is triggered when a file opened for any OTHER
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
    int wd = ievnt.wd;
    pthread_mutex_lock(&tex);
    char *pathname = hashmap_get(map, wd);
    pthread_mutex_unlock(&tex);

    int status = watchdog_act(&ievnt, pathname);

    if (0 > status)
      logging(LOG_ERROR, "Watchdog didn't act. Status: %d", status);
  }
  return 0;
}

int main(void) {

  TIMETRACK_LOG = fopen(".TIMETRACKLOG", "a");

  int wd_fd = initialize_watchdog(); // file_descriptor of the ipc pipe
  map = hashmap();

  pthread_mutexattr_t tex_attr;
  pthread_mutexattr_init(&tex_attr);

  if (pthread_mutexattr_settype(&tex_attr, PTHREAD_MUTEX_ERRORCHECK)) {
    logging(LOG_ERROR, "Failed to initialize mutex attribute. Quitting.");
    return -1;
  }

  if (pthread_mutex_init(&tex, &tex_attr)) {
    logging(LOG_ERROR, "Failed to initialize mutex. Quitting.");
    return -1;
  }

  pthread_mutexattr_destroy(&tex_attr);

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

  pid_t inotify_pid = fork();
  if (-1 == inotify_init()) {
    perror("fork");
    return -1;
  } else if (0 == inotify_pid) {
    inotify_loop(wd_fd);
    pthread_mutex_destroy(&tex);
  }

  return 0;
}
