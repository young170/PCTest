#include "pctest.h"


int check_file (char *filename) {
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}

char *remove_file_ext (char *org) {
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

char *read_file_contents (char *filename) {
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

    fclose(fp);

    buf[buf_size] = '\0';
    return buf;
}


int run_program (char *solution_exe_file, char *target_exe_file, char *input, int timeout) {
    char *solution_buf;
    char *target_buf;

    int solution_input_pipe[2];
    int solution_runtime_pipe[2];

    int target_input_pipe[2];
    int target_runtime_pipe[2];

    if (pipe(solution_input_pipe) == -1) {
        fprintf(stderr, "pipe failed\n");
        exit(EXIT_FAILURE);
    }

    if (pipe(solution_runtime_pipe) == -1) {
        fprintf(stderr, "pipe failed\n");
        exit(EXIT_FAILURE);
    }

    if (pipe(target_input_pipe) == -1) {
        fprintf(stderr, "pipe failed\n");
        exit(EXIT_FAILURE);
    }

    if (pipe(target_runtime_pipe) == -1) {
        fprintf(stderr, "pipe failed\n");
        exit(EXIT_FAILURE);
    }

    // execute the solution process
    pid_t solution_pid = fork();
    if (solution_pid < 0) {
        fprintf(stderr, "fork failed\n");
        exit(EXIT_FAILURE);
    } else if (solution_pid == 0) {
        close(solution_runtime_pipe[0]);
        dup2(solution_runtime_pipe[1], STDOUT_FILENO);

        close(solution_input_pipe[1]);
        dup2(solution_input_pipe[0], STDIN_FILENO);

        execv(solution_exe_file, NULL);

        fprintf(stderr, "execv failed\n");
        exit(EXIT_FAILURE);
    }

    // clock time
    struct timespec *timer = (struct timespec *) malloc(sizeof(struct timespec));

    if (clock_gettime(CLOCK_REALTIME, timer) == -1) {
        fprintf(stderr, "clock_gettime() failed\n");
        exit(EXIT_FAILURE);
    }
    
    time_t start_time = timer->tv_sec;

    // execute the target process
    pid_t target_pid = fork();
    if (target_pid < 0) {
        fprintf(stderr, "fork failed\n");
        exit(EXIT_FAILURE);
    } else if (target_pid == 0) {
        struct rlimit rl;

        rl.rlim_cur = 3;
        rl.rlim_max = 3;

        if (setrlimit(RLIMIT_NOFILE, &rl) != 0) {
            printf("setrlimit() failed with errno=%d\n", errno);
            exit(1);
        }

        close(target_runtime_pipe[0]);
        dup2(target_runtime_pipe[1], STDOUT_FILENO);

        close(target_input_pipe[1]);
        dup2(target_input_pipe[0], STDIN_FILENO);

        close(STDERR_FILENO);
        execv(target_exe_file, NULL);

        fprintf(stderr, "execv failed\n");
        exit(EXIT_FAILURE);
    }

    close(solution_runtime_pipe[1]);
    close(solution_input_pipe[0]);
    close(target_runtime_pipe[1]);
    close(target_input_pipe[0]);

    write(solution_input_pipe[1], input, strlen(input));
    write(target_input_pipe[1], input, strlen(input));

    int status;

    // if timeout, kill
    while (1) {
        int result = waitpid(target_pid, &status, WNOHANG);
        
        if (result == 0) {
            // Child process is still running
            if (clock_gettime(CLOCK_REALTIME, timer) == -1) {
                fprintf(stderr, "clock_gettime() failed\n");
                exit(EXIT_FAILURE);
            }

            time_t curr_time = timer->tv_sec - start_time;

            if (curr_time >= timeout) {
                kill(target_pid, SIGTERM);
                return 3;
            }
        } else {
            break;
        }
    }

    waitpid(solution_pid, &status, 0);

    solution_buf = (char *) malloc(sizeof(char) * BUFSIZ);
    int buf_len = read(solution_runtime_pipe[0], solution_buf, BUFSIZ);
    
    if (buf_len != 0) {
        solution_buf[buf_len] = '\0';
    }

    target_buf = (char *) malloc(sizeof(char) * BUFSIZ);
    buf_len = read(target_runtime_pipe[0], target_buf, BUFSIZ);
    
    if (buf_len != 0) {
        target_buf[buf_len] = '\0';
    }

    printf("sol: %s\n", solution_buf);
    printf("tar: %s\n", target_buf);

    if (strcmp(solution_buf, target_buf) == 0) {
        return 0;
    } else {
        return 1;
    }

    free(solution_buf);
    free(target_buf);
}

struct timespec *time_execution (struct timespec *timer) {
    struct timespec *tmp = (struct timespec *) malloc(sizeof(struct timespec));

    if (clock_gettime(CLOCK_REALTIME, tmp) == -1) {
        fprintf(stderr, "clock_gettime() failed\n");
        return NULL;
    }

    timer->tv_sec = tmp->tv_sec - timer->tv_sec;
    timer->tv_nsec = tmp->tv_nsec - timer->tv_nsec;

    free(tmp);
    return timer;
}

int execute_programs (char *dirpath, char *solution_exe_file, char *target_exe_file, struct timespec **timings, int *correct_cnt, int *failed_cnt, int timeout) {
    DIR *dir = opendir(dirpath);

    if (dir == NULL) {
        fprintf(stderr, "invalid testdir\n");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    struct stat st;

    int i = 0;
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

            timings = (struct timespec **) realloc(timings, (i + 1) * sizeof(struct timespec *));            
            timings[i] = (struct timespec *) malloc(sizeof(struct timespec));

            if (clock_gettime(CLOCK_REALTIME, timings[i]) == -1) {
                fprintf(stderr, "clock_gettime() failed\n");
                exit(EXIT_FAILURE);
            }

                int exe_result = run_program(solution_exe_file, target_exe_file, file_contents, timeout);

            timings[i] = time_execution(timings[i]);
printf("res: %d\n", exe_result);
            if (exe_result != 0) {
                (*failed_cnt)++;
            } else {
                (*correct_cnt)++;
            }
        
            i++;
            free(file_contents);
        }

        free(filepath);
    }

    return i;
}

int compile (char *source_filename, char *output_filename) {
    char *argv[] = {"gcc", "-o", output_filename, source_filename, NULL};

    int status;
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
            return 1;
        }
    } else {
        waitpid(pid, &status, 0);
        
        close(compilation_pipe[1]);

        char *buf = (char *) malloc(sizeof(char) * 1);
        int buf_len = read(compilation_pipe[0], buf, 1);

        if (buf_len != 0) {
	        return 1;
        }
    }

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
    int compile_result = compile(filename, output_filename);

    if (compile_result != 0) {
        fprintf(stderr, "Compile error\n");
        exit(EXIT_FAILURE);
    }

    return output_filename;
}

int max_exe_time (struct timespec **timings, int len) {
    int i = 0;
    int max_idx = -1;
    time_t max_sec = -1;
    long max_nsec = -1;

    while (i < len) {
        if ((max_sec <= timings[i]->tv_sec) || max_idx == -1) {
            if ((max_sec == timings[i]->tv_sec) && (timings[0]->tv_nsec < max_nsec)) {
                i++;
                continue;
            }

            max_sec = timings[i]->tv_sec;
            max_nsec = timings[i]->tv_nsec;
            max_idx = i;
        }

        i++;
    }

    return max_idx;
}

int min_exe_time (struct timespec **timings, int len) {
    int i = 0;
    int min_idx = -1;
    time_t min_sec = -1;
    long min_nsec = -1;

    while (i < len) {
        if ((timings[i]->tv_sec <= min_sec) || min_idx == -1) {
            if ((min_sec == timings[i]->tv_sec) && (min_nsec < timings[0]->tv_nsec)) {
                i++;
                continue;
            }

            min_sec = timings[i]->tv_sec;
            min_nsec = timings[i]->tv_nsec;
            min_idx = i;
        }

        i++;
    }

    return min_idx;
}

struct timespec sum_exe_time (struct timespec **timings, int len) {
    int i = 0;
    time_t sum_sec = 0;
    long sum_nsec = 0;

    while (i < len) {
        sum_sec += timings[i]->tv_sec;
        sum_nsec += timings[i]->tv_nsec;

        i++;
    }

    struct timespec sum_time = {sum_sec, sum_nsec};
    return sum_time;
}

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

    // summary necessary data
    struct timespec **timings = (struct timespec **) malloc(sizeof(struct timespec *));
    int correct_cnt = 0;
    int failed_cnt = 0;
    
    int time_length = execute_programs(testdir, solution_output_filename, target_output_filename, timings, &correct_cnt, &failed_cnt, timeout);

    int max_idx = max_exe_time(timings, time_length);
    int min_idx = min_exe_time(timings, time_length);
    struct timespec sum_timer = sum_exe_time(timings, time_length);
    
    printf("> PCTest Summary:\nCorrect count: %d, Failed count: %d\nMax time: %ld, Min time: %ld, Total time: %ld\n", correct_cnt, failed_cnt, timings[max_idx]->tv_sec, timings[min_idx]->tv_sec, sum_timer.tv_sec);

    int i = 0;
    while (i < time_length) {
        free(timings[i]);
        i++;
    }
    free(timings);

    free(solution_output_filename);
    free(target_output_filename);

    return 0;
}
