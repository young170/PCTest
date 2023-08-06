#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/wait.h>


char *build_file (char *filename);
int check_file (char *filename);
char *remove_file_ext(char *org);
char *compile (char *source_filename, char *output_filename);
