#include "../include/io.h"
#include "../include/shell.h"

void display_prompt(void) {
  char hostname[HOST_NAME_MAX];
  char cwd[PATH_MAX];
  char *username = getlogin();

  // --- Get system info ---
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

  // --- Format the path with tilde replacement ---
  char display_path[PATH_MAX];
  size_t home_len = strlen(g_shell_home_dir);

  // Check if the current path is the shell's home directory or a subdirectory
  if (strncmp(cwd, g_shell_home_dir, home_len) == 0) {
    // Check for exact match or ancestor directory
    if (cwd[home_len] == '/' || cwd[home_len] == '\0') {
      snprintf(display_path, sizeof(display_path), "~%s", cwd + home_len);
    } else {
      // Not a true ancestor (e.g., home is /usr/user, cwd is /usr/username)
      strncpy(display_path, cwd, sizeof(display_path));
    }
  } else {
    // Current path is not a descendant of shell's home, show absolute path
    strncpy(display_path, cwd, sizeof(display_path));
  }
  display_path[sizeof(display_path) - 1] = '\0'; // Ensure null termination

  // --- Print the final prompt ---
  printf("<%s@%s:%s> ", username, hostname, display_path);
  fflush(stdout);
}
