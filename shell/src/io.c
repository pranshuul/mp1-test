#include "../include/io.h"
#include "../include/shell.h"

void display_prompt(void) {
  char hostname[HOST_NAME_MAX];
  char *username = getlogin();
  char cwd[PATH_MAX];
  char *home_dir = getenv("HOME");

  if (gethostname(hostname, HOST_NAME_MAX) != 0) {
    perror("gethostname");
    strcpy(hostname, "localhost");
  }
  if (!username) {
    struct passwd *pw = getpwuid(getuid());
    username = pw ? pw->pw_name : "user";
  }
  if (getcwd(cwd, PATH_MAX) == NULL) {
    perror("getcwd");
    strcpy(cwd, "");
  }

  char display_path[PATH_MAX];
  if (home_dir && strncmp(cwd, home_dir, strlen(home_dir)) == 0) {
    snprintf(display_path, PATH_MAX, "~%s", cwd + strlen(home_dir));
  } else {
    strncpy(display_path, cwd, PATH_MAX);
  }

  printf("%s@%s:%s> ", username, hostname, display_path);
  fflush(stdout);
}
