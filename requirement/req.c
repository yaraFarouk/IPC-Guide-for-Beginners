// cmpsh.c – A mini shell with pipe support
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_CMDS 10
#define MAX_ARGS 20
#define MAX_LINE 1024

// Helper to trim whitespace
char* trim(char* str) {
    while (*str == ' ') str++;
    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\n')) *end-- = '\0';
    return str;
}

// Helper to split command into args
void parse_args(char* cmd, char* args[]) {
    int i = 0;
    char* token = strtok(cmd, " ");
    while (token && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

int main() {
    char input[MAX_LINE];

    while (1) {
        printf("cmpsh> ");
        if (!fgets(input, sizeof(input), stdin)) break;

        // Handle exit
        if (strncmp(input, "exit", 4) == 0) break;

        // Split on pipes
        char* cmds[MAX_CMDS];
        int num_cmds = 0;
        char* token = strtok(input, "|");

        while (token && num_cmds < MAX_CMDS) {
            cmds[num_cmds++] = trim(token);
            token = strtok(NULL, "|");
        }

        int prev_fd = -1;
        int pipefd[2];

        for (int i = 0; i < num_cmds; ++i) {
            if (i < num_cmds - 1 && pipe(pipefd) == -1) {
                perror("pipe");
                exit(1);
            }

            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(1);
            } else if (pid == 0) {
                // Redirect input if not the first command
                if (prev_fd != -1) {
                    dup2(prev_fd, STDIN_FILENO);
                    close(prev_fd);
                }

                // Redirect output if not the last command
                if (i < num_cmds - 1) {
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[0]);
                    close(pipefd[1]);
                }

                // Parse and exec
                char* args[MAX_ARGS];
                parse_args(cmds[i], args);
                execvp(args[0], args);
                perror("execvp");
                exit(1);
            }

            // Parent process closes write end
            if (prev_fd != -1) close(prev_fd);
            if (i < num_cmds - 1) {
                close(pipefd[1]);
                prev_fd = pipefd[0];
            }
        }

        // Wait for all children
        for (int i = 0; i < num_cmds; ++i) wait(NULL);
    }

    return 0;
}
