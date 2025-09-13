#include "../include/parser.h"
#include <ctype.h> // For isspace()

// Tokenizer state
static const char *g_input_stream;
static char g_current_token[MAX_INPUT_SIZE];

// Forward declarations for recursive parsing
static ASTNode *parse_sequence(void);

static void skip_whitespace() {
  while (*g_input_stream && isspace((unsigned char)*g_input_stream)) {
    g_input_stream++;
  }
}

static int get_token() {
  skip_whitespace();
  if (*g_input_stream == '\0')
    return 0;

  const char *p = g_input_stream;
  int len = 0;

  if (strchr("|;&<>", *p)) {
    if (*p == '>' && *(p + 1) == '>') {
      len = 2;
    } else if (*p == '<' && *(p + 1) == '<') {
      len = 2;
    } else {
      len = 1;
    }
  } else {
    while (*p && !isspace((unsigned char)*p) && !strchr("|;&<>", *p)) {
      p++;
    }
    len = p - g_input_stream;
  }

  strncpy(g_current_token, g_input_stream, len);
  g_current_token[len] = '\0';
  g_input_stream += len;
  return 1;
}

static int peek(const char *str) {
  skip_whitespace();
  return strncmp(g_input_stream, str, strlen(str)) == 0;
}

static ASTNode *parse_command() {
  CommandNode *cmd = calloc(1, sizeof(CommandNode));
  if (!cmd) {
    perror("calloc");
    return NULL;
  }
  cmd->type = NODE_COMMAND;
  cmd->args = calloc(MAX_ARGS, sizeof(char *));
  if (!cmd->args) {
    perror("calloc");
    free(cmd);
    return NULL;
  }
  cmd->redirections = NULL;

  Redirection **redir_next = &cmd->redirections;

  while
