#include <stdio.h>
#include <stdlib.h>

int main() {
    int n;
    int *p;

    printf("Enter the size of dynamic memory: ");
    scanf("%d", &n);

    if (n <= 0) {
        printf("Invalid size.\n");
        return 1;
    }

    p = (int *)malloc(n * sizeof(int));

    if (p == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        printf("%d element: ", i + 1);
        scanf("%d", p + i);
    }

    printf("\nElements before reallocation:\n");

    for (int i = 0; i < n; i++) {
        printf("%d element is: %d\n", i + 1, *(p + i));
    }

    printf("\nMemory after increasing the size:\n");

    p = (int *)realloc(p, (n + 2) * sizeof(int));

    if (p == NULL) {
        printf("Memory reallocation failed.\n");
        return 1;
    }

    p[n] = 5;
    p[n + 1] = 10;

    for (int i = 0; i < n + 2; i++) {
        printf("%d element is: %d\n", i + 1, *(p + i));
    }

    if (n >= 3) {
        printf("\nMemory after decreasing the size:\n");

        p = (int *)realloc(p, (n - 3) * sizeof(int));

        if (p == NULL && n - 3 > 0) {
            printf("Memory reallocation failed.\n");
            return 1;
        }

        for (int i = 0; i < n - 3; i++) {
            printf("%d element is: %d\n", i + 1, *(p + i));
        }
    } else {
        printf("\nMemory cannot be decreased by 3 elements because the initial size is less than 3.\n");
    }

    free(p);

    printf("\nMemory released successfully.\n");

    return 0;
}
