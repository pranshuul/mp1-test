// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#ifndef H_INCLUDE_UTILS
#define H_INCLUDE_UTILS

// Alias utilities
void load_aliases(void);
void save_aliases(void);
char *get_alias_command(const char *name);
void add_alias(char *name, char *command);

// History utilities
void load_history(void);
void save_history(void);
void add_to_history(const char *command);

// Input utilities
char *read_line_raw(const char *prompt);
void enable_raw_mode(void);
void disable_raw_mode(void);

#endif // H_INCLUDE_UTILS
