#ifndef SHELL_H
#define SHELL_H

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64
#define MAX_JOBS 64
#define MAX_HISTORY 100

// Main shell loop
void shell_loop(void);

// Signal handlers
void sigint_handler(int sig);
void sigtstp_handler(int sig);

// Global variables
extern pid_t g_fg_pgid;
extern char
    g_shell_home_dir[PATH_MAX]; // The directory where the shell was started

#endif // SHELL_H
