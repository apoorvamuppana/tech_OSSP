#include <stdio.h>
#include <string.h>
int main() {
    char command[100];
    while (1) {
        // Display prompt
        printf("myshell> ");
        fflush(stdout);
        // Read user input
        fgets(command, sizeof(command), stdin);
        // Remove newline
        command[strcspn(command, "\n")] = '\0';
        // Check exit condition
        if (strcmp(command, "exit") == 0) {
            printf("Exiting shell...\n");
            break;
        }
        // Process command
        printf("You entered: %s\n", command);
    }
    return 0;
}
