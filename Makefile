# Compiler
CC = gcc

# Compiler Flags
CFLAGS = -Wall -Wextra -std=c99 -g

# Source Files
SRCS = county_analysis.c

# Output Executable
TARGET = demographics

# Build Target
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Clean Up
clean:
	rm -f $(TARGET) *.o