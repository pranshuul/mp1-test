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

Tokenizer tokenizer;

// --- Tokenizer ---
static void init_tokenizer(const char *source) {
  tokenizer.start = source;
  tokenizer.current = source;
}

static Token make_token(TokenType type) {
  Token token;
  token.type = type;
  token.start = tokenizer.start;
  token.length = (int)(tokenizer.current - tokenizer.start);
  return token;
}

static Token error_token(const char *message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = strlen(message);
  return token;
}

static void skip_whitespace() {
  while (*tokenizer.current == ' ' || *tokenizer.current == '\t' ||
         *tokenizer.current == '\r' || *tokenizer.current == '\n') {
    tokenizer.current++;
  }
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
  default: {
    // Handle quoted strings
    if (c == '"' || c == '\'') {
      char quote = c;
      tokenizer.start++; // Skip opening quote
      while (*tokenizer.current != quote && *tokenizer.current != '\0') {
        tokenizer.current++;
      }
      if (*tokenizer.current == quote) {
        Token t = make_token(TOKEN_WORD);
        tokenizer.current++; // Skip closing quote
        return t;
      } else {
        return error_token("Unterminated string.");
      }
    }
    // Handle regular words
    while (*tokenizer.current != '\0' &&
           !strchr(" \t\r\n|&;<>()", *tokenizer.current)) {
      tokenizer.current++;
    }
    return make_token(TOKEN_WORD);
  }
  }
}

// --- Parser ---
static SimpleCommand *parse_atomic();
static Pipeline *parse_pipeline();

static Token current_token;
static int has_error = 0;

static void advance() { current_token = scan_token(); }
static int match(TokenType type) {
  if (current_token.type == type) {
    advance();
    return 1;
  }
  return 0;
}
static int check(TokenType type) { return current_token.type == type; }

char *token_to_str(Token t) {
  char *s = malloc(t.length + 1);
  strncpy(s, t.start, t.length);
  s[t.length] = '\0';
  return s;
}

SimpleCommand *parse_atomic() {
  SimpleCommand *sc = calloc(1, sizeof(SimpleCommand));
  int argc = 0;

  while (check(TOKEN_WORD) || check(TOKEN_LT) || check(TOKEN_GT) ||
         check(TOKEN_GTGT)) {
    if (match(TOKEN_LT)) {
      if (match(TOKEN_WORD)) {
        sc->inputFile = token_to_str(current_token); // The word after '<'
      } else {
        has_error = 1;
        fprintf(stderr, "syntax error: expected filename after '<'\n");
      }
    } else if (match(TOKEN_GT)) {
      if (match(TOKEN_WORD)) {
        sc->outputFile = token_to_str(current_token);
      } else {
        has_error = 1;
        fprintf(stderr, "syntax error: expected filename after '>'\n");
      }
    } else if (match(TOKEN_GTGT)) {
      if (match(TOKEN_WORD)) {
        sc->outputFile = token_to_str(current_token);
        sc->append = 1;
      } else {
        has_error = 1;
        fprintf(stderr, "syntax error: expected filename after '>>'\n");
      }
    } else if (match(TOKEN_WORD)) {
      if (argc < MAX_ARGS - 1) {
        sc->args[argc++] = token_to_str(current_token);
      }
    } else {
      break;
    }
    advance();
  }
  sc->args[argc] = NULL;
  return sc;
}

Pipeline *parse_pipeline() {
  Pipeline *p = calloc(1, sizeof(Pipeline));
  p->commands =
      malloc(sizeof(SimpleCommand *) * 10); // Start with capacity for 10
  p->num_commands = 0;

  p->commands[p->num_commands++] = parse_atomic();

  while (match(TOKEN_PIPE)) {
    p->commands[p->num_commands++] = parse_atomic();
  }

  return p;
}

Pipeline **parse_line(const char *line, int *pipeline_count) {
  init_tokenizer(line);
  advance();
  has_error = 0;

  Pipeline **pipelines = malloc(sizeof(Pipeline *) * 20);
  int count = 0;

  // const char *line_start = line;

  while (!check(TOKEN_EOF) && !has_error) {
    const char *pipeline_start = tokenizer.current;
    pipelines[count] = parse_pipeline();

    if (match(TOKEN_AMPERSAND)) {
      pipelines[count]->background = 1;
    } else if (match(TOKEN_SEMICOLON)) {
      // Sequential, do nothing special
    }

    const char *pipeline_end = tokenizer.current;
    int len = pipeline_end - pipeline_start;
    pipelines[count]->full_text = strndup(pipeline_start, len);

    count++;
  }

  *pipeline_count = count;
  return pipelines;
}

// --- Memory Management ---
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
