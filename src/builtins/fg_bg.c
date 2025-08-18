// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

extern pid_t g_shell_pgid;
extern struct termios g_shell_termios;

void fg(SimpleCommand *cmd) {
  if (cmd->args[1] == NULL) {
    fprintf(stderr, "fg: usage: fg <job_id>\n");
    return;
  }
  int job_id = atoi(cmd->args[1]);
  Job *job = get_job_by_id(job_id);

  if (!job) {
    fprintf(stderr, "fg: job not found: %d\n", job_id);
    return;
  }

  // Give terminal control to the job's process group
  tcsetpgrp(STDIN_FILENO, job->pgid);
  tcsetattr(STDIN_FILENO, TCSADRAIN, &job->tmodes);

  // Send SIGCONT to the process group to resume it
  if (killpg(job->pgid, SIGCONT) < 0) {
    perror("kill (SIGCONT)");
    return;
  }

  int status;
  waitpid(job->pgid, &status, WUNTRACED);

  if (WIFSTOPPED(status)) {
    fprintf(stderr, "\nStopped: %s\n", job->command);
    job->status = STOPPED;
    tcgetattr(STDIN_FILENO, &job->tmodes); // Save its terminal state
  } else {
    remove_job_by_pgid(job->pgid);
  }

  // Give terminal control back to the shell
  tcsetpgrp(STDIN_FILENO, g_shell_pgid);
  tcsetattr(STDIN_FILENO, TCSADRAIN, &g_shell_termios);
}

void bg(SimpleCommand *cmd) {
  if (cmd->args[1] == NULL) {
    fprintf(stderr, "bg: usage: bg <job_id>\n");
    return;
  }
  int job_id = atoi(cmd->args[1]);
  Job *job = get_job_by_id(job_id);

  if (!job) {
    fprintf(stderr, "bg: job not found: %d\n", job_id);
    return;
  }

  if (job->status == RUNNING) {
    fprintf(stderr, "bg: job %d already in background\n", job_id);
    return;
  }

  // Send SIGCONT to the process group
  if (killpg(job->pgid, SIGCONT) < 0) {
    perror("kill (SIGCONT)");
    return;
  }

  job->status = RUNNING;
  printf("[%d] %s &\n", job_id, job->command);
}
