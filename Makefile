# Compiler and flags
CC = gcc
CFLAGS = #-Wall

# Debug flags
DEBUG_CFLAGS = -g

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Common source files
COMMON_SRCS = $(wildcard $(SRC_DIR)/common/*.c)
COMMON_OBJS = $(patsubst $(SRC_DIR)/common/%.c, $(OBJ_DIR)/common/%.o, $(COMMON_SRCS))
DEBUG_OBJS = $(patsubst $(SRC_DIR)/common/%.c, $(OBJ_DIR)/common/%_debug.o, $(COMMON_SRCS))

# Source files for executables in each subdirectory of src
EXE_SRCS = $(wildcard $(SRC_DIR)/*/main.c)
EXE_DIRS = $(dir $(EXE_SRCS))
EXE_NAMES = $(patsubst $(SRC_DIR)/%/main.c, $(BIN_DIR)/%, $(EXE_SRCS))

# Debug executables
DEBUG_EXE_NAMES = $(addsuffix _debug, $(EXE_NAMES))

# Build all executables
all: $(EXE_NAMES)

debug: $(DEBUG_EXE_NAMES)

# Build each executable
$(BIN_DIR)/%: $(SRC_DIR)/%/main.c $(COMMON_OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^

# Build each debug executable
$(BIN_DIR)/%_debug: $(SRC_DIR)/%/main.c $(DEBUG_OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -o $@ $^

# Build common object files
$(OBJ_DIR)/common/%.o: $(SRC_DIR)/common/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Build debug common object files
$(OBJ_DIR)/common/%_debug.o: $(SRC_DIR)/common/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean debug
