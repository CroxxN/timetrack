#ifndef TIMETRACK_H_
#define TIMETRACK_H_

#include <stdio.h>
char* get_working_dir(void);
int load_config_w(char *);
int get_current_date(void);
int initialize(FILE *);

#endif
