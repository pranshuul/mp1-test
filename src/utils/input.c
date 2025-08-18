// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

static struct termios orig_termios;
static int raw_mode_enabled = 0;

// Restore original terminal settings
void disable_raw_mode() {
  if (raw_mode_enabled) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    raw_mode_enabled = 0;
  }
}

// Enable raw mode for character-by-character input
void enable_raw_mode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
    perror("tcgetattr");
    exit(1);
  }
  atexit(disable_raw_mode);

  struct termios raw = orig_termios;
  // Input flags: no break, no CR to NL, no parity check, no strip bit, no
  // start/stop output control.
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  // Output flags: disable post-processing.
  raw.c_oflag &= ~(OPOST);
  // Control flags: set character size to 8 bits.
  raw.c_cflag |= (CS8);
  // Local flags: no echo, no canonical mode, no extended input, no signal chars
  // (SIGINT, SIGTSTP).
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  raw.c_cc[VMIN] = 0; // Return each byte, or zero bytes if no data is available
  raw.c_cc[VTIME] = 1; // Wait up to 1/10th of a second for data

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    perror("tcsetattr");
    exit(1);
  }
  raw_mode_enabled = 1;
}

// The core function to read a line with advanced editing capabilities
char *read_line_raw(const char *prompt) {
  enable_raw_mode();

  char line[MAX_LINE_LEN] = {0};
  int len = 0; // total length of the string in 'line'
  int pos = 0; // current cursor position in 'line'

  int history_index = g_history_count;

  printf("%s", prompt);
  fflush(stdout);

  while (1) {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1)
      continue;

    if (c == '\x1b') { // Escape sequence (arrow keys)
      char seq[3];
      if (read(STDIN_FILENO, &seq[0], 1) != 1)
        continue;
      if (read(STDIN_FILENO, &seq[1], 1) != 1)
        continue;

      if (seq[0] == '[') {
        if (seq[1] == 'A') { // Up arrow
          if (history_index > 0) {
            history_index--;
            strcpy(line, g_history[history_index]);
            len = pos = strlen(line);
            printf("\r%s\x1b[K%s", prompt, line); // Redraw line
          }
        } else if (seq[1] == 'B') { // Down arrow
          if (history_index < g_history_count - 1) {
            history_index++;
            strcpy(line, g_history[history_index]);
            len = pos = strlen(line);
            printf("\r%s\x1b[K%s", prompt, line);
          } else { // Go to a new blank line
            history_index = g_history_count;
            line[0] = '\0';
            len = pos = 0;
            printf("\r%s\x1b[K", prompt);
          }
        } else if (seq[1] == 'C') { // Right arrow
          if (pos < len) {
            pos++;
            printf("\x1b[C"); // Move cursor right
          }
        } else if (seq[1] == 'D') { // Left arrow
          if (pos > 0) {
            pos--;
            printf("\x1b[D"); // Move cursor left
          }
        }
      }
      fflush(stdout);
      continue;
    }

    switch (c) {
    case '\n': // Enter key
      printf("\n");
      disable_raw_mode();
      return strdup(line);

    case 127: // Backspace key
      if (pos > 0) {
        memmove(&line[pos - 1], &line[pos], len - pos);
        pos--;
        len--;
        line[len] = '\0';
        // Redraw the line from the cursor position
        printf("\x1b[D\x1b[K%s", &line[pos]);
        // Move cursor back to the correct position
        if (len > pos)
          printf("\x1b[%dD", len - pos);
      }
      break;

    case 4: // Ctrl-D
      if (len == 0) {
        printf("\nexit\n");
        disable_raw_mode();
        exit(0);
      }
      break;

    default: // Regular character input
      if (isprint(c) && len < MAX_LINE_LEN - 1) {
        memmove(&line[pos + 1], &line[pos], len - pos);
        line[pos] = c;
        len++;
        pos++;
        line[len] = '\0';
        // Redraw the line from the cursor position
        printf("\x1b[K%s", &line[pos - 1]);
        // Move cursor back to the correct position
        if (len > pos)
          printf("\x1b[%dD", len - pos);
      }
      break;
    }
    fflush(stdout);
  }
}
