#include "stm8l15x.h"
#include "stm8l15x_adc.h"
#include "stm8l15x_clk.h"
#include "stm8l15x_gpio.h"
#include <stdio.h>
#include "serial.h"

#define WET_VAL 980
#define DRY_VAL 2755

void setupADC(void);
uint16_t readSensor(void);
uint16_t readVREF(void);
void readMoistureSensor(uint8_t enableSerial);

int main(void) {
  GPIO_Init(GPIOA, GPIO_Pin_3, GPIO_Mode_Out_PP_High_Fast);

  Serial_begin(115200);
  setupADC();
  
  while(1) {
    readMoistureSensor(1);
    for (volatile uint32_t i = 0; i < 200000; i++);
  }
}


void setupADC(void){
  GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_In_FL_No_IT);
  CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);
  ADC_DeInit(ADC1);

  ADC_Init(ADC1, ADC_ConversionMode_Single, ADC_Resolution_10Bit, ADC_Prescaler_1);

  ADC_VrefintCmd(ENABLE);
  ADC_ChannelCmd(ADC1, ADC_Channel_Vrefint, ENABLE); // VREFINT
  ADC_ChannelCmd(ADC1, ADC_Channel_18, ENABLE);

  ADC_SamplingTimeConfig(ADC1, ADC_Group_SlowChannels, ADC_SamplingTime_384Cycles);
  ADC_SamplingTimeConfig(ADC1, ADC_Group_FastChannels, ADC_SamplingTime_384Cycles);

  ADC_Cmd(ADC1, ENABLE);

  for (volatile uint32_t i = 0; i < 50000; i++);
}

uint16_t readSensor(void){
  ADC_ChannelCmd(ADC1, ADC_Channel_Vrefint, DISABLE); // VREFINT
  ADC_ChannelCmd(ADC1, ADC_Channel_18, ENABLE);
  ADC_SoftwareStartConv(ADC1);

  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
  return ADC_GetConversionValue(ADC1);
}

uint16_t readVREF(void){
  ADC_ChannelCmd(ADC1, ADC_Channel_Vrefint, ENABLE); // VREFINT
  ADC_ChannelCmd(ADC1, ADC_Channel_18, DISABLE);
  ADC_SoftwareStartConv(ADC1);
  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
  return ADC_GetConversionValue(ADC1);
}

void readMoistureSensor(uint8_t enableSerial){
    uint16_t sensor_raw = readSensor();
    uint16_t vref_raw = readVREF();

    uint32_t actual_vdd = (1225UL * 1024) / vref_raw;
    uint32_t sensor_mV = ((uint32_t)sensor_raw * actual_vdd)  / 1024;

    uint32_t moist_pctg = 0;
    if (sensor_mV >= DRY_VAL) moist_pctg = 0;    
    else if (sensor_mV <= WET_VAL) moist_pctg = 100;    
    else moist_pctg = ((DRY_VAL - sensor_mV) * 100) / (DRY_VAL - WET_VAL);

    if(moist_pctg < 10) GPIO_SetBits(GPIOA, GPIO_Pin_3);
    else GPIO_ResetBits(GPIOA, GPIO_Pin_3);

    if(enableSerial == 1){
      printf("\nsensor:   %d\n\r", sensor_raw);
      printf("vref:     %d\n\r", vref_raw);
      printf("Vdd:      %lumV\n\r", actual_vdd);
      printf("Sensor:   %lumV\n\r", sensor_mV);
      printf("Moisture: %lu%%\n\r", moist_pctg);
      printf("----------------");
    }
}
