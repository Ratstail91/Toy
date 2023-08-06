CC = gcc
CFLAGS = -Wall -Wextra -pedantic

# Library name and source files
LIB_NAME = hal.a
SRC_FILES = ports/hal_file.c
BUILD_DIR = bin

# Object files derived from source files
OBJ_FILES = $(SRC_FILES:.c=.o)

# Target: build the library
all: $(BUILD_DIR)/$(LIB_NAME)

# Build the library from object files
$(LIB_NAME): $(OBJ_FILES)
	ar rcs $@ $^

# Compile from object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(LIB_NAME) $(OBJ_FILES)