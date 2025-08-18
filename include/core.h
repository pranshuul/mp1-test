// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#ifndef H_INCLUDE_CORE
#define H_INCLUDE_CORE

// Standard Libraries
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

// Project Headers
#include "./builtins.h"
#include "./utils.h"

// --- Constants ---
#define MAX_ARGS 64
#define MAX_JOBS 100
#define MAX_ALIASES 100
#define MAX_PATH_LEN 1024
#define MAX_LINE_LEN 2048
#define HISTORY_CAPACITY 50

// --- AST (Abstract Syntax Tree) Structures ---

// Represents an 'atomic' command (e.g., ls -l > out.txt)
typedef struct SimpleCommand {
  char *args[MAX_ARGS];
  char *inputFile;
  char *outputFile;
  int append;
} SimpleCommand;

// Represents a 'cmd_group' (pipeline of atomic commands)
typedef struct Pipeline {
  SimpleCommand **commands;
  int num_commands;
  int background;
  char *full_text; // Store original text for job display
} Pipeline;

// --- Enums and Other Structs ---
typedef enum { RUNNING, STOPPED, DONE } JobStatus;
typedef struct {
  char *name;
  char *command;
} Alias;
typedef struct {
  pid_t pgid;
  char *command;
  JobStatus status;
  struct termios tmodes;
} Job;

// --- Global Variables ---
extern Alias g_aliases[MAX_ALIASES];
extern int g_alias_count;
extern Job g_jobs[MAX_JOBS];
extern int g_job_count;
extern char *g_shell_home_dir;
extern char *g_prev_dir;
extern char *g_history[HISTORY_CAPACITY];
extern int g_history_count;

// --- Core Function Prototypes ---
// shell.c
void launch_pipeline(Pipeline *p);

// prompt.c
char *get_prompt(void);

// parser.c
Pipeline **parse_line(const char *line, int *pipeline_count);
void free_simple_command(SimpleCommand *sc);
void free_pipeline(Pipeline *p);

// jobs.c
void init_jobs(void);
Job *add_job(pid_t pgid, const char *line, int is_background);
void remove_job_by_pgid(pid_t pgid);
void update_job_status(pid_t pgid, JobStatus status);
Job *get_job_by_id(int id);
Job *get_job_by_pgid(pid_t pgid);
void reap_children(int signal);

#endif // H_INCLUDE_CORE
