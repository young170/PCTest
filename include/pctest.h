#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


char *build_file (char *filename);
int check_file (char *filename);
char *remove_file_ext(char *org);
struct timespec *time_execution(struct timespec *timer);
char *compile (char *source_filename, char *output_filename);
int execute_programs(char *dirpath, char *solution_exe_file, char *target_exe_file, struct timespec **timings, int *correct_cnt, int *failed_cnt);
char *run_program (char *exe_file, char *input);
char *read_file_contents(char *filename);
int min_exe_time (struct timespec **timings);
int max_exe_time (struct timespec **timings);
struct timespec sum_exe_time (struct timespec **timings);