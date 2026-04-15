#ifndef __SPI_H
#define __SPI_H

#include "stm8l15x.h"

// SPI1 Register Map Addresses
#define SPI1_BASE_ADDR 0x5200
#define B_SPI1_CR1    (*(volatile uint8_t *)(SPI1_BASE_ADDR + 0x00))
#define B_SPI1_CR2    (*(volatile uint8_t *)(SPI1_BASE_ADDR + 0x01))
#define B_SPI1_ICR    (*(volatile uint8_t *)(SPI1_BASE_ADDR + 0x02))
#define B_SPI1_SR     (*(volatile uint8_t *)(SPI1_BASE_ADDR + 0x03))
#define B_SPI1_DR     (*(volatile uint8_t *)(SPI1_BASE_ADDR + 0x04))

// Status Register Bits
#define SPI_SR_BSY     (1 << 7)
#define SPI_SR_TXE     (1 << 1)
#define SPI_SR_RXNE    (1 << 0)

// Function Prototypes
void spi_init(void);
uint8_t spi_transfer(uint8_t data);
uint8_t lis_read_reg(uint8_t reg);
void lis_write_reg(uint8_t reg, uint8_t data);

#endif
