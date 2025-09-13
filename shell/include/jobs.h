#ifndef JOBS_H
#define JOBS_H

#include "parser.h"
#include "shell.h"

// Enum for job status
typedef enum { JOB_RUNNING, JOB_STOPPED, JOB_DONE } JobStatus;

// Struct to represent a job
typedef struct {
  pid_t pgid;
  int job_id;
  JobStatus status;
  char *command;
  int is_background;
} Job;

// Job table
extern Job job_table[MAX_JOBS];
extern int next_job_id;

// Job management functions
void init_jobs(void);
int add_job(pid_t pgid, const char *command, int is_background);
void remove_job(pid_t pgid);
Job *get_job_by_id(int job_id);
Job *get_job_by_pgid(pid_t pgid);
void update_job_status(pid_t pgid, int status);
void check_background_jobs(void);
void print_job_status(Job *job, int is_bg_completion);

// Execution functions
void execute_ast(ASTNode *node);

#endif // JOBS_H
