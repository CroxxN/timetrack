#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int check_repo(){
    int res = system("git rev-parse --is-inside-work-tree");
    return res;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "%s: Error: No commands passed\nUsage: %s <command> [arguments...]\n", argv[0], argv[0]);
        return 1;
    }
    // Convert the list of commands to a single string---later passed on to the user's shell
    // Using this to properly handle shell alias
    char commands[255] = {0};
    for (int i=1; i<argc; i++){
        strcat(commands, argv[i]);
        strcat(commands, " ");
    }
    printf("%d\n", check_repo());

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

        if (WIFEXITED(status)) {
            printf("Command executed successfully with exit status %d\n", WEXITSTATUS(status));
        } else {
            printf("Command failed to execute\n");
        }

        printf("Execution time: %ld seconds\n", end_time - start_time);
    }

    return 0;
}
