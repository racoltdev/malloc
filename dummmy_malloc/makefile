TARGET := dummy_malloc
LIB := lib_$(TARGET)

BUILD_DIR := build/
SOURCE_DIR := ./

CXXFLAGS := -g3 -Wall -Wextra -Wpedantic -Wconversion -Wno-missing-field-initializers -fPIC
CXXFLAGS += -MMD -MP -I$(SOURCE_DIR)
LD_FLAGS := -shared

THIS_MAKEFULE := $(firstword $(MAKEFILE_LIST))
SOURCES != find $(SOURCE_DIR) -name '*.c' ! -wholename 'src/main.c'
OBJECTS := ${SOURCES:%.c=$(BUILD_DIR)%.o}

$(BUILD_DIR)%.o: %.c $(THIS_MAKEFILE)
	@mkdir -p "${@D}"
	@echo "[CXX] ${@F}"
	@$(CXX) $(CXXFLAGS) -c "$<" -o "$@"

$(TARGET): $(OBJECTS) $(BUILD_DIR)$(TARGET).o
	@echo "[LD] ${@F}"

$(LIB): $(OBJECTS)
	@echo "[SO] ${@F}"
	@$(CXX) $(LD_FLAGS) $^ -o $(BUILD_DIR)$@.so

all: $(TARGET) $(LIB)
	@cp $(BUILD_DIR)$(LIB).so ./

clean:
	rm -rf $(BUILD_DIR)
