// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

// Helper function to print file permissions in a format like "drwxr-xr--"
static void print_perms(mode_t mode) {
  printf((S_ISDIR(mode)) ? "d" : "-");
  printf((mode & S_IRUSR) ? "r" : "-");
  printf((mode & S_IWUSR) ? "w" : "-");
  printf((mode & S_IXUSR) ? "x" : "-");
  printf((mode & S_IRGRP) ? "r" : "-");
  printf((mode & S_IWGRP) ? "w" : "-");
  printf((mode & S_IXGRP) ? "x" : "-");
  printf((mode & S_IROTH) ? "r" : "-");
  printf((mode & S_IWOTH) ? "w" : "-");
  printf((mode & S_IXOTH) ? "x" : "-");
}

// Helper function to display detailed information for a single file
static void reveal_file(const char *path, const char *name, int long_format) {
  struct stat file_stat;
  if (stat(path, &file_stat) < 0) {
    perror("reveal: stat");
    return;
  }

  if (long_format) {
    print_perms(file_stat.st_mode);
    printf(" %2lu ", file_stat.st_nlink);

    struct passwd *pw = getpwuid(file_stat.st_uid);
    struct group *gr = getgrgid(file_stat.st_gid);
    printf("%-8s ", pw ? pw->pw_name : "n/a");
    printf("%-8s ", gr ? gr->gr_name : "n/a");

    printf("%8ld ", file_stat.st_size);

    char time_buf[80];
    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M",
             localtime(&file_stat.st_mtime));
    printf("%s ", time_buf);
  }
  printf("%s\n", name);
}

// Main function for the 'reveal' built-in command
void reveal(SimpleCommand *cmd) {
  int long_format = 0;
  int show_hidden = 0;
  char *target_path = "."; // Default to current directory

  // 1. Parse arguments for flags and the target path
  for (int i = 1; cmd->args[i]; i++) {
    if (strcmp(cmd->args[i], "--aliases") == 0) {
      for (int j = 0; j < g_alias_count; j++) {
        printf("%s: %s\n", g_aliases[j].name, g_aliases[j].command);
      }
      return;
    }
    if (cmd->args[i][0] == '-') {
      for (int j = 1; cmd->args[i][j]; j++) {
        if (cmd->args[i][j] == 'l')
          long_format = 1;
        else if (cmd->args[i][j] == 'a')
          show_hidden = 1;
      }
    } else {
      target_path = cmd->args[i];
    }
  }

  // 2. Expand '~' if it's the start of the path
  char expanded_path[MAX_PATH_LEN];
  if (target_path[0] == '~') {
    snprintf(expanded_path, sizeof(expanded_path), "%s%s", g_shell_home_dir,
             target_path + 1);
    target_path = expanded_path;
  }

  // 3. Check if the target path is a directory or a file
  struct stat path_stat;
  if (stat(target_path, &path_stat) != 0) {
    perror("reveal");
    return;
  }

  if (S_ISDIR(path_stat.st_mode)) {
    // It's a directory, so open and read its contents
    DIR *d = opendir(target_path);
    if (!d) {
      perror("reveal: opendir");
      return;
    }
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
      if (!show_hidden && dir->d_name[0] == '.')
        continue;

      char full_path[MAX_PATH_LEN];
      snprintf(full_path, sizeof(full_path), "%s/%s", target_path, dir->d_name);
      reveal_file(full_path, dir->d_name, long_format);
    }
    closedir(d);
  } else {
    // It's a single file, just show its info
    reveal_file(target_path, target_path, long_format);
  }
}
