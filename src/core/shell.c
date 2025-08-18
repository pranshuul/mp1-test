// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

// Define global variables
Alias g_aliases[MAX_ALIASES];
int g_alias_count = 0;
Job g_jobs[MAX_JOBS];
int g_job_count = 0;
char *g_shell_home_dir;
char *g_prev_dir;
char *g_history[HISTORY_CAPACITY];
int g_history_count = 0;

pid_t g_shell_pgid;
struct termios g_shell_termios;

void init_shell() {
  g_shell_home_dir = getcwd(NULL, 0); // Home dir is where shell starts
  g_prev_dir = strdup(g_shell_home_dir);

  load_aliases();
  load_history();
  init_jobs();

  // Signal Handling
  signal(SIGINT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGCHLD, reap_children);

  g_shell_pgid = getpid();
  if (setpgid(g_shell_pgid, g_shell_pgid) < 0) {
    perror("setpgid");
    exit(EXIT_FAILURE);
  }
  tcsetpgrp(STDIN_FILENO, g_shell_pgid);
  tcgetattr(STDIN_FILENO, &g_shell_termios);
}

void launch_pipeline(Pipeline *p) {
  if (p->num_commands == 1 && is_builtin(p->commands[0]->args[0])) {
    execute_builtin(p->commands[0]);
    return;
  }

  pid_t pgid = 0;
  int pipe_fds[2], in_fd = STDIN_FILENO, out_fd = STDOUT_FILENO;

  for (int i = 0; i < p->num_commands; i++) {
    if (i < p->num_commands - 1) {
      if (pipe(pipe_fds) < 0) {
        perror("pipe");
        return;
      }
      out_fd = pipe_fds[1];
    } else {
      out_fd = STDOUT_FILENO;
    }

    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      return;
    } else if (pid == 0) { // Child
      if (i == 0)
        pgid = getpid();
      setpgid(getpid(), pgid);
      if (!p->background)
        tcsetpgrp(STDIN_FILENO, pgid);

      signal(SIGINT, SIG_DFL);
      signal(SIGTSTP, SIG_DFL);

      if (in_fd != STDIN_FILENO) {
        dup2(in_fd, STDIN_FILENO);
        close(in_fd);
      }
      if (out_fd != STDOUT_FILENO) {
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
      }

      SimpleCommand *sc = p->commands[i];
      if (sc->inputFile) {
        int fd = open(sc->inputFile, O_RDONLY);
        if (fd < 0) {
          perror("open");
          exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
      }
      if (sc->outputFile) {
        int flags = O_WRONLY | O_CREAT | (sc->append ? O_APPEND : O_TRUNC);
        int fd = open(sc->outputFile, flags, 0644);
        if (fd < 0) {
          perror("open");
          exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
      }

      if (is_builtin(sc->args[0])) {
        execute_builtin(sc);
        exit(0);
      } else {
        execvp(sc->args[0], sc->args);
        perror(sc->args[0]);
        exit(1);
      }
    } else { // Parent
      if (i == 0)
        pgid = pid;
      setpgid(pid, pgid);
    }

    if (in_fd != STDIN_FILENO)
      close(in_fd);
    if (out_fd != STDOUT_FILENO)
      close(out_fd);
    in_fd = pipe_fds[0];
  }

  Job *job = add_job(pgid, p->full_text, p->background);

  if (!p->background) {
    tcsetpgrp(STDIN_FILENO, pgid);
    int status;
    waitpid(pgid, &status, WUNTRACED);

    if (WIFSTOPPED(status)) {
      printf("\nStopped: %s\n", p->full_text);
      job->status = STOPPED;
      tcgetattr(STDIN_FILENO, &job->tmodes);
    } else {
      remove_job_by_pgid(pgid);
    }
    tcsetpgrp(STDIN_FILENO, g_shell_pgid);
  } else {
    printf("[%d] %d\n", g_job_count, pgid);
  }
}

void shell_loop(void) {
  char *line;
  Pipeline **pipelines;
  int pipeline_count;

  while (1) {
    reap_children(0);
    char *prompt = get_prompt();
    line = read_line_raw(prompt);
    free(prompt);

    if (strlen(line) > 0) {
      add_to_history(line);
      save_history();

      pipelines = parse_line(line, &pipeline_count);
      for (int i = 0; i < pipeline_count; i++) {
        launch_pipeline(pipelines[i]);
        free_pipeline(pipelines[i]);
      }
      free(pipelines);
    }
    free(line);
  }
}

int main() {
  init_shell();
  shell_loop();
  return EXIT_SUCCESS;
}
