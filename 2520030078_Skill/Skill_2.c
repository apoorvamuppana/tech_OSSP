#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX_INPUT 100
/* Process the entered command */
void processCommand(char *command) {
    if (strcmp(command, "hello") == 0) {
        printf("Hello! Welcome to MyShell.\n");
    }
    else if (strcmp(command, "help") == 0) {
        printf("\nAvailable commands:\n");
        printf("  hello - Display greeting\n");
        printf("  help  - Display available commands\n");
        printf("  exit  - Exit the shell\n");
    }
    else if (strlen(command) == 0) {
        /* Do nothing for empty input */
    }
    else {
        printf("Unknown command: %s\n", command);
    }
}
int main() {
    char buffer[MAX_INPUT];

    while (1) {
        printf("myshell> ");
        fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin);
        // Remove newline
        buffer[strcspn(buffer, "\n")] = '\0';
        // Check exit condition
        if (strcmp(buffer, "exit") == 0) {
            printf("Exiting shell...\n");
            break;
        }
        else {
            processCommand(buffer);
            fflush(stdout);
        }
    }
    return 0;
}
