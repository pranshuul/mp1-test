#include "../include/io.h"
#include "../include/shell.h"

void display_prompt(void) {
  char hostname[HOST_NAME_MAX];
  char cwd[PATH_MAX];
  char *username = getlogin();
  char *home_dir = getenv("HOME");

  if (gethostname(hostname, sizeof(hostname)) != 0) {
    perror("gethostname");
    strcpy(hostname, "localhost");
  }
  if (!username) {
    struct passwd *pw = getpwuid(getuid());
    username = pw ? pw->pw_name : "user";
  }
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    perror("getcwd");
    strcpy(cwd, "");
  }

  char display_path[PATH_MAX];
  if (home_dir && strlen(cwd) >= strlen(home_dir) &&
      strncmp(cwd, home_dir, strlen(home_dir)) == 0) {
    // Replace home directory path with ~
    snprintf(display_path, sizeof(display_path), "~%s", cwd + strlen(home_dir));
  } else {
    // Otherwise, use the full path
    strncpy(display_path, cwd, sizeof(display_path) - 1);
    display_path[sizeof(display_path) - 1] = '\0'; // Ensure null termination
  }

  printf("%s@%s:%s> ", username, hostname, display_path);
  fflush(stdout);
}
