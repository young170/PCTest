#include <stdio.h>
#include <stdlib.h>

int main (void) {
    int x;

    scanf("%d", &x);
    printf("%d\n", x);

    // runtime error
    // char *str = NULL;
    // str[0] = 'a';
    // str[1] = 'b';
    // str[2] = 'c';
    // str[3] = '\0';
    // printf("%s\n", str);

    return 0;
}