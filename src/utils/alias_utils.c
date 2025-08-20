// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

#define ALIAS_FILE ".aliases"

void load_aliases(void) {
  FILE *file = fopen(ALIAS_FILE, "r");
  if (!file)
    return;

  char line[MAX_LINE_LEN];
  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\r\n")] = 0;
    if (strncmp(line, "alias ", 6) == 0) {
      char *equals = strchr(line + 6, '=');
      if (equals) {
        *equals = '\0';
        char *name = line + 6;
        char *command = equals + 1;
        if ((*command == '"' || *command == '\'') &&
            command[strlen(command) - 1] == *command) {
          command++;
          command[strlen(command) - 1] = '\0';
        }
        add_alias(name, command);
      }
    }
  }
  fclose(file);
}

void save_aliases(void) {
  FILE *file = fopen(ALIAS_FILE, "w");
  if (!file) {
    perror("alias save");
    return;
  }
  for (int i = 0; i < g_alias_count; i++) {
    fprintf(file, "alias %s=\"%s\"\n", g_aliases[i].name, g_aliases[i].command);
  }
  fclose(file);
}

void add_alias(const char *name, const char *command) {
  for (int i = 0; i < g_alias_count; i++) {
    if (strcmp(g_aliases[i].name, name) == 0) {
      free(g_aliases[i].command);
      g_aliases[i].command = strdup(command);
      return;
    }
  }
  if (g_alias_count < MAX_ALIASES) {
    g_aliases[g_alias_count].name = strdup(name);
    g_aliases[g_alias_count].command = strdup(command);
    g_alias_count++;
  }
}

char *get_alias_command(const char *name) {
  for (int i = 0; i < g_alias_count; i++) {
    if (strcmp(g_aliases[i].name, name) == 0) {
      return g_aliases[i].command;
    }
  }
  return NULL;
}
