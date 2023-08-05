#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

int main (int argc, char *argv[]) {
    int opt;
    int timeout;
    DIR *testdir;

    while ((opt = getopt(argc, argv, "i:t:")) != 1) {
        switch (opt) {
            case 'i':
                testdir = opendir(optarg);

                if (testdir == NULL) {
                    fprintf(stderr, "pctest: [-i testdir] invalid directory path\n");
                    exit(EXIT_FAILURE);
                }

                break;
            case 't':
                timeout = atoi(optarg);

                if (timeout < 1 || timeout > 10 || timeout == NULL) {
                    fprintf(stderr, "pctest: [-t timeout] invalid value\n");
                    exit(EXIT_FAILURE);
                }

                break;
            default:
                fprintf(stderr, "Usage: %s [-i testdir] [-t timeout] <solution> <target>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!(optind == (argc - 2))) {
        fprintf(stderr, "Usage: %s [-i testdir] [-t timeout] <solution> <target>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *solution_file = argv[optind];
    FILE *target_file = argv[optind + 1];

    if (solution_file == NULL || target_file == NULL) {
        fprintf(stderr, "pctest: <solution> <target> invalid file path\n");
        exit(EXIT_FAILURE);
    }

    // build given program source codes
    
    return 0;
}
