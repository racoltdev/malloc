TARGET := mtrace_malloc
LIB := lib$(TARGET)

BUILD_DIR := build/
SOURCE_DIR := ./

CXXFLAGS := -g3 -Wall -Wextra -Wpedantic -Wconversion -Wno-missing-field-initializers -fPIC
CXXFLAGS += -MMD -MP -I$(SOURCE_DIR)
LD_FLAGS := -shared -ldl

THIS_MAKEFULE := $(firstword $(MAKEFILE_LIST))
SOURCES != find $(SOURCE_DIR) -name '*.c'
OBJECTS := ${SOURCES:%.c=$(BUILD_DIR)%.o}

$(BUILD_DIR)%.o: %.c $(THIS_MAKEFILE)
	@mkdir -p "${@D}"
	@echo "[CXX] ${@F}"
	@$(CC) $(CXXFLAGS) -c "$<" -o "$@"

$(TARGET): $(OBJECTS) $(BUILD_DIR)$(TARGET).o
	@echo "[LD] ${@F}"

$(LIB): $(OBJECTS)
	@echo "[SO] ${@F}"
	@$(CC) $(LD_FLAGS) $^ -o $(BUILD_DIR)$@.so

all: $(TARGET) $(LIB)
	@cp $(BUILD_DIR)$(LIB).so ./

clean:
	rm -rf $(BUILD_DIR)
