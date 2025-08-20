// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#include "../../include/core.h"

// Simple tokenizer state
typedef struct {
  const char *start;
  const char *current;
} Tokenizer;
typedef enum {
  TOKEN_WORD,
  TOKEN_PIPE,
  TOKEN_AMPERSAND,
  TOKEN_SEMICOLON,
  TOKEN_LT,
  TOKEN_GT,
  TOKEN_GTGT,
  TOKEN_EOF,
  TOKEN_ERROR
} TokenType;
typedef struct {
  TokenType type;
  const char *start;
  int length;
} Token;

static Tokenizer tokenizer;
static Token current_token;
static int has_error = 0;

static void advance();

// --- Memory Management (CRITICAL TO PREVENT LEAKS) ---
void free_simple_command(SimpleCommand *sc) {
  if (!sc)
    return;
  for (int i = 0; sc->args[i]; i++)
    free(sc->args[i]);
  free(sc->inputFile);
  free(sc->outputFile);
  free(sc);
}

void free_pipeline(Pipeline *p) {
  if (!p)
    return;
  for (int i = 0; i < p->num_commands; i++) {
    free_simple_command(p->commands[i]);
  }
  free(p->commands);
  free(p->full_text);
  free(p);
}

// --- Tokenizer Implementation ---
static void init_tokenizer(const char *source) {
  tokenizer.start = source;
  tokenizer.current = source;
}
static Token make_token(TokenType type) {
  return (Token){type, tokenizer.start,
                 (int)(tokenizer.current - tokenizer.start)};
}
static void skip_whitespace() {
  while (*tokenizer.current == ' ' || *tokenizer.current == '\t')
    tokenizer.current++;
}

static Token scan_token() {
  skip_whitespace();
  tokenizer.start = tokenizer.current;
  if (*tokenizer.current == '\0')
    return make_token(TOKEN_EOF);
  char c = *tokenizer.current++;
  switch (c) {
  case '|':
    return make_token(TOKEN_PIPE);
  case '&':
    return make_token(TOKEN_AMPERSAND);
  case ';':
    return make_token(TOKEN_SEMICOLON);
  case '<':
    return make_token(TOKEN_LT);
  case '>':
    if (*tokenizer.current == '>') {
      tokenizer.current++;
      return make_token(TOKEN_GTGT);
    }
    return make_token(TOKEN_GT);
  default:
    if (c == '"' || c == '\'') {
      tokenizer.start++;
      while (*tokenizer.current != c && *tokenizer.current != '\0')
        tokenizer.current++;
      Token t = make_token(TOKEN_WORD);
      if (*tokenizer.current == c)
        tokenizer.current++;
      return t;
    }
    while (*tokenizer.current && !strchr(" \t|&;<>()", *tokenizer.current))
      tokenizer.current++;
    return make_token(TOKEN_WORD);
  }
}

// --- Parser Implementation ---
static char *token_to_str() {
  char *s = malloc(current_token.length + 1);
  if (!s)
    return NULL;
  strncpy(s, current_token.start, current_token.length);
  s[current_token.length] = '\0';
  return s;
}

static SimpleCommand *parse_atomic() {
  SimpleCommand *sc = calloc(1, sizeof(SimpleCommand));
  int argc = 0;
  while (current_token.type == TOKEN_WORD || current_token.type == TOKEN_LT ||
         current_token.type == TOKEN_GT || current_token.type == TOKEN_GTGT) {
    if (current_token.type == TOKEN_LT) {
      advance();
      if (current_token.type == TOKEN_WORD)
        sc->inputFile = token_to_str();
      else
        has_error = 1;
    } else if (current_token.type == TOKEN_GT) {
      advance();
      if (current_token.type == TOKEN_WORD)
        sc->outputFile = token_to_str();
      else
        has_error = 1;
    } else if (current_token.type == TOKEN_GTGT) {
      advance();
      if (current_token.type == TOKEN_WORD) {
        sc->outputFile = token_to_str();
        sc->append = 1;
      } else
        has_error = 1;
    } else if (current_token.type == TOKEN_WORD) {
      if (argc < MAX_ARGS - 1)
        sc->args[argc++] = token_to_str();
    }
    advance();
  }
  return sc;
}

static Pipeline *parse_pipeline() {
  Pipeline *p = calloc(1, sizeof(Pipeline));
  p->commands = malloc(sizeof(SimpleCommand *) * 10);
  p->commands[p->num_commands++] = parse_atomic();
  while (current_token.type == TOKEN_PIPE) {
    advance();
    p->commands[p->num_commands++] = parse_atomic();
  }
  return p;
}

static void advance() { current_token = scan_token(); }

Pipeline **parse_line(const char *line, int *pipeline_count) {
  init_tokenizer(line);
  advance();
  has_error = 0;
  Pipeline **pipelines = malloc(sizeof(Pipeline *) * 20);
  int count = 0;
  // const char *line_start = line;
  while (current_token.type != TOKEN_EOF && !has_error) {
    const char *pipeline_start = tokenizer.start;
    pipelines[count] = parse_pipeline();
    if (current_token.type == TOKEN_AMPERSAND) {
      pipelines[count]->background = 1;
      advance();
    }
    if (current_token.type == TOKEN_SEMICOLON)
      advance();
    const char *pipeline_end = tokenizer.start;
    pipelines[count]->full_text =
        strndup(pipeline_start, pipeline_end - pipeline_start);
    count++;
  }
  *pipeline_count = count;
  return pipelines;
}
