#include <stdio.h>
#include <time.h>

typedef struct {
  FILE *db;
} DB;

typedef enum { OPEN, CLOSEW, CLOSENW, DELETE } EVENT;

// TODO: create a db file and return it
int stamp_init(char *workdir) {

  // --------------------TEST------------------------
  FILE *f = fopen(workdir, "a+");
  fprintf(f, "2025\n"); // maybe date on the first line
  // ------------------------------------------------

  return 0;
}

int stamp_event(DB *dbase, EVENT evnt, char *file, struct tm time) {
  int error = 0;

  error = fprintf(dbase->db, "%s ", file);

  if (0 > error)
    return error;
  switch (evnt) {
  case OPEN:
    error = fprintf(dbase->db, "OPEN ");
    break;
  case CLOSEW:
    error = fprintf(dbase->db, "CLOSEW ");
    break;
  case CLOSENW:
    error = fprintf(dbase->db, "CLOSENW ");
    break;
  case DELETE:
    error = fprintf(dbase->db, "DELETE ");
    break;
  }
  if (0 > error)
    return -1;

  int year = time.tm_year + 1900;
  int month = time.tm_mon + 1;
  int day = time.tm_mday;

  int hour = time.tm_hour;
  int minute = time.tm_min;
  int seconds = time.tm_sec;

  error = fprintf(dbase->db, "%d-%02d-%02d %02d:%02d:%02d\n", year, month, day,
                  hour, minute, seconds);

  if (error < 0)
    return error;
  return 0;
}
