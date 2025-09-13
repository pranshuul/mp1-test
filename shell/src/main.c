#include "../include/intrinsics.h"
#include "../include/io.h"
#include "../include/jobs.h"
#include "../include/parser.h"
#include "../include/shell.h"

// --- Global Variable Definitions ---
pid_t g_fg_pgid = 0;
char g_shell_home_dir[PATH_MAX];

void sigint_handler(int sig) {
  // Suppress unused variable warning
  (void)sig;
  if (g_fg_pgid > 0) {
    kill(-g_fg_pgid, SIGINT);
  }
}

void sigtstp_handler(int sig) {
  // Suppress unused variable warning
  (void)sig;
  if (g_fg_pgid > 0) {
    kill(-g_fg_pgid, SIGTSTP);
  }
}

void shell_loop(void) {
  char input[MAX_INPUT_SIZE];
  ASTNode *ast = NULL;

  while (1) {
    // Only display prompt if a foreground job isn't running
    if (g_fg_pgid == 0) {
      check_background_jobs();
      if (isatty(STDIN_FILENO)) {
        display_prompt();
      }
    }

    if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
      // Handle Ctrl-D (EOF)
      if (isatty(STDIN_FILENO))
        printf("\n");
      break;
    }

    // Ignore empty input or comments
    if (strlen(input) > 0 && input[0] != '\n' && input[0] != '#') {
      input[strcspn(input, "\n")] = 0; // Remove trailing newline
      add_to_history(input);
      ast = parse_input(input);
      if (ast) {
        execute_ast(ast);
        free_ast(ast);
      }
    }
  }
}

int main(void) {
  // --- Initialize Shell Home Directory ---
  if (getcwd(g_shell_home_dir, sizeof(g_shell_home_dir)) == NULL) {
    perror("Failed to get shell home directory");
    return EXIT_FAILURE;
  }

  // Setup signal handlers
  signal(SIGINT, sigint_handler);
  signal(SIGTSTP, sigtstp_handler);
  signal(SIGQUIT, SIG_IGN); // Ignore Ctrl-Backslash (SIGQUIT)
  signal(SIGTTIN, SIG_IGN); // Ignore terminal input for background processes
  signal(SIGTTOU, SIG_IGN); // Ignore terminal output for background processes

  // Set shell's process group and take control of the terminal
  if (isatty(STDIN_FILENO)) {
    pid_t shell_pgid = getpid();
    setpgid(shell_pgid, shell_pgid);
    tcsetpgrp(STDIN_FILENO, shell_pgid);
  }

  init_jobs();
  init_history();
  shell_loop();

  return EXIT_SUCCESS;
}
