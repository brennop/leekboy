# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -Wall -Werror -std=c99 -Iinclude `sdl2-config --cflags`

# Linker flags
LDFLAGS = `sdl2-config --libs`

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Files
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

# Targets
TARGET = $(BIN_DIR)/leekboy

# Phony targets
.PHONY: all clean

# Default target
all: $(TARGET)

# Main target
$(TARGET): $(OBJ_FILES)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

# Object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Run target
run: $(TARGET)
	./$(TARGET)
