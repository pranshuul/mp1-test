// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

void ping(SimpleCommand *cmd) {
  if (cmd->args[1] == NULL || cmd->args[2] == NULL) {
    fprintf(stderr, "Usage: ping <job_id> <signal_number>\n");
    return;
  }
  int job_id = atoi(cmd->args[1]);
  int signal_num = atoi(cmd->args[2]);

  Job *job = get_job_by_id(job_id);
  if (!job) {
    fprintf(stderr, "ping: job not found: %d\n", job_id);
    return;
  }

  // Use killpg to send signal to the entire process group
  if (killpg(job->pgid, signal_num) < 0) {
    perror("ping: killpg failed");
  } else {
    printf("Sent signal %d to job %d (pgid %d)\n", signal_num, job_id,
           job->pgid);
  }
}
