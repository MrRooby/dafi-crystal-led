#include <stdint.h>

/* STM8L051F3 Register Map */
#define CLK_PCKENR1  (*(volatile uint8_t *)0x50C7)
#define PA_ODR       (*(volatile uint8_t *)0x5000)
#define PA_DDR       (*(volatile uint8_t *)0x5002)
#define PA_CR1       (*(volatile uint8_t *)0x5003)
#define PB_ODR       (*(volatile uint8_t *)0x5005)
#define PB_DDR       (*(volatile uint8_t *)0x5007)
#define PB_CR1       (*(volatile uint8_t *)0x5008)

#define SPI1_CR1     (*(volatile uint8_t *)0x5200)
#define SPI1_CR2     (*(volatile uint8_t *)0x5201)
#define SPI1_SR      (*(volatile uint8_t *)0x5203)
#define SPI1_DR      (*(volatile uint8_t *)0x5204)

/* LIS2DW12 Registers */
#define LIS2_WHO_AM_I 0x0F
#define LIS2_CTRL1    0x20
#define LIS2_OUT_T_L  0x26
#define LIS2_OUT_T_H  0x27

void delay(uint32_t n) {
    while (n--) { __asm__("nop"); }
}

uint8_t spi_xfer(uint8_t data) {
    SPI1_DR = data;
    while (!(SPI1_SR & 0x02)); // Wait for TXE (Transmit buffer empty)
    while (!(SPI1_SR & 0x01)); // Wait for RXNE (Receive buffer not empty)
    return SPI1_DR;
}

uint8_t read_reg(uint8_t reg) {
    uint8_t val;
    PB_ODR &= ~(1 << 4);        // CS LOW
    spi_xfer(reg | 0x80);       // Bit 7 is 1 for Read
    val = spi_xfer(0x00);       // Dummy byte to get data
    PB_ODR |= (1 << 4);         // CS HIGH
    return val;
}

void write_reg(uint8_t reg, uint8_t val) {
    PB_ODR &= ~(1 << 4);        // CS LOW
    spi_xfer(reg & 0x7F);       // Bit 7 is 0 for Write
    spi_xfer(val);
    PB_ODR |= (1 << 4);         // CS HIGH
}

void main(void) {
    // 1. Enable SPI Peripheral Clock
    CLK_PCKENR1 |= (1 << 4);

    // 2. Setup PA3 (LED) as Output Push-Pull
    PA_DDR |= (1 << 3);
    PA_CR1 |= (1 << 3);
    
    // 3. Setup PB4(CS), PB5(SCK), PB6(MOSI)
    // Note: PB7(MISO) is automatically handled by SPI as Input
    PB_DDR |= (1 << 4) | (1 << 5) | (1 << 6);
    PB_CR1 |= (1 << 4) | (1 << 5) | (1 << 6);
    PB_ODR |= (1 << 4); // CS High

    // 4. SPI Init
    // BR[2:0] = 100 (fMaster/32 = 500kHz @ 16MHz) - Start slow for stability
    // MSTR = 1, SPE = 1, Mode 0 (CPOL=0, CPHA=0)
    SPI1_CR1 = (1 << 2) | (0x04 << 3) | (1 << 6);
    SPI1_CR2 = (1 << 1) | (1 << 0); // SSM = 1, SSI = 1

    delay(200000); // Wait for sensor to wake up (~20ms)

    // 5. Sensor Configuration
    // Set CTRL1 to 0x44 (ODR 200Hz, High Performance, Low Noise)
    // This activates both the Accelerometer and the Temperature sensor
    write_reg(LIS2_CTRL1, 0x44);

    while (1) {
        // Read Temperature High Byte
        uint8_t temp = read_reg(LIS2_OUT_T_H);

        // If 'temp' is not 0 (typical ambient is ~0x10 to 0x1A), 
        // it proves we are getting valid SPI data.
        if (temp != 0x00 && temp != 0xFF) {
            PA_ODR |= (1 << 3);  // Success!
        } else {
            PA_ODR &= ~(1 << 3); // Fail
        }
        
        delay(100000);
    }
}
