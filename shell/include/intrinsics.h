#ifndef INTRINSICS_H
#define INTRINSICS_H

#include "parser.h"
#include "shell.h"

// Type definition for a built-in command function
typedef int (*builtin_func)(char **args);

// Struct to map command names to functions
typedef struct {
  const char *name;
  builtin_func func;
} BuiltinCommand;

// Main handler to find and execute a built-in
int handle_builtin(CommandNode *cmd);

// Built-in command implementations
int builtin_hop(char **args);
int builtin_reveal(char **args);
int builtin_log(char **args);
int builtin_ping(char **args);
int builtin_activities(char **args);
int builtin_fg(char **args);
int builtin_bg(char **args);

// History management functions
void init_history(void);
void add_to_history(const char *command);
void clear_history(void);
void display_history(void);
int save_history(const char *filename);

#endif // INTRINSICS_H
