# Compiler and flags
CC = gcc
CFLAGS = -Wall

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# Executables
BINARIES = receptionist visitor monitor

# Object files
OBJS = $(BUILD_DIR)/shared_memory.o

# Shared memory header
HEADERS = $(INCLUDE_DIR)/shared_memory.h

# Default target
all: $(BINARIES)

# Build rules for executables
receptionist: $(BUILD_DIR)/receptionist.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

visitor: $(BUILD_DIR)/visitor.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

monitor: $(BUILD_DIR)/monitor.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Clean rule
clean:
	rm -rf $(BUILD_DIR) $(BINARIES)

# PHONY targets
.PHONY: all clean
