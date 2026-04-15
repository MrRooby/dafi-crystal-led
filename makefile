# Project Settings
TARGET = serial.ihx
MCU    = stm8
DEVICE = STM8L15X_LD

# Paths
LIB_DIR    = lib/STM8L15x_StdPeriph_Driver
INC_DIR    = $(LIB_DIR)/inc
SRC_DIR    = $(LIB_DIR)/src
CUSTOM_LIB = custom_lib 

# Compiler and Flags
CC      = sdcc-sdcc
# -I. includes current dir, others include library headers and your custom headers
CFLAGS  = -m$(MCU) -DSDCC -D$(DEVICE) -I. -I$(INC_DIR) -I$(CUSTOM_LIB)

# Search paths for source files
# This tells make where to look for .c files automatically
VPATH = $(SRC_DIR):$(CUSTOM_LIB)

# Core Source Files
# List the .c files you need. 
# Note: We don't need the full path here because of VPATH
SOURCES = main.c \
          stm8l15x_clk.c \
          stm8l15x_adc.c \
          stm8l15x_gpio.c \
          stm8l15x_spi.c \
          serial.c

# Generate list of object files (.rel)
# This strips the directory and changes .c to .rel
OBJECTS = $(notdir $(SOURCES:.c=.rel))

# Default Rule
all: $(TARGET)

# Link everything
$(TARGET): $(OBJECTS)
	$(CC) -m$(MCU) $(OBJECTS) -o $(TARGET)

# General rule to compile ANY .rel from a .c file
# This handles main.c, custom_lib/*.c, and the StdPeriph/*.c
%.rel: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean Up
.PHONY: clean
clean:
	rm -f *.rel *.asm *.lst *.rst *.sym *.cdb *.map *.lk
