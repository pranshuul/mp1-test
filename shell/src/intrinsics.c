#include "../include/intrinsics.h"
#include "../include/jobs.h"

// History storage
static char *history[MAX_HISTORY];
static int history_count = 0;
static int history_start = 0;

void init_history() {
  for (int i = 0; i < MAX_HISTORY; ++i)
    history[i] = NULL;
}

void add_to_history(const char *command) {
  if (history_count > 0 &&
      strcmp(history[(history_start + history_count - 1) % MAX_HISTORY],
             command) == 0) {
    return; // Don't add duplicate consecutive commands
  }
  if (history_count == MAX_HISTORY) {
    free(history[history_start]);
    history_start = (history_start + 1) % MAX_HISTORY;
    history_count--;
  }
  int index = (history_start + history_count) % MAX_HISTORY;
  history[index] = strdup(command);
  history_count++;
}

void clear_history() {
  for (int i = 0; i < history_count; ++i) {
    free(history[(history_start + i) % MAX_HISTORY]);
    history[(history_start + i) % MAX_HISTORY] = NULL;
  }
  history_count = 0;
  history_start = 0;
}

void display_history() {
  int start = (history_count > 10) ? history_count - 10 : 0;
  for (int i = start; i < history_count; ++i) {
    int index = (history_start + i) % MAX_HISTORY;
    printf("%d: %s\n", i + 1, history[index]);
  }
}

int save_history(const char *filename) {
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    perror("log -s");
    return 1;
  }
  for (int i = 0; i < history_count; ++i) {
    int index = (history_start + i) % MAX_HISTORY;
    fprintf(fp, "%s\n", history[index]);
  }
  fclose(fp);
  return 0;
}

int builtin_log(char **args) {
  if (args[1] == NULL) {
    display_history();
  } else if (strcmp(args[1], "-c") == 0) {
    clear_history();
  } else if (strcmp(args[1], "-s") == 0) {
    if (args[2] == NULL) {
      fprintf(stderr, "log -s: missing filename\n");
      return 1;
    }
    return save_history(args[2]);
  } else {
    fprintf(stderr, "log: invalid option %s\n", args[1]);
    return 1;
  }
  return 0;
}

int builtin_hop(char **args) {
  static char prev_dir[PATH_MAX] = "";
  char current_dir[PATH_MAX];

  if (getcwd(current_dir, PATH_MAX) == NULL) {
    perror("hop");
    return 1;
  }

  const char *target_dir = args[1];
  if (target_dir == NULL) {
    target_dir = getenv("HOME");
  } else if (strcmp(target_dir, "-") == 0) {
    if (strlen(prev_dir) == 0) {
      fprintf(stderr, "hop: no previous directory\n");
      return 1;
    }
    target_dir = prev_dir;
  }

  if (chdir(target_dir) != 0) {
    perror("hop");
    return 1;
  }

  strcpy(prev_dir, current_dir);
  return 0;
}

int builtin_reveal(char **args) {
  int show_hidden = 0;
  int long_format = 0;
  char *target_dir = ".";
  int path_arg_found = 0;

  for (int i = 1; args[i] != NULL; ++i) {
    if (args[i][0] == '-') {
      for (size_t j = 1; j < strlen(args[i]); ++j) {
        if (args[i][j] == 'a')
          show_hidden = 1;
        else if (args[i][j] == 'l')
          long_format = 1;
        else {
          fprintf(stderr, "reveal: invalid option -- '%c'\n", args[i][j]);
          return 1;
        }
      }
    } else {
      if (path_arg_found) {
        fprintf(stderr, "reveal: too many arguments\n");
        return 1;
      }
      target_dir = args[i];
      path_arg_found = 1;
    }
  }

  DIR *dir = opendir(target_dir);
  if (!dir) {
    perror("reveal");
    return 1;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (!show_hidden && entry->d_name[0] == '.') {
      continue;
    }
    printf("%s%s", entry->d_name, long_format ? "\n" : "  ");
  }

  if (!long_format)
    printf("\n");
  closedir(dir);
  return 0;
}

int builtin_ping(char **args) {
  if (args[1] == NULL || args[2] == NULL) {
    fprintf(stderr, "Usage: ping <pid> <signal>\n");
    return 1;
  }
  pid_t pid = atoi(args[1]);
  int sig = atoi(args[2]);
  if (kill(pid, sig) != 0) {
    perror("ping");
    return 1;
  }
  return 0;
}

int builtin_activities(char **args) {
  for (int i = 0; i < MAX_JOBS; ++i) {
    if (job_table[i].pgid != 0) {
      const char *status_str = "Running";
      if (job_table[i].status == JOB_STOPPED) {
        status_str = "Stopped";
      }
      printf("[%d] %d %s %s\n", job_table[i].job_id, job_table[i].pgid,
             status_str, job_table[i].command);
    }
  }
  return 0;
}

int builtin_fg(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "fg: job ID required\n");
    return 1;
  }
  int job_id = atoi(args[1]);
  Job *job = get_job_by_id(job_id);
  if (!job) {
    fprintf(stderr, "fg: job not found: %d\n", job_id);
    return 1;
  }

  g_fg_pgid = job->pgid;
  tcsetpgrp(STDIN_FILENO, g_fg_pgid);

  if (job->status == JOB_STOPPED) {
    kill(-job->pgid, SIGCONT);
  }

  int status;
  waitpid(job->pgid, &status, WUNTRACED);

  if (WIFSTOPPED(status)) {
    update_job_status(job->pgid, WSTOPSIG(status));
    print_job_status(job, 0);
  } else {
    remove_job(job->pgid);
  }

  g_fg_pgid = 0;
  tcsetpgrp(STDIN_FILENO, getpgrp());
  return 0;
}

int builtin_bg(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "bg: job ID required\n");
    return 1;
  }
  int job_id = atoi(args[1]);
  Job *job = get_job_by_id(job_id);
  if (!job) {
    fprintf(stderr, "bg: job not found: %d\n", job_id);
    return 1;
  }
  if (job->status != JOB_STOPPED) {
    fprintf(stderr, "bg: job %d is already running\n", job_id);
    return 1;
  }

  kill(-job->pgid, SIGCONT);
  job->status = JOB_RUNNING;
  print_job_status(job, 0);
  return 0;
}

// Map command names to functions
static const BuiltinCommand builtins[] = {{"hop", builtin_hop},
                                          {"reveal", builtin_reveal},
                                          {"log", builtin_log},
                                          {"ping", builtin_ping},
                                          {"activities", builtin_activities},
                                          {"fg", builtin_fg},
                                          {"bg", builtin_bg},
                                          {NULL, NULL}};

int handle_builtin(CommandNode *cmd) {
  if (cmd->arg_count == 0)
    return -1;
  for (int i = 0; builtins[i].name; i++) {
    if (strcmp(cmd->args[0], builtins[i].name) == 0) {
      return builtins[i].func(cmd->args);
    }
  }
  return -1; // Not a built-in
}
