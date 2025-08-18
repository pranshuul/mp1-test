// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

void alias_cmd(SimpleCommand *cmd) {
  if (cmd->args[1] == NULL) {
    for (int i = 0; i < g_alias_count; i++) {
      printf("alias %s='%s'\n", g_aliases[i].name, g_aliases[i].command);
    }
    return;
  }

  char *arg = cmd->args[1];
  char *equals = strchr(arg, '=');

  if (equals == NULL) {
    for (int i = 0; i < g_alias_count; i++) {
      if (strcmp(g_aliases[i].name, arg) == 0) {
        printf("alias %s='%s'\n", g_aliases[i].name, g_aliases[i].command);
        return;
      }
    }
  } else {
    *equals = '\0';
    char *name = arg;
    char *command = equals + 1;

    // Remove surrounding quotes from command if they exist
    if ((*command == '"' || *command == '\'') &&
        command[strlen(command) - 1] == *command) {
      command++;
      command[strlen(command) - 1] = '\0';
    }

    add_alias(name, command);
    save_aliases();
  }
}
