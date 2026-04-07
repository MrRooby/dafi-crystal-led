# Compiler and programmer
CC = sdcc
FLASH = stm8flash

# MCU and programmer
MCU = stm8l051f3
PROG = stlinkv2

# Source and target
SRCS = main.c
TARGET = main

# Compiler flags
CFLAGS = -mstm8

all: $(TARGET).ihx

%.ihx: %.c
	$(CC) $(CFLAGS) -o $@ $<

flash: $(TARGET).ihx
	$(FLASH) -c $(PROG) -p $(MCU) -w $<

clean:
	rm -f *.ihx *.asm *.lst *.lk *.map *.rel
