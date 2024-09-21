#include <git2/errors.h>
#include <git2/global.h>
#include <git2/repository.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <git2.h>

// WORKS:
char* get_working_dir(void){
    git_buf repo = {0};
    int res = git_repository_discover(&repo, ".", 0, NULL);
    if (res!=0){
        const git_error* ge = git_error_last();
        printf("git error: class: %d, Message: %s\n", ge->klass, ge->message);
        return NULL;
    }
    int new_repo_size = repo.size - 5;
    char* work_dir = realloc(repo.ptr, new_repo_size);
    work_dir[new_repo_size] = '\0';
    return work_dir;
}

// WORKS: TODO: seperate load for writing and reading? --yes
int load_config(char *work_dir){
    if (!work_dir){
        return -1;
    }

    FILE *f;
    f = fopen(strcat(work_dir, ".timetrack"), "a");
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "%s: Error: No commands passed\nUsage: %s <command> [arguments...]\n", argv[0], argv[0]);
        return 1;
    }
    // initialize libgit2
    git_libgit2_init();

    // Convert the list of commands to a single string---later passed on to the user's shell
    // Using this to properly handle shell alias
    char commands[255] = {0};
    for (int i=1; i<argc; i++){
        strcat(commands, argv[i]);
        strcat(commands, " ");
    }
    char *work_dir = get_working_dir();
    load_config(work_dir);

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
        // "-i" forces an interactive shell such the configs of the shell are loaded up
        execl(user_shell, user_shell, "-i", "-c", commands, (char *) NULL);

        // If execl fails
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);

        time_t end_time = time(NULL);
        printf("Execution time: %ld seconds\n", end_time - start_time);
    }

    // close libgit2
    git_libgit2_shutdown();

    return 0;
}
