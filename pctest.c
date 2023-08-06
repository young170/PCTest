#include "pctest.h"


int main (int argc, char *argv[]) {
    int opt;
    int timeout;
    DIR *testdir;

    while ((opt = getopt(argc, argv, "i:t:")) != -1) {
        switch (opt) {
            case 'i':
                testdir = opendir(optarg);

                if (testdir == NULL) {
                    fprintf(stderr, "invalid directory path [-i testdir]\n");
                    exit(EXIT_FAILURE);
                }

                break;
            case 't':
                timeout = atoi(optarg);

                if (timeout < 1 || timeout > 10) {
                    fprintf(stderr, "invalid value [-t timeout]\n");
                    exit(EXIT_FAILURE);
                }

                break;
            default:
                fprintf(stderr, "Usage: %s [-i testdir] [-t timeout] <solution> <target>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!(optind == (argc - 2))) { // if extra argc is not 2
        fprintf(stderr, "Usage: %s [-i testdir] [-t timeout] <solution> <target>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *solution_filename = argv[optind];
    char *target_filename = argv[optind + 1];

    char *solution_output = build_file(solution_filename);
    char *target_output = build_file(target_filename);
    
    return 0;
}

char *build_file (char *filename) {
    // check file is valid
    if (check_file(filename) == 1) {
        fprintf(stderr, "invalid file path <solution> <target>\n");
        exit(EXIT_FAILURE);
    }

    // remove file extension in filename
    char *output_filename = remove_file_ext(filename);
    
    if (output_filename == NULL) {
        exit(EXIT_FAILURE);
    }

    // compile given program source codes
    char *compile_result = compile(filename, output_filename);

    if (compile_result != NULL) {
        fprintf(stderr, "%s\n", compile_result);
        exit(EXIT_FAILURE);
    }

    return output_filename;
}

int check_file (char *filename) {
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}

char *remove_file_ext(char *org) {
    int concat_pos = 0;

    while (org[concat_pos] != '\0') {
        if (org[concat_pos] == '.') {
            break;
        }

        concat_pos++;
    }

    char *result = (char *)malloc(concat_pos + 1);
    
    if (result == NULL) {
        fprintf(stderr, "malloc failed\n");
        return NULL;
    }

    strncpy(result, org, concat_pos);
    result[concat_pos] = '\0';

    return result;
}

char *compile (char *source_filename, char *output_filename) {
    char *argv[] = {"gcc", "-o", output_filename, source_filename, NULL};

    int compilation_pipe[2];

    if (pipe(compilation_pipe) != 0) {
        // fprintf(stderr, "pipe failed\n");
        // exit(EXIT_FAILURE);
        return "pipe failed\n";
    }

    pid_t pid = fork();

    if (pid < 0) {
        // fprintf(stderr, "fork failed\n");
        // exit(EXIT_FAILURE);
        return "fork failed\n";
    } else if (pid == 0) {
        close(compilation_pipe[0]);
        dup2(compilation_pipe[1], STDERR_FILENO);

        if (execvp("gcc", argv) == -1) {
            // fprintf(stderr, "compilation failed\n");
            // exit(EXIT_FAILURE);
            return "compilation failed\n";
        }
    } else {
        wait(0x0);

        close(compilation_pipe[1]);

        char *buf = (char *) malloc(sizeof(char) * BUFSIZ);
        int buf_len = read(compilation_pipe[0], buf, BUFSIZ);

        if (buf_len != 0) {
	        buf[buf_len] = '\0';
            return buf;
        }
    }

    return NULL;
}
