// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

// Define control characters for clarity
#define CTRL_C 3
#define CTRL_D 4
#define BACKSPACE 127
#define ENTER_KEY_LF 10 // Line Feed ('\n')
#define ENTER_KEY_CR 13 // Carriage Return ('\r')

static struct termios orig_termios;
static int raw_mode_enabled = 0;

void disable_raw_mode() {
  if (raw_mode_enabled)
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
  raw_mode_enabled = 0;
}

void enable_raw_mode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
    perror("tcgetattr");
    exit(1);
  }
  atexit(disable_raw_mode);
  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    perror("tcsetattr");
    exit(1);
  }
  raw_mode_enabled = 1;
}

char *read_line_raw(const char *prompt) {
  enable_raw_mode();
  char line[MAX_LINE_LEN] = {0};
  int len = 0, pos = 0, history_index = g_history_count;
  printf("%s", prompt);
  fflush(stdout);
  while (1) {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1)
      continue;
    if (c == '\x1b') { // Escape sequence (arrow keys)
      char seq[2];
      if (read(STDIN_FILENO, &seq[0], 1) != 1)
        continue;
      if (read(STDIN_FILENO, &seq[1], 1) != 1)
        continue;
      if (seq[0] == '[') {
        if (seq[1] == 'A') { // Up
          if (history_index > 0) {
            history_index--;
            strcpy(line, g_history[history_index]);
            len = pos = strlen(line);
            printf("\r%s\x1b[K%s", prompt, line);
          }
        } else if (seq[1] == 'B') { // Down
          if (history_index < g_history_count - 1) {
            history_index++;
            strcpy(line, g_history[history_index]);
            len = pos = strlen(line);
            printf("\r%s\x1b[K%s", prompt, line);
          } else {
            history_index = g_history_count;
            line[0] = '\0';
            len = pos = 0;
            printf("\r%s\x1b[K", prompt);
          }
        } else if (seq[1] == 'C') {
          if (pos < len) {
            pos++;
            printf("\x1b[C");
          }
        } // Right
        else if (seq[1] == 'D') {
          if (pos > 0) {
            pos--;
            printf("\x1b[D");
          }
        } // Left
      }
      fflush(stdout);
      continue;
    }
    switch (c) {
    case ENTER_KEY_CR:
    case ENTER_KEY_LF:
      printf("\n");
      disable_raw_mode();
      return strdup(line);
    case CTRL_C:
      printf("^C\n");
      disable_raw_mode();
      return strdup("");
    case CTRL_D:
      if (len == 0) {
        printf("\nexit\n");
        exit(0);
      }
      break;
    case BACKSPACE:
      if (pos > 0) {
        memmove(&line[pos - 1], &line[pos], len - pos);
        pos--;
        len--;
        line[len] = '\0';
        printf("\x1b[D\x1b[K%s", &line[pos]);
        if (len > pos)
          printf("\x1b[%dD", len - pos);
      }
      break;
    default:
      if (isprint(c) && len < MAX_LINE_LEN - 1) {
        memmove(&line[pos + 1], &line[pos], len - pos);
        line[pos] = c;
        len++;
        pos++;
        line[len] = '\0';
        printf("\x1b[K%s", &line[pos - 1]);
        if (len > pos)
          printf("\x1b[%dD", len - pos);
      }
      break;
    }
    fflush(stdout);
  }
}
