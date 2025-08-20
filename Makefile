CC = gcc
# Non-negotiable CFLAGS as provided
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE -g -Wall -Wextra -Werror -Wno-unused-parameter -Iinclude -fno-asm

SRCDIR = src

CORE_SRC = $(wildcard $(SRCDIR)/core/*.c)
BUILTINS_SRC = $(wildcard $(SRCDIR)/builtins/*.c)
UTILS_SRC = $(wildcard $(SRCDIR)/utils/*.c)

SRC = $(CORE_SRC) $(BUILTINS_SRC) $(UTILS_SRC)
OBJ = $(SRC:.c=.o)

TARGET = shell.out

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) .shell_history .shell_log .aliases
