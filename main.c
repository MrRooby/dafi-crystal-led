#include "stm8l15x.h"
#include "stm8l15x_adc.h"
#include "stm8l15x_clk.h"
#include "stm8l15x_gpio.h"
#include <stdio.h>
#include "serial.h"

void setupADC(void);
int readADC(void);

int main(void) {
  GPIO_Init(GPIOA, GPIO_Pin_3, GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_In_FL_No_IT);

  Serial_begin(115200);

  CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);
  ADC_DeInit(ADC1);

  ADC_Init(ADC1, ADC_ConversionMode_Single, ADC_Resolution_10Bit, ADC_Prescaler_1);
  ADC_ChannelCmd(ADC1, ADC_Channel_18, ENABLE);
  ADC_SamplingTimeConfig(ADC1, ADC_Group_SlowChannels, ADC_SamplingTime_384Cycles);
  // ADC_TempSensorCmd(ENABLE);
  // ADC_ChannelCmd(ADC1, ADC_Channel_TempSensor, ENABLE);
  ADC_Cmd(ADC1, ENABLE);
  for (volatile uint32_t i = 0; i < 50000; i++);

  
  int counter = 0;

  while(1) {
    ADC_SoftwareStartConv(ADC1);

    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    uint16_t adc = ADC_GetConversionValue(ADC1);

    if(adc > 800){
      GPIO_SetBits(GPIOA, GPIO_Pin_3);
    }
    else {
      GPIO_ResetBits(GPIOA, GPIO_Pin_3);
    }
    int32_t temp_x10 = 2300 + ((int32_t)adc - 157) * 138; 

    // printf("Internal temp: %ld.%02ld C\r\n", temp_x10 / 100, temp_x10 % 100);
    printf("%d\n\r", adc);

    for (volatile uint32_t i = 0; i < 100000; i++);
  }
}
