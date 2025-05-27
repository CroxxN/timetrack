#ifndef TIMETRACK_H_
#define TIMETRACK_H_

#include <stdio.h>

typedef struct {
  FILE *db;
} DB;

char *get_working_dir(void);
char *repo_name(char *);
int open_db(char *, DB *);
int initialize(FILE *, char *);
void diagnostics(void);
int get_current_date(void);

#endif
