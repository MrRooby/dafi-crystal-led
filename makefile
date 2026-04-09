# Project Settings
TARGET = test.ihx
MCU    = stm8
DEVICE = STM8L15X_LD

# Paths
LIB_DIR = lib/STM8L15x_StdPeriph_Driver
INC_DIR = $(LIB_DIR)/inc
SRC_DIR = $(LIB_DIR)/src

# Compiler and Flags
CC      = sdcc-sdcc
CFLAGS  = -m$(MCU) -DSDCC -D$(DEVICE) -I. -I$(INC_DIR)

# Core Source Files
# Add only the peripheral files you actually use here to save time/space
SOURCES = main.c \
          $(SRC_DIR)/stm8l15x_clk.c \
          $(SRC_DIR)/stm8l15x_gpio.c

# Generate list of object files (.rel) in the current directory
OBJECTS = $(notdir $(SOURCES:.c=.rel))

# Default Rule
all: $(TARGET)

# Link everything
$(TARGET): $(OBJECTS)
	$(CC) -m$(MCU) $(OBJECTS) -o $(TARGET)

# Compile main.c
main.rel: main.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile Library Files (Pattern Rule)
# This handles files located in the library src folder
%.rel: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean Up
clean:
	rm -f *.rel *.asm *.lst *.rst *.sym *.cdb *.map *.lk
