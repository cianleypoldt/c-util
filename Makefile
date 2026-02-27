# Compiler / tools
CC      := gcc
AR      := ar
CFLAGS  := -Wall -Wextra -O2 -MMD -MP
ARFLAGS := rcs
LDLIBS  := -lm

# Directories
SRC_DIR    := .
BUILD_DIR  := build
TEST_DIR   := tests

# Sources and objects
SRC := $(SRC_DIR)/slotmap.c $(SRC_DIR)/freelist.c $(SRC_DIR)/dynamic_array.c
OBJ := $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SRC)))
DEP := $(OBJ:.o=.d)
LIB := $(BUILD_DIR)/libutil.a

# Test
TEST_SRC := $(TEST_DIR)/test_array.c
TEST_BIN := $(BUILD_DIR)/test_array.t

.PHONY: all clean test compile_commands

# Default target
all: $(LIB)

# Build static library
$(LIB): $(OBJ)
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $^

# Compile objects
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Build and link test
$(TEST_BIN): $(TEST_SRC) $(LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -I$(SRC_DIR) $(LIB) $(LDLIBS) -o $@

# Run test
test: $(TEST_BIN)
	./$(TEST_BIN)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Optional: regenerate compile_commands.json using Bear
compile_commands:
	if command -v bear >/dev/null 2>&1; then \
		bear --output=compile_commands.json -- $(MAKE) -B all; \
	else \
		echo "Bear not found; cannot generate compile_commands.json"; \
	fi

# Include dependencies
-include $(DEP)
