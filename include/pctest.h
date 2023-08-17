#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


char *build_file (char *filename);
int execute_programs (char *dirpath, char *solution_exe_file, char *target_exe_file, struct timespec **timings, int *correct_cnt, int *failed_cnt, int timeout);
int run_program (char *solution_exe_file, char *target_exe_file, char *input, int timeout);
int max_exe_time (struct timespec **timings, int len);
int min_exe_time (struct timespec **timings, int len);
struct timespec sum_exe_time (struct timespec **timings, int len);