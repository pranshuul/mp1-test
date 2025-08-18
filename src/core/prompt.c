// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

char *get_prompt() {
  char username[256];
  if (getlogin_r(username, sizeof(username)) != 0) {
    struct passwd *pw = getpwuid(getuid());
    if (pw)
      strcpy(username, pw->pw_name);
    else
      strcpy(username, "user");
  }

  char hostname[256];
  if (gethostname(hostname, sizeof(hostname)) != 0) {
    strcpy(hostname, "localhost");
  }

  char current_path[MAX_PATH_LEN];
  if (getcwd(current_path, sizeof(current_path)) == NULL) {
    strcpy(current_path, "?");
  }

  char display_path[MAX_PATH_LEN];
  if (strcmp(current_path, g_shell_home_dir) == 0) {
    strcpy(display_path, "~");
  } else if (strstr(current_path, g_shell_home_dir) == current_path) {
    snprintf(display_path, sizeof(display_path), "~%s",
             current_path + strlen(g_shell_home_dir));
  } else {
    strncpy(display_path, current_path, sizeof(display_path));
  }

  char *prompt = malloc(MAX_PATH_LEN * 2);
  snprintf(prompt, MAX_PATH_LEN * 2, "<%s@%s:%s> ", username, hostname,
           display_path);
  return prompt;
}
