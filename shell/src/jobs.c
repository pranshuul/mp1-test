#include "../include/jobs.h"
#include "../include/intrinsics.h"

// Globals for job management
Job job_table[MAX_JOBS];
int next_job_id = 1;
extern pid_t g_fg_pgid;

void init_jobs() {
  for (int i = 0; i < MAX_JOBS; ++i) {
    job_table[i].pgid = 0;
    job_table[i].command = NULL;
  }
}

int add_job(pid_t pgid, const char *command, int is_background) {
  for (int i = 0; i < MAX_JOBS; ++i) {
    if (job_table[i].pgid == 0) {
      job_table[i].pgid = pgid;
      job_table[i].job_id = next_job_id++;
      job_table[i].status = JOB_RUNNING;
      job_table[i].command = strdup(command);
      job_table[i].is_background = is_background;
      return job_table[i].job_id;
    }
  }
  fprintf(stderr, "shell: too many jobs\n");
  return -1;
}

void remove_job(pid_t pgid) {
  for (int i = 0; i < MAX_JOBS; ++i) {
    if (job_table[i].pgid == pgid) {
      free(job_table[i].command);
      job_table[i].pgid = 0;
      job_table[i].command = NULL;
      return;
    }
  }
}

Job *get_job_by_id(int job_id) {
  for (int i = 0; i < MAX_JOBS; ++i) {
    if (job_table[i].pgid != 0 && job_table[i].job_id == job_id) {
      return &job_table[i];
    }
  }
  return NULL;
}

Job *get_job_by_pgid(pid_t pgid) {
  for (int i = 0; i < MAX_JOBS; ++i) {
    if (job_table[i].pgid == pgid) {
      return &job_table[i];
    }
  }
  return NULL;
}

void update_job_status(pid_t pgid, int status) {
  Job *job = get_job_by_pgid(pgid);
  if (!job)
    return;
  if (WIFSTOPPED(status)) {
    job->status = JOB_STOPPED;
  } else if (WIFSIGNALED(status) || WIFEXITED(status)) {
    job->status = JOB_DONE;
  }
}

void print_job_status(Job *job, int is_bg_completion) {
  if (is_bg_completion) {
    printf("[%d] Done %s\n", job->job_id, job->command);
  } else {
    printf("[%d] %d %s\n", job->job_id, job->pgid, job->command);
  }
}

void check_background_jobs() {
  int status;
  pid_t pid;
  while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
    Job *job = get_job_by_pgid(pid);
    if (job) {
      if (WIFSTOPPED(status)) {
        job->status = JOB_STOPPED;
        fprintf(stdout, "\nStopped: [%d] %s\n", job->job_id, job->command);
      } else {
        if (job->is_background) {
          print_job_status(job, 1);
        }
        remove_job(pid);
      }
    }
  }
}

static char *reconstruct_command(CommandNode *cmd) {
  char buffer[MAX_INPUT_SIZE] = {0};
  for (int i = 0; cmd->args[i]; i++) {
    strcat(buffer, cmd->args[i]);
    strcat(buffer, " ");
  }
  return strdup(buffer);
}

static void apply_redirections(CommandNode *cmd) {
  Redirection *r = cmd->redirections;
  while (r) {
    int fd;
    if (r->type == REDIR_IN || r->type == REDIR_HEREDOC) {
      fd = open(r->filename, O_RDONLY);
      if (fd < 0) {
        fprintf(stderr, "%s: No such file or directory\n", r->filename);
        exit(EXIT_FAILURE);
      }
      dup2(fd, STDIN_FILENO);
      close(fd);
    } else if (r->type == REDIR_OUT) {
      fd = open(r->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      dup2(fd, STDOUT_FILENO);
      close(fd);
    } else if (r->type == REDIR_APPEND) {
      fd = open(r->filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
      dup2(fd, STDOUT_FILENO);
      close(fd);
    }
    r = r->next;
  }
}

static void launch_process(CommandNode *cmd, pid_t pgid, int is_background,
                           int in_fd, int out_fd) {
  pid_t pid = getpid();
  if (pgid == 0)
    pgid = pid;
  setpgid(pid, pgid);

  if (!is_background) {
    tcsetpgrp(STDIN_FILENO, pgid);
  }

  signal(SIGINT, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);
  signal(SIGTSTP, SIG_DFL);
  signal(SIGTTIN, SIG_DFL);
  signal(SIGTTOU, SIG_DFL);

  if (in_fd != STDIN_FILENO) {
    dup2(in_fd, STDIN_FILENO);
    close(in_fd);
  }
  if (out_fd != STDOUT_FILENO) {
    dup2(out_fd, STDOUT_FILENO);
    close(out_fd);
  }

  apply_redirections(cmd);

  if (execvp(cmd->args[0], cmd->args) < 0) {
    perror(cmd->args[0]);
    exit(EXIT_FAILURE);
  }
}

static void execute_command(CommandNode *cmd, pid_t pgid, int is_background) {
  if (handle_builtin(cmd) != -1) {
    return;
  }

  char *full_command = reconstruct_command(cmd);

  pid_t pid = fork();
  if (pid == 0) { // Child
    launch_process(cmd, pgid, is_background, STDIN_FILENO, STDOUT_FILENO);
  } else { // Parent
    if (pgid == 0)
      pgid = pid;
    setpgid(pid, pgid);

    add_job(pgid, full_command, is_background);

    if (!is_background) {
      g_fg_pgid = pgid;
      int status;
      waitpid(pid, &status, WUNTRACED);

      if (WIFSTOPPED(status)) {
        update_job_status(pgid, status);
        Job *job = get_job_by_pgid(pgid);
        if (job)
          print_job_status(job, 0);
      } else {
        remove_job(pgid);
      }

      g_fg_pgid = 0;
      tcsetpgrp(STDIN_FILENO, getpgrp());
    } else {
      Job *job = get_job_by_pgid(pgid);
      if (job)
        print_job_status(job, 0);
    }
  }
  free(full_command);
}

static void execute_pipe(PipeNode *node) {
  int pipefd[2];
  pipe(pipefd);

  pid_t left_pid, right_pid;

  left_pid = fork();
  if (left_pid == 0) { // First child
    close(pipefd[0]);
    // Recursive call for complex pipes
    if (node->left->type == NODE_COMMAND) {
      launch_process((CommandNode *)node->left, 0, 0, STDIN_FILENO, pipefd[1]);
    } else {
      // This simple implementation does not fully support chaining complex
      // structures in pipes For the assignment, we assume pipe is between
      // simple commands.
      exit(1);
    }
  }

  right_pid = fork();
  if (right_pid == 0) { // Second child
    close(pipefd[1]);
    if (node->right->type == NODE_COMMAND) {
      launch_process((CommandNode *)node->right, left_pid, 0, pipefd[0],
                     STDOUT_FILENO);
    } else if (node->right->type ==
               NODE_PIPE) { // Handle pipe chains like a | b | c
      execute_pipe((PipeNode *)node->right);
    } else {
      exit(1);
    }
  }

  // Parent
  close(pipefd[0]);
  close(pipefd[1]);

  int status;
  waitpid(left_pid, &status, 0);
  waitpid(right_pid, &status, 0);
}

void execute_ast(ASTNode *node) {
  if (!node)
    return;
  switch (node->type) {
  case NODE_COMMAND:
    execute_command((CommandNode *)node, 0, ((CommandNode *)node)->background);
    break;
  case NODE_PIPE:
    execute_pipe((PipeNode *)node);
    break;
  case NODE_SEQUENCE:
    execute_ast(((SequenceNode *)node)->left);
    check_background_jobs(); // Reap jobs between sequential commands
    execute_ast(((SequenceNode *)node)->right);
    break;
  }
}
