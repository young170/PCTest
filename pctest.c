#include "pctest.h"


int main (int argc, char *argv[]) {
    int opt;
    int timeout;
    char *testdir;

    while ((opt = getopt(argc, argv, "i:t:")) != -1) {
        switch (opt) {
            case 'i':
                testdir = strdup(optarg);

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

    char *solution_output_filename = build_file(solution_filename);
    char *target_output_filename = build_file(target_filename);

    int result = execute_programs(testdir, solution_output_filename, target_output_filename);
    
    if (result == 1) {
        printf("WRONG\n");
    } else {
        printf("CORRECT\n");
    }

    return 0;
}

int execute_programs(char *dirpath, char *solution_exe_file, char *target_exe_file) {
    DIR *dir = opendir(dirpath);

    if (dir == NULL) {
        fprintf(stderr, "invalid testdir\n");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    struct stat st;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        int filepath_length = strlen(dirpath) + strlen(entry->d_name) + 2;
        char *filepath = (char *) malloc(sizeof(char) * filepath_length);
        snprintf(filepath, filepath_length, "%s/%s", dirpath, entry->d_name);

        stat(filepath, &st);
        
        if (S_ISREG(st.st_mode)) {

            char *file_contents = read_file_contents(filepath);

            char *solution_result = run_program(solution_exe_file, file_contents);
            char *target_result = run_program(target_exe_file, file_contents);
            
            if (strcmp(solution_result, target_result) != 0) {
                // different to the solution
                free(solution_result);
                free(target_result);

                return 1;
            }

            free(solution_result);
            free(target_result);
        }

        free(filepath);
    }

    return 0;
}

char *run_program (char *exe_file, char *input) {
    char *buf;

    int test_input_pipe[2];
    int runtime_pipe[2];

    if (pipe(test_input_pipe) == -1) {
        fprintf(stderr, "pipe failed\n");
        exit(EXIT_FAILURE);
    }

    if (pipe(runtime_pipe) == -1) {
        fprintf(stderr, "pipe failed\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "fork failed\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        close(runtime_pipe[0]);
        dup2(runtime_pipe[1], STDOUT_FILENO);

        close(test_input_pipe[1]);
        dup2(test_input_pipe[0], STDIN_FILENO);

        execv(exe_file, NULL);

        fprintf(stderr, "execv failed\n");
        exit(EXIT_FAILURE);
    } else {
        close(runtime_pipe[1]);
        close(test_input_pipe[0]);

        write(test_input_pipe[1], input, strlen(input));

        int status;
        waitpid(pid, &status, 0);

        if (!WIFEXITED(status)) {
            fprintf(stderr, "runtime error\n");
            exit(EXIT_FAILURE);
        }

        buf = (char *) malloc(sizeof(char) * BUFSIZ);
        int buf_len = read(runtime_pipe[0], buf, BUFSIZ);
        
        if (buf_len != 0) {
	        buf[buf_len] = '\0';
        }
    }

    return buf;
}

char *read_file_contents(char *filename) {
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        fprintf(stderr, "invalid file\n");
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0L, SEEK_END);
    long buf_size = ftell(fp);
    char *buf = (char *) malloc(sizeof(char) * (buf_size + 1));

    rewind(fp);

    int used = 0;
    int n = 0;
    while (1) {
        n = fread(buf + used, sizeof(char), buf_size, fp);

        if (n == 0) {
            break;
        }

        used += n;
    }

    buf[buf_size] = '\0';
    return buf;
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

    if (pipe(compilation_pipe) == -1) {
        fprintf(stderr, "pipe failed\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "fork failed\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        close(compilation_pipe[0]);
        dup2(compilation_pipe[1], STDERR_FILENO);

        if (execvp("gcc", argv) == -1) {
            fprintf(stderr, "compilation failed\n");
            exit(EXIT_FAILURE);
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
