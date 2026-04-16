#include "stm8l15x.h"
#include "stm8l15x_clk.h"
#include "stm8l15x_gpio.h"
#include "stm8l15x_spi.h"
#include <stdio.h>
#include <stdbool.h>
#include "serial.h"

// On LED
#define RED_PORT GPIOA
#define RED_PIN GPIO_Pin_3

// SPI - Accelerometer
#define CS_PORT GPIOB
#define CS_PIN GPIO_Pin_4
#define SCL_PORT GPIOB
#define SCL_PIN GPIO_Pin_5
#define MOSI_PORT GPIOB
#define MOSI_PIN GPIO_Pin_6
#define MISO_PORT GPIOB
#define MISO_PIN GPIO_Pin_7

void Delay(uint32_t tics);
void setupSPI();
uint8_t LIS2DW12_ReadReg(uint8_t reg);
void LIS2DW12_WriteReg(uint8_t reg, uint8_t value);
uint8_t SPI_Transfer(uint8_t data);
int32_t read_axis(uint8_t regH, uint8_t regL);
int32_t z_axis();
int32_t x_axis();
int32_t y_axis();

int main(void) {
  GPIO_Init(RED_PORT, RED_PIN, GPIO_Mode_Out_PP_High_Fast);
  Serial_begin(115200);

  setupSPI();
  LIS2DW12_WriteReg(0x20, 0x44); //Enable accelerometer and temperature sensor

  while(true) {
    uint8_t id = LIS2DW12_ReadReg(0x0F);
    uint8_t tempL = LIS2DW12_ReadReg(0x0D);
    uint8_t tempH = LIS2DW12_ReadReg(0x0E);
    uint16_t temp_raw = ((uint16_t)tempH << 8 ) | tempL;
    temp_raw = temp_raw >> 4;
    int16_t tempActual = 25 + (temp_raw) / 16;

    printf("DAFI_T:%d_X:%d_Y:%d_Z:%d\n", 
        tempActual,
        (int)x_axis(),
        (int)y_axis(),
        (int)z_axis()
    );
  }
}

void Delay(uint32_t tics){
    for (volatile uint32_t i = 0; i < tics; i++);
}

void setupSPI(){
  GPIO_Init(CS_PORT, CS_PIN, GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init(MOSI_PORT, MOSI_PIN, GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init(MISO_PORT, MISO_PIN, GPIO_Mode_In_FL_No_IT);
  GPIO_Init(SCL_PORT, SCL_PIN, GPIO_Mode_Out_PP_High_Fast);

  CLK_PeripheralClockConfig(CLK_Peripheral_SPI1, ENABLE);
  SPI_DeInit(SPI1);
  SPI_Init(SPI1, 
           SPI_FirstBit_MSB,
           SPI_BaudRatePrescaler_8,
           SPI_Mode_Master,
           SPI_CPOL_High,
           SPI_CPHA_2Edge,
           SPI_Direction_2Lines_FullDuplex,
           SPI_NSS_Soft, 
           0x07
  );
  
  SPI_Cmd(SPI1, ENABLE);
}

uint8_t SPI_Transfer(uint8_t data) {
    while (SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET); // Wait for TX buffer empty
    SPI_SendData(SPI1, data);
    while (SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE) == RESET); // Wait for byte received
    return SPI_ReceiveData(SPI1);
}

uint8_t LIS2DW12_ReadReg(uint8_t reg) {
    uint8_t val;
    
    GPIO_ResetBits(CS_PORT, CS_PIN);     // Pull CS Low
    
    SPI_Transfer(reg | 0x80);            // Send address with READ bit (0x80)
    val = SPI_Transfer(0x00);            // Send dummy byte to get result
    
    GPIO_SetBits(CS_PORT, CS_PIN);       // Pull CS High
    return val;
}

void LIS2DW12_WriteReg(uint8_t reg, uint8_t value){
  GPIO_ResetBits(CS_PORT, CS_PIN);

  SPI_Transfer(reg & 0x7F);
  SPI_Transfer(value);

  GPIO_SetBits(CS_PORT, CS_PIN);
}

int32_t read_axis(uint8_t regH, uint8_t regL){
    uint8_t low = LIS2DW12_ReadReg(regL);
    uint8_t high = LIS2DW12_ReadReg(regH);
    
    // Combine to signed 16-bit
    int16_t combined = (int16_t)(((uint16_t)high << 8) | low);
    
    // Shift right by 2 for 14-bit resolution
    return ((int32_t)(combined >> 2) * 244) / 1000;
}

int32_t z_axis(){
  return read_axis(0x2D, 0x2C);
}

int32_t x_axis(){
  return read_axis(0x29, 0x28);
}

int32_t y_axis(){
  return read_axis(0x2B, 0x2A);
}
