// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

typedef struct {
  const char *name;
  void (*func)(SimpleCommand *);
} builtin_command;

static const builtin_command builtins[] = {
    {"hop", hop},         {"reveal", reveal},
    {"alias", alias_cmd}, {"log", log_cmd},
    {"ping", ping},       {"activities", activities},
    {"fg", fg},           {"bg", bg},
    {"exit", NULL} // Special case handled in shell loop
};

int is_builtin(const char *cmd_name) {
  if (!cmd_name)
    return 0;
  int num_builtins = sizeof(builtins) / sizeof(builtin_command);
  for (int i = 0; i < num_builtins; i++) {
    if (strcmp(cmd_name, builtins[i].name) == 0) {
      return 1;
    }
  }
  return 0;
}

void execute_builtin(SimpleCommand *cmd) {
  if (cmd->args[0] == NULL)
    return;
  if (strcmp(cmd->args[0], "exit") == 0) {
    exit(0);
  }

  int num_builtins = sizeof(builtins) / sizeof(builtin_command);
  for (int i = 0; i < num_builtins; i++) {
    if (strcmp(cmd->args[0], builtins[i].name) == 0) {
      builtins[i].func(cmd);
      return;
    }
  }
}
