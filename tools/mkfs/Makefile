#
# Makefile
# Created by Matheus Leme Da Silva
#

TARGET?=mkfs.bfx

CC=gcc
CFLAGS=-Wall -Wextra -Werror -O3

.PHONY: all clean

all: $(TARGET)

$(TARGET): mkfs.bfx.c
	@echo "  CC $@"
	@mkdir -p $(dir $(TARGET))
	@$(CC) $(CFLAGS) -o $(TARGET) mkfs.bfx.c
	@chmod 755 $(TARGET)

clean:
	@echo "Cleaning..."
	@rm -f $(TARGET)

