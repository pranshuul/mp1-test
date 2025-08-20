// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

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

void reveal(SimpleCommand *cmd) {
  int long_format = 0, show_hidden = 0;
  char *target_path = ".";
  for (int i = 1; cmd->args[i]; i++) {
    if (strcmp(cmd->args[i], "--aliases") == 0) {
      for (int j = 0; j < g_alias_count; j++)
        printf("%s: %s\n", g_aliases[j].name, g_aliases[j].command);
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
  char expanded_path[MAX_PATH_LEN];
  if (target_path[0] == '~') {
    snprintf(expanded_path, sizeof(expanded_path), "%s%s", g_shell_home_dir,
             target_path + 1);
    target_path = expanded_path;
  }
  struct stat path_stat;
  if (stat(target_path, &path_stat) != 0) {
    perror("reveal");
    return;
  }
  if (S_ISDIR(path_stat.st_mode)) {
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
      // **FIX**: Check the return value of snprintf to satisfy the compiler's
      // safety checks.
      int len_written = snprintf(full_path, sizeof(full_path), "%s/%s",
                                 target_path, dir->d_name);

      // If len_written is negative (error) or would have overflowed the buffer,
      // print an error and skip.
      if (len_written < 0 || (size_t)len_written >= sizeof(full_path)) {
        fprintf(stderr, "reveal: path too long: %s/%s\n", target_path,
                dir->d_name);
        continue;
      }

      reveal_file(full_path, dir->d_name, long_format);
    }
    closedir(d);
  } else {
    reveal_file(target_path, target_path, long_format);
  }
}
