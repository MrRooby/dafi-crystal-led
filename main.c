#include "stm8l15x.h"
#include "stm8l15x_usart.h"
#include "stm8l15x_adc.h"
#include "stm8l15x_clk.h"
#include "stm8l15x_gpio.h"

char Serial_read_char(void);
void Serial_print_char(char value);
void Serial_begin(uint32_t baud_rate);
void setupADC(void);
int readADC(void);
bool Serial_available();

int main(void) {
  GPIO_Init(GPIOA, GPIO_Pin_3, GPIO_Mode_Out_PP_High_Fast);
  Serial_begin(115200);
  while(1) {
    GPIO_SetBits(GPIOA, GPIO_Pin_3);
    Serial_print_char('t');
    Serial_print_char('\n');
    Serial_print_char('\r');
    for (volatile uint32_t i = 0; i < 50000; i++);
    GPIO_ResetBits(GPIOA, GPIO_Pin_3);
    for (volatile uint32_t i = 0; i < 50000; i++);
  }
}


char Serial_read_char(void){
  while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
  USART_ClearFlag(USART1, USART_FLAG_RXNE);
  return (USART_ReceiveData8(USART1));
}

void Serial_print_char(char value){
  while (!(USART1->SR & 0x80));
  USART1->DR = value;
  while (!(USART1->SR & 0x40));
}

void Serial_begin(uint32_t baud_rate){
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
  CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);

  SYSCFG->RMPCR1 |= 0x1C;
  GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast);

  (void) USART1->SR;
  (void) USART1->DR;
  uint16_t div = (uint16_t)(16000000L / baud_rate);

  /* 5. Split div into BRR2 and BRR1
     Example for 0x0682:
     BRR2: (0x0682 & 0xF000) >> 8 | (0x0682 & 0x000F) = 0x00 | 0x02 = 0x02
     BRR1: (0x0682 & 0x0FF0) >> 4 = 0x068 */
  USART1->BRR2 = (uint8_t)(((div & 0xF000) >> 8) | (div & 0x000F));
  USART1->BRR1 = (uint8_t)((div & 0x0FF0) >> 4);
  // USART1->BRR2 = 0x02; // Baud rate of 9600 
  // USART1->BRR1 = 0x68; 

  USART1->CR1 = 0x00;  /* Set USART_CR1 to reset value 0x00 */
  USART1->CR2 = 0x08;  /* Set USART_CR2 to reset value 0x00 */
  USART1->CR3 = 0x00;  /* Set USART_CR3 to reset value 0x00 */
}

bool Serial_available(){
  if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == TRUE)
    return TRUE;
  else
    return FALSE;
}

void setupADC(void){
  ADC_ChannelCmd(ADC1, ADC_Channel_0, DISABLE);
  ADC_Cmd(ADC1, DISABLE);
  CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);
  GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_No_IT);
  ADC_Init(ADC1, ADC_ConversionMode_Single, ADC_Resolution_10Bit, ADC_Prescaler_1);
  ADC_ChannelCmd(ADC1, ADC_Channel_15, ENABLE);
  ADC_Cmd(ADC1, ENABLE);
}

int readADC(void){
  ADC_SoftwareStartConv(ADC1);

  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
  ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

  return (int)ADC_GetConversionValue(ADC1);
}
