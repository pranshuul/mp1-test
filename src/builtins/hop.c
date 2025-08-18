// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

void hop(SimpleCommand *cmd) {
  char *target_dir;
  char current_dir[MAX_PATH_LEN];
  getcwd(current_dir, sizeof(current_dir));

  char **args = cmd->args;
  if (args[1] == NULL || strcmp(args[1], "~") == 0) {
    target_dir = g_shell_home_dir;
  } else if (strcmp(args[1], "-") == 0) {
    if (g_prev_dir == NULL) {
      fprintf(stderr, "hop: OLDPWD not set\n");
      return;
    }
    target_dir = g_prev_dir;
    printf("%s\n", target_dir);
  } else {
    target_dir = args[1];
  }

  if (chdir(target_dir) != 0) {
    perror("hop");
  } else {
    free(g_prev_dir);
    g_prev_dir = strdup(current_dir);
  }
}
