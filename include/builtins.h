// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#ifndef H_INCLUDE_BUILTINS
#define H_INCLUDE_BUILTINS

// Forward declaration
typedef struct SimpleCommand SimpleCommand;

// Built-in function prototypes
int is_builtin(const char *cmd_name);
void execute_builtin(SimpleCommand *cmd);
void hop(SimpleCommand *cmd);
void reveal(SimpleCommand *cmd);
void alias_cmd(SimpleCommand *cmd);
void log_cmd(SimpleCommand *cmd);
void ping(SimpleCommand *cmd);
void activities(SimpleCommand *cmd);
void fg(SimpleCommand *cmd);
void bg(SimpleCommand *cmd);

#endif // H_INCLUDE_BUILTINS
