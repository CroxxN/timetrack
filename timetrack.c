#include "timetrack.h"
#include <git2.h>
#include <git2/buffer.h>
#include <git2/errors.h>
#include <git2/global.h>
#include <git2/repository.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

char *get_working_dir(void) {
  git_buf repo = {0};
  int status = git_repository_discover(&repo, ".", 0, NULL);

  // check for errors
  if (status != 0) {
    const git_error *ge = git_error_last();
    printf("git error: class: %d, Message: %s\n", ge->klass, ge->message);
    return NULL;
  }

  int new_repo_size = repo.size - 5;

  if (new_repo_size < 1) {
    git_buf_dispose(&repo);
    return NULL;
  }

  // 12 extra bytes for ".timetrackdb", 1 more for the null terminator
  char *work_dir = malloc(new_repo_size + 1);
  strncpy(work_dir, repo.ptr, new_repo_size);
  work_dir[new_repo_size] = '\0';
  return work_dir;
}

char *get_repo_name(char *workspace) {
  char *name = basename(workspace);
  return name;
}

int open_db(char *work_dir, DB *database) {
  if (!work_dir) {
    return -1;
  }

  int work_dir_len = strlen(work_dir);
  char *db_path = malloc(work_dir_len + 12 + 1);
  strcpy(db_path, work_dir);
  strcpy(db_path + work_dir_len, ".timetrackdb");
  db_path[work_dir_len + 12] = '\0';

  char *repo_name = get_repo_name(work_dir);

  FILE *f = fopen(db_path, "a+");
  database->db = f;

  fseek(f, 0, SEEK_END);

  int size = ftell(f);

  if (size == 0) {
    // if the database if empty, initialize it with the name of the git repo
    int status = initialize(f, repo_name);
    if (status)
      return -1;
  }

  return 0;
}

int initialize(FILE *file, char *repo_name) {
  if (repo_name == NULL) {
    printf("Failed to discover git workspace");
    return -1;
  }
  fprintf(file, "%s\n", repo_name);
  return 0;
}

int stamp_db(DB *dbase, char *process_name, struct tm start_time,
             struct tm end_time) {
  if (NULL == process_name)
    return -1;

  int error;

  error = fprintf(dbase->db, "%s ", process_name);
  if (error < 0)
    return -1;

  int st_year = start_time.tm_year + 1900;
  int st_month = start_time.tm_mon + 1;
  int st_day = start_time.tm_mday;

  int st_hour = start_time.tm_hour;
  int st_minute = start_time.tm_min;
  int st_seconds = start_time.tm_sec;

  error = fprintf(dbase->db, "%d-%02d-%02d %02d:%02d:%02d ", st_year, st_month,
                  st_day, st_hour, st_minute, st_seconds);

  if (error < 0)
    return error;

  int end_year = end_time.tm_year + 1900;
  int end_month = end_time.tm_mon + 1;
  int end_day = end_time.tm_mday;

  int end_hour = end_time.tm_hour;
  int end_minute = end_time.tm_min;
  int end_seconds = end_time.tm_sec;

  error = fprintf(dbase->db, "%d-%02d-%02d %02d:%02d:%02d\n", end_year,
                  end_month, end_day, end_hour, end_minute, end_seconds);
  if (error < 0)
    return error;

  return 0;
}

// TODO: implement diagnostics
void diagnostics(void) {}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(
        stderr,
        "%s: Error: No commands passed\nUsage: %s <command> [arguments...]\n",
        argv[0], argv[0]);
    return 1;
  }
  // initialize libgit2
  git_libgit2_init();

  // Convert the list of commands to a single string---later passed on to the
  // user's shell. Using this to properly handle shell alias
  char commands[255] = {0};
  for (int i = 1; i < argc; i++) {
    strcat(commands, argv[i]);
    strcat(commands, " ");
  }

  char *work_dir = get_working_dir();
  if (work_dir == NULL) {
    printf("Invalid Working Directory");
    return -1;
  }
  DB database;
  // database.db = malloc(sizeof(FILE *));

  if (open_db(work_dir, &database)) {
    printf("Error Opening the Working Diretory");
    return -1;
  }

  // add current directory to the watch dog if `timetrack init`
  if (!strcmp(argv[1], "init")) {
    printf("Initialized\n");
    // TODO: implement
    return 0;
  }

  // Record start time
  time_t start_time = time(NULL);

  pid_t pid = fork();

  if (pid == -1) {
    perror("fork");
    return -1;
  }

  if (pid == 0) {
    // get the user's shell; makes this program shell agnostic
    char *user_shell = getenv("SHELL");

    // "-i" forces an interactive shell such that configs of the shell are
    // loaded
    execl(user_shell, user_shell, "-i", "-c", commands, (char *)NULL);

    // If execl fails
    perror("execl");
    // exit(EXIT_FAILURE);
    return -1;
  }

  int status;
  waitpid(pid, &status, 0);

  time_t end_time = time(NULL);

  // to get the year, month, day, hour, minutes and seconds from `time_t`
  struct tm local_start_tm = *localtime(&start_time);
  struct tm local_end_tm = *localtime(&end_time);

  stamp_db(&database, argv[1], local_start_tm, local_end_tm);

  printf("Execution time: %ld seconds\n", end_time - start_time);

  // close the filestream
  fclose(database.db);

  // close libgit2
  git_libgit2_shutdown();

  return 0;
}
