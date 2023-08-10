#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

    // timer
    sleep(2);

    // open new file descriptor
    // FILE *fp = fopen("foo.txt", "w");
    // int fd = fileno(fp);
    // printf("fileno: %d\n", fd);

    // remove file
    // remove("foo.txt");

    return 0;
}