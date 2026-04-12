#include "stm8l15x.h"
#include "stm8l15x_adc.h"
#include "stm8l15x_clk.h"
#include "stm8l15x_gpio.h"
#include "stm8l15x_syscfg.h"

void setupADC(void){
  CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);
  CLK->PCKENR2 |= 0x80; // Włącza zegar dla RI/SYSCFG (bit 7)
  GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_No_IT);

  ADC_Init(ADC1, ADC_ConversionMode_Single, ADC_Resolution_10Bit, ADC_Prescaler_1);
  ADC_SamplingTimeConfig(ADC1, ADC_Group_SlowChannels, ADC_SamplingTime_192Cycles);
  ADC_Cmd(ADC1, ENABLE);
  ADC_ChannelCmd(ADC1, ADC_Channel_15, ENABLE);
}

int readADC(void){
  ADC_SoftwareStartConv(ADC1);

  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

  return (int)ADC_GetConversionValue(ADC1);
}

int main(void) {
  GPIO_Init(GPIOA, GPIO_Pin_3, GPIO_Mode_Out_PP_Low_Fast);
  
  setupADC();

  int sensor = 0;

  while(1) {
    if(readADC()){
      GPIO_SetBits(GPIOA, GPIO_Pin_3);
    }
    else {
      GPIO_ResetBits(GPIOA, GPIO_Pin_3);
    } 
    for (volatile uint32_t i = 0; i < 50000; i++);
  }
}

