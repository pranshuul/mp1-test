#ifndef PARSER_H
#define PARSER_H

#include "shell.h"

// Enum for AST node types
typedef enum {
  NODE_COMMAND,
  NODE_PIPE,
  NODE_SEQUENCE,
} NodeType;

// Enum for redirection types
typedef enum {
  REDIR_NONE,
  REDIR_IN,     // <
  REDIR_OUT,    // >
  REDIR_APPEND, // >>
  REDIR_HEREDOC // <<
} RedirType;

// Base struct for all AST nodes
typedef struct ASTNode {
  NodeType type;
} ASTNode;

// Struct for a redirection
typedef struct Redirection {
  RedirType type;
  char *filename;
  struct Redirection *next;
} Redirection;

// AST node for a single command
typedef struct {
  NodeType type;
  char **args;
  int arg_count;
  Redirection *redirections;
  int background; // 1 if background (&), 0 otherwise
} CommandNode;

// AST node for a pipe
typedef struct {
  NodeType type;
  ASTNode *left;
  ASTNode *right;
} PipeNode;

// AST node for a sequence
typedef struct {
  NodeType type;
  ASTNode *left;
  ASTNode *right;
} SequenceNode;

// Function prototypes
ASTNode *parse_input(const char *input);
void free_ast(ASTNode *node);

#endif // PARSER_H
