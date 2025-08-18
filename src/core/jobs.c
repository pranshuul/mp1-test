// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

void init_jobs() {
  for (int i = 0; i < MAX_JOBS; i++) {
    g_jobs[i].pgid = 0;
  }
}

Job *add_job(pid_t pgid, const char *line, int is_background) {
  if (g_job_count >= MAX_JOBS) {
    fprintf(stderr, "shell: too many jobs\n");
    return NULL;
  }
  g_jobs[g_job_count].pgid = pgid;
  g_jobs[g_job_count].command = strdup(line);
  g_jobs[g_job_count].status = RUNNING;
  tcgetattr(STDIN_FILENO, &g_jobs[g_job_count].tmodes);
  g_job_count++;
  return &g_jobs[g_job_count - 1];
}

void remove_job_by_pgid(pid_t pgid) {
  for (int i = 0; i < g_job_count; i++) {
    if (g_jobs[i].pgid == pgid) {
      free(g_jobs[i].command);
      for (int j = i; j < g_job_count - 1; j++) {
        g_jobs[j] = g_jobs[j + 1];
      }
      g_job_count--;
      return;
    }
  }
}

void update_job_status(pid_t pgid, JobStatus status) {
  for (int i = 0; i < g_job_count; i++) {
    if (g_jobs[i].pgid == pgid) {
      g_jobs[i].status = status;
      return;
    }
  }
}

Job *get_job_by_id(int id) {
  if (id > 0 && id <= g_job_count) {
    return &g_jobs[id - 1];
  }
  return NULL;
}

Job *get_job_by_pgid(pid_t pgid) {
  for (int i = 0; i < g_job_count; i++) {
    if (g_jobs[i].pgid == pgid) {
      return &g_jobs[i];
    }
  }
  return NULL;
}

void reap_children(int signal) {
  int status;
  pid_t pid;
  while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
    Job *job = get_job_by_pgid(pid);
    if (job) { // Only manage PIDs that are process group leaders
      if (WIFEXITED(status) || WIFSIGNALED(status)) {
        if (job->status != DONE) {
          printf("[%d] Done\t%s\n", g_job_count, job->command);
        }
        remove_job_by_pgid(pid);
      } else if (WIFSTOPPED(status)) {
        update_job_status(pid, STOPPED);
      } else if (WIFCONTINUED(status)) {
        update_job_status(pid, RUNNING);
      }
    }
  }
}
