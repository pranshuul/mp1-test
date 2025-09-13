# Compiler and flags
CC = gcc
CFLAGS = -std=c99 -Wall -Werror -fsanitize=address,undefined -Iinclude -D_POSIX_C_SOURCE=200809L
LDFLAGS = -fsanitize=address,undefined

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = .

# Source files and object files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))
EXECUTABLE = $(BINDIR)/shell.out

# Default target
all: $(EXECUTABLE)

# Link the executable
$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Shell executable created at $(EXECUTABLE)"

# Compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	@rm -rf $(OBJDIR) $(EXECUTABLE)
	@echo "Cleaned up build artifacts."

# Phony targets
.PHONY: all clean
