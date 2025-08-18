// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

#define HISTORY_FILE ".shell_history"

void load_history(void) {
  FILE *file = fopen(HISTORY_FILE, "r");
  if (!file)
    return;

  char line[MAX_LINE_LEN];
  while (fgets(line, sizeof(line), file) &&
         g_history_count < HISTORY_CAPACITY) {
    line[strcspn(line, "\r\n")] = 0; // Remove trailing newline
    add_to_history(line);
  }
  fclose(file);
}

void save_history(void) {
  FILE *file = fopen(HISTORY_FILE, "w");
  if (!file) {
    perror("history");
    return;
  }
  for (int i = 0; i < g_history_count; i++) {
    fprintf(file, "%s\n", g_history[i]);
  }
  fclose(file);
}

void add_to_history(const char *command) {
  if (g_history_count > 0 &&
      strcmp(g_history[g_history_count - 1], command) == 0) {
    return; // Don't add duplicate consecutive commands
  }

  if (g_history_count < HISTORY_CAPACITY) {
    g_history[g_history_count++] = strdup(command);
  } else {
    free(g_history[0]);
    for (int i = 1; i < HISTORY_CAPACITY; i++) {
      g_history[i - 1] = g_history[i];
    }
    g_history[HISTORY_CAPACITY - 1] = strdup(command);
  }
}
