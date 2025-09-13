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

  while (!peek("|") && !peek(";") && !peek("&") && *g_input_stream != '\0') {
    RedirType redir_type = REDIR_NONE;
    if (peek(">>")) {
      redir_type = REDIR_APPEND;
    } else if (peek("<<")) {
      redir_type = REDIR_HEREDOC;
    } else if (peek(">")) {
      redir_type = REDIR_OUT;
    } else if (peek("<")) {
      redir_type = REDIR_IN;
    }

    if (redir_type != REDIR_NONE) {
      get_token(); // Consume the redirection operator
      if (!get_token()) {
        fprintf(stderr, "Invalid Syntax!\n");
        free_ast((ASTNode *)cmd);
        return NULL;
      }
      Redirection *r = malloc(sizeof(Redirection));
      r->type = redir_type;
      r->filename = strdup(g_current_token);
      r->next = NULL;
      *redir_next = r;
      redir_next = &r->next;
    } else {
      if (!get_token())
        break;
      if (cmd->arg_count < MAX_ARGS - 1) {
        cmd->args[cmd->arg_count++] = strdup(g_current_token);
      }
    }
  }
  cmd->args[cmd->arg_count] = NULL;

  if (cmd->arg_count == 0 && cmd->redirections == NULL) {
    free_ast((ASTNode *)cmd);
    // Don't print syntax error for empty commands
    return NULL;
  }
  return (ASTNode *)cmd;
}

static ASTNode *parse_job() {
  ASTNode *node = parse_command();
  if (!node)
    return NULL;

  skip_whitespace();
  if (peek("&")) {
    get_token();
    ((CommandNode *)node)->background = 1;
  }
  return node;
}

static ASTNode *parse_pipe() {
  ASTNode *left = parse_job();
  if (!left)
    return NULL;

  skip_whitespace();
  if (peek("|")) {
    get_token();
    ASTNode *right = parse_pipe();
    if (!right) {
      fprintf(stderr, "Invalid Syntax!\n");
      free_ast(left);
      return NULL;
    }
    PipeNode *pipe_node = malloc(sizeof(PipeNode));
    pipe_node->type = NODE_PIPE;
    pipe_node->left = left;
    pipe_node->right = right;
    return (ASTNode *)pipe_node;
  }
  return left;
}

static ASTNode *parse_sequence() {
  ASTNode *left = parse_pipe();
  if (!left)
    return NULL;

  skip_whitespace();
  if (peek(";")) {
    get_token();
    ASTNode *right = parse_sequence();
    if (!right) {
      // Allow trailing semicolon
      return left;
    }
    SequenceNode *seq_node = malloc(sizeof(SequenceNode));
    seq_node->type = NODE_SEQUENCE;
    seq_node->left = left;
    seq_node->right = right;
    return (ASTNode *)seq_node;
  }
  return left;
}

ASTNode *parse_input(const char *input) {
  g_input_stream = input;
  ASTNode *ast = parse_sequence();
  skip_whitespace();
  if (*g_input_stream != '\0') {
    fprintf(stderr, "Invalid Syntax!\n");
    free_ast(ast);
    return NULL;
  }
  return ast;
}

void free_ast(ASTNode *node) {
  if (!node)
    return;
  switch (node->type) {
  case NODE_COMMAND: {
    CommandNode *cmd = (CommandNode *)node;
    for (int i = 0; i < cmd->arg_count; ++i)
      free(cmd->args[i]);
    free(cmd->args);
    Redirection *r = cmd->redirections;
    while (r) {
      Redirection *next = r->next;
      free(r->filename);
      free(r);
      r = next;
    }
    break;
  }
  case NODE_PIPE:
  case NODE_SEQUENCE: {
    PipeNode *p = (PipeNode *)node;
    free_ast(p->left);
    free_ast(p->right);
    break;
  }
  }
  free(node);
}
