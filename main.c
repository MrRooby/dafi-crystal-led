// #define DEBUG

#include "stm8l15x.h"
#include "stm8l15x_adc.h"
#include "stm8l15x_clk.h"
#include "stm8l15x_gpio.h"
#include "stm8l15x_rtc.h"
#include "stm8l15x_pwr.h"
#include <stdio.h>
#include <stdbool.h>
#ifdef DEBUG
  #include "serial.h"
#endif /* ifdef DEBUG */

#define RED_PORT GPIOA
#define RED_PIN GPIO_Pin_3

#define SLEEP_SECONDS 1800 - 1 // 5 min wakeup routine
                            
#define WET_VAL 1000
#define DRY_VAL 2750
#define MOISTURE 10

#define RED_PORT GPIOA
#define BLUE_PORT GPIOD
#define DIODE_PORT GPIOB
#define CAP_PORT GPIOA
#define SENSOR_PORT GPIOB
#define RED_PIN GPIO_Pin_3
#define BLUE_PIN  GPIO_Pin_0
#define DIODE_PIN GPIO_Pin_1
#define CAP_PIN GPIO_Pin_0
#define SENSOR_PIN GPIO_Pin_2

inline void Delay(const uint32_t millis);
void initRTC(void);
void setupWakeup(void);
void timeForBed(void);
void wakeyWakey(void);
void setupADC(void);
uint16_t readSensor(void);
uint16_t readVREF(void);
void readMoistureSensor(void);

RTC_InitTypeDef RTC_ConfigStruct;
RTC_TimeTypeDef RTC_TimeInit;
RTC_TimeTypeDef RTC_TimeRead;
bool bit = 1;

int main(void) {
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
  GPIO_Init(GPIOA, GPIO_Pin_All, GPIO_Mode_Out_PP_Low_Slow);
  GPIO_Init(GPIOB, GPIO_Pin_All, GPIO_Mode_Out_PP_Low_Slow);
  GPIO_Init(GPIOC, GPIO_Pin_All, GPIO_Mode_Out_PP_Low_Slow);
  GPIO_Init(GPIOD, GPIO_Pin_All, GPIO_Mode_Out_PP_Low_Slow);

  GPIO_Init(RED_PORT, RED_PIN, GPIO_Mode_Out_PP_Low_Fast);
  GPIO_Init(RED_PORT, RED_PIN, GPIO_Mode_Out_PP_Low_Fast);
  GPIO_Init(SENSOR_PORT, SENSOR_PIN, GPIO_Mode_Out_PP_Low_Fast);
  GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_In_FL_No_IT);

  GPIO_ResetBits(RED_PORT, RED_PIN);
  GPIO_ResetBits(SENSOR_PORT, SENSOR_PIN);

  #ifdef DEBUG
    Serial_begin(115200);
  #endif /* ifdef DEBUG */

  initRTC();
  setupWakeup();
  setupADC();
  enableInterrupts();

  while(true) {
    wakeyWakey(); 
      
#ifdef DEBUG
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeRead);
    printf("\n|===%02d:%02d:%02d===|", 
        RTC_TimeRead.RTC_Hours,
        RTC_TimeRead.RTC_Minutes,
        RTC_TimeRead.RTC_Seconds
        );
#endif /* ifdef DEBUG */

    readMoistureSensor();
    
    timeForBed();
    halt(); 
  }
}

inline void Delay(const uint32_t millis){
    for (volatile uint32_t i = 0; i < millis * 800; i++);
}


void initRTC(void){
  CLK_LSICmd(ENABLE);
  while(CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET);
  CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_1);
  CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);

  RTC_DeInit();
  RTC_StructInit(&RTC_ConfigStruct);

  /* Tune the prescalers for ~38kHz LSI clock to get 1Hz */
  RTC_ConfigStruct.RTC_AsynchPrediv = 0x7F; // 127
  RTC_ConfigStruct.RTC_SynchPrediv = 296;   // (127+1) * (296+1) = ~38016 Hz

  RTC_Init(&RTC_ConfigStruct);
  RTC_TimeStructInit(&RTC_TimeInit);
  if(RTC_SetTime(RTC_Format_BIN, &RTC_TimeInit) != SUCCESS){
    printf("Cannot set time");
  }
}

void setupWakeup(void){
  // 38kHz LSI clock / 16 = 2317
  RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);

  RTC_SetWakeUpCounter(SLEEP_SECONDS);
  RTC_ITConfig(RTC_IT_WUT, ENABLE);
  RTC_WakeUpCmd(ENABLE);
}

void timeForBed(void){
    GPIO_ResetBits(SENSOR_PORT, SENSOR_PIN);

    ADC_VrefintCmd(DISABLE);
    ADC_Cmd(ADC1, DISABLE);

    CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, DISABLE);

    PWR_UltraLowPowerCmd(ENABLE);
    FLASH->CR1 |= (uint8_t)FLASH_CR1_EEPM; // Flash power down during sleep
    RTC_ClearFlag(RTC_FLAG_WUTF);
}

void wakeyWakey(void){
    FLASH->CR1 &= (uint8_t)(~FLASH_CR1_EEPM); // Flash power up after sleep
    PWR_UltraLowPowerCmd(DISABLE); // Enable Ultra Low Power mode
    CLK_HSICmd(ENABLE); // Re-enable high speed clock after waking up 
    while(CLK_GetFlagStatus(CLK_FLAG_HSIRDY) == RESET);
    RTC_WaitForSynchro(); // Necessary for synchronization and proper readings

    CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);
    ADC_VrefintCmd(ENABLE);
    ADC_Cmd(ADC1, ENABLE);
    GPIO_SetBits(SENSOR_PORT, SENSOR_PIN);
}

void setupADC(void){
  CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);
  ADC_DeInit(ADC1);

  ADC_Init(ADC1, ADC_ConversionMode_Single, ADC_Resolution_10Bit, ADC_Prescaler_1);

  ADC_VrefintCmd(ENABLE);
  ADC_ChannelCmd(ADC1, ADC_Channel_Vrefint, ENABLE); // VREFINT
  ADC_ChannelCmd(ADC1, ADC_Channel_18, ENABLE);

  ADC_SamplingTimeConfig(ADC1, ADC_Group_SlowChannels, ADC_SamplingTime_384Cycles);
  ADC_SamplingTimeConfig(ADC1, ADC_Group_FastChannels, ADC_SamplingTime_384Cycles);

  ADC_Cmd(ADC1, ENABLE);

  // Delay(50000);
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

void readMoistureSensor(void){
  Delay(1000); // Let sensor stabilize

  long sensor_avg = 0;
  for (int i = 0; i < 16; i++) {
    sensor_avg += readSensor(); 
  } // Take 100 readings and average them
  sensor_avg /= 16;

  const uint16_t vref_raw = readVREF();
  const uint32_t actual_vdd = (1225UL * 1024) / vref_raw;
  const uint32_t sensor_mV = (sensor_avg * actual_vdd)  / 1024;

  uint32_t sensor_mV_adj = sensor_mV;
  if (sensor_mV > DRY_VAL) sensor_mV_adj = DRY_VAL;
  else if (sensor_mV < WET_VAL) sensor_mV_adj = WET_VAL;
  
  const uint32_t range = ((uint32_t)DRY_VAL - (uint32_t)WET_VAL);
  uint32_t moist_pctg = ((DRY_VAL - sensor_mV_adj) * 100) / range;

  if(moist_pctg < MOISTURE){
    GPIO_SetBits(RED_PORT, RED_PIN); 
  } 
  else{
    GPIO_ResetBits(RED_PORT, RED_PIN); 
  }

#ifdef DEBUG
  printf("\nsensor:   %lu\n\r", sensor_avg);
  printf("VRef:     %d\n\r", vref_raw);
  printf("Vdd:      %lumV\n\r", actual_vdd);
  printf("Sensor:   %lumV\n\r", sensor_mV);
  printf("Moisture: %lu%%\n\r", moist_pctg);
  printf("----------------");
#endif /* ifdef DEBUG */

}

// Interrupt vector for wakeup handling
void RTC_CSSLSE_IRQHandler(void) __interrupt(4) {
    if (RTC_GetITStatus(RTC_IT_WUT) != RESET) {
        RTC_ClearITPendingBit(RTC_IT_WUT);
    }
}
