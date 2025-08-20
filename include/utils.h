// Copyright (c) 2025 Pranshul Shenoy. All Rights Reserved.

#ifndef H_INCLUDE_UTILS
#define H_INCLUDE_UTILS

// Alias utility functions
void load_aliases(void);
void save_aliases(void);
char *get_alias_command(const char *name);
void add_alias(const char *name, const char *command);

// History utility functions
void load_history(void);
void save_history(void);
void add_to_history(const char *command);

// Raw input utility function
char *read_line_raw(const char *prompt);

#endif
