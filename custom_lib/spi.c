#include "spi.h"

/**
 * Initializes SPI1 for LIS2DW12:
 * Mode 3 (CPOL=1, CPHA=1), MSB First, Master Mode, Baud Rate ~2MHz
 */
void spi_init(void) {
    // 1. Enable SPI1 Peripheral Clock (PCKEN14)
    CLK->PCKENR1 |= (1 << 4);

    // 2. Ensure SPI is disabled before configuration
    B_SPI1_CR1 = 0x00;

    /**
     * CR1 Configuration:
     * Bit 7: LSBFIRST = 0 (MSB First)
     * Bit 6: SPE = 0 (Enabled later)
     * Bits 5-3: BR[2:0] = 010 (f_SYS / 8 = 2MHz if f_SYS is 16MHz)
     * Bit 2: MSTR = 1 (Master Mode)
     * Bit 1: CPOL = 1 (Clock Idles High)
     * Bit 0: CPHA = 1 (Data captured on 2nd edge / Rising edge)
     */
    B_SPI1_CR1 = (0x02 << 3) | (1 << 2) | (1 << 1) | (1 << 0);

    /**
     * CR2 Configuration (Critical for Master Mode):
     * Bit 1: SSM = 1 (Software Slave Management)
     * Bit 0: SSI = 1 (Internal Slave Select High - prevents Mode Fault)
     */
    B_SPI1_CR2 = (1 << 1) | (1 << 0);

    // 3. Enable SPI
    B_SPI1_CR1 |= (1 << 6); // SPE = 1
}

/**
 * Full-duplex byte exchange
 */
uint8_t spi_transfer(uint8_t data) {
    // Wait until TX buffer is empty
    while (!(B_SPI1_SR & SPI_SR_TXE));
    
    // Write data to be sent
    B_SPI1_DR = data;
    
    // Wait until RX buffer is not empty
    while (!(B_SPI1_SR & SPI_SR_RXNE));
    
    // Read and return received data
    return B_SPI1_DR;
}

/**
 * Reads a single register from LIS2DW12
 * SPI Header: [Read_Bit(1)][Register Address(6:0)]
 */
uint8_t lis_read_reg(uint8_t reg) {
    uint8_t value;
    
    // CS Low (Assuming PB4 is CS)
    GPIOB->ODR &= ~(1 << 4); 

    // Send address with READ bit (Bit 7 set)
    spi_transfer(reg | 0x80);
    
    // Clock in the data from the sensor
    value = spi_transfer(0x00);

    // CS High
    GPIOB->ODR |= (1 << 4); 
    
    return value;
}

/**
 * Writes to a register
 * SPI Header: [Write_Bit(0)][Register Address(6:0)]
 */
void lis_write_reg(uint8_t reg, uint8_t data) {
    GPIOB->ODR &= ~(1 << 4); 

    spi_transfer(reg & 0x7F); // Write bit is 0
    spi_transfer(data);

    GPIOB->ODR |= (1 << 4); 
}
