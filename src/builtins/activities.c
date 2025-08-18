// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

void activities(SimpleCommand *cmd) {
  for (int i = 0; i < g_job_count; i++) {
    char status_str[20];
    switch (g_jobs[i].status) {
    case RUNNING:
      strcpy(status_str, "Running");
      break;
    case STOPPED:
      strcpy(status_str, "Stopped");
      break;
    case DONE:
      strcpy(status_str, "Done");
      break;
    }
    printf("[%d] %s (pgid: %d)\t%s\n", i + 1, status_str, g_jobs[i].pgid,
           g_jobs[i].command);
  }
}
