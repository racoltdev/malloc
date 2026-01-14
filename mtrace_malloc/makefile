# mtrace library build tool
#	This file is part of the mtrace_malloc library.
#
#	This library is free software; you can redistribute it and/or
#	modify it under the terms of the GNU Lesser General Public
#	License as published by the Free Software Foundation; either
#	version 2.1 of the License, or (at your option) any later version.
#
#	The mtrace_malloc library is distributed in the hope that it will be
#	useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#	Lesser General Public License for more details.
#
#	You should have received a copy of the GNU Lesser General Public
#	License along with the mtrace_malloc library; if not, see
#	<https://www.gnu.org/licenses/>.

TARGET := mtrace_malloc
LIB := lib$(TARGET)

BUILD_DIR := build/
SOURCE_DIR := ./
INC_DIR := include/

CXXFLAGS := -g3 -Wall -Wextra -Wpedantic -Wconversion -Wno-missing-field-initializers -fPIC
CXXFLAGS += -MMD -MP -I$(SOURCE_DIR) -I$(INC_DIR)
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
