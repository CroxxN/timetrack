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
  char *work_dir = malloc(new_repo_size + 12 + 1);
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

  FILE *f = fopen(strcat(work_dir, ".timetrackdb"), "a+");
  database->db = f;

  fseek(f, 0, SEEK_END);

  int size = ftell(f);

  if (size == 0) {
    // if the database if empty, initialize it with the name of the git repo
    int status = initialize(f, work_dir);
    if (status)
      return -1;
  }

  return 0;
}

int initialize(FILE *file, char *workspace) {
  if (workspace == NULL) {
    printf("Failed to discover git workspace");
    return -1;
  }
  char *repo = get_repo_name(workspace);
  fprintf(file, "%s\n", repo);
  return 0;
}

int stamp_db(DB *dbase, long start_time, long end_time) {
  return fprintf(dbase->db, "%ld %ld\n", start_time, end_time);
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

  // Record start time
  time_t start_time = time(NULL);

  pid_t pid = fork();

  if (pid == -1) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    // get the user's shell; makes this program shell agnostic
    char *user_shell = getenv("SHELL");

    // "-i" forces an interactive shell such the configs of the shell are loaded
    // up
    execl(user_shell, user_shell, "-i", "-c", commands, (char *)NULL);

    // If execl fails
    perror("execl");
    // exit(EXIT_FAILURE);
    return -1;
  }

  int status;
  waitpid(pid, &status, 0);

  time_t end_time = time(NULL);
  stamp_db(&database, start_time, end_time);

  printf("Execution time: %ld seconds\n", end_time - start_time);

  fclose(database.db);

  // close libgit2
  git_libgit2_shutdown();

  return 0;
}
