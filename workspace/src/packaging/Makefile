MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
ROOT_DIR := $(abspath $(MAKEFILE_DIR)/../../..)
SRC_ROOT := $(abspath $(MAKEFILE_DIR)/..)

CC := x86_64-w64-mingw32-gcc
TARGET := $(MAKEFILE_DIR)samsung-fwloader.exe
OBJ_DIR := $(MAKEFILE_DIR)build
DIST_DIR := $(MAKEFILE_DIR)dist

SOURCES := \
	$(SRC_ROOT)/cli_interface/main.c \
	$(SRC_ROOT)/state_machine/fwloader.c \
	$(SRC_ROOT)/usb_layer/usb_win.c \
	$(SRC_ROOT)/firmware_parser/firmware_parser.c

OBJECTS := $(patsubst $(SRC_ROOT)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

CPPFLAGS := -I$(ROOT_DIR)/include
CFLAGS := -std=c11 -O2 -Wall -Wextra -pedantic
LDFLAGS := -static-libgcc -L$(ROOT_DIR)/lib
LDLIBS := -lusb-1.0 -lsetupapi

.PHONY: all clean dist

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

dist: $(TARGET)
	@mkdir -p $(DIST_DIR)
	cp $(TARGET) $(DIST_DIR)/
	cp $(ROOT_DIR)/lib/libusb-1.0.dll $(DIST_DIR)/
	cp $(ROOT_DIR)/INSTALL.md $(DIST_DIR)/

clean:
	rm -rf $(OBJ_DIR) $(DIST_DIR) $(TARGET)
