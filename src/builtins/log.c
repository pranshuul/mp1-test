// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

void log_cmd(SimpleCommand *cmd) {
  if (cmd->args[1] == NULL) {
    fprintf(stderr, "log: expected message to log\n");
    return;
  }

  FILE *logfile = fopen(".shell_log", "a");
  if (!logfile) {
    perror("log: couldn't open .shell_log");
    return;
  }

  time_t now = time(NULL);
  char *time_str = ctime(&now);
  time_str[strlen(time_str) - 1] = '\0'; // Remove newline

  fprintf(logfile, "[%s] ", time_str);
  for (int i = 1; cmd->args[i] != NULL; i++) {
    fprintf(logfile, "%s ", cmd->args[i]);
  }
  fprintf(logfile, "\n");

  fclose(logfile);
  printf("Message logged to .shell_log\n");
}
