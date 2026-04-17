#include "stm8l15x.h"
#include "stm8l15x_clk.h"
#include "stm8l15x_gpio.h"
#include "stm8l15x_rtc.h"
#include "stm8l15x_pwr.h"
#include <stdio.h>
#include <stdbool.h>
#include "serial.h"

#define RED_PORT GPIOA
#define RED_PIN GPIO_Pin_3

#define SLEEP_SECONDS 1 - 1 // 5 min wakeup routine
                            
#define WET_VAL 980
#define DRY_VAL 2755

#define RED_PORT GPIOA
#define BLUE_PORT GPIOD
#define DIODE_PORT GPIOB
#define CAP_PORT GPIOA
#define RED_PIN GPIO_Pin_3
#define BLUE_PIN  GPIO_Pin_0
#define DIODE_PIN GPIO_Pin_1
#define CAP_PIN GPIO_Pin_0

void Delay(uint32_t tics);
void initRTC();
void setupWakeup();
void timeForBed();
void wakeyWakey();
void setupADC(void);
uint16_t readSensor(void);
uint16_t readVREF(void);
void readMoistureSensor(bool enableSerial);

RTC_InitTypeDef RTC_ConfigStruct;
RTC_TimeTypeDef RTC_TimeInit;
RTC_TimeTypeDef RTC_TimeRead;
bool bit = 1;

int main(void) {
  GPIO_Init(RED_PORT, RED_PIN, GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init(DIODE_PORT, DIODE_PIN, GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init(CAP_PORT, CAP_PIN, GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init(BLUE_PORT, BLUE_PIN, GPIO_Mode_Out_PP_High_Fast);
  GPIO_SetBits(DIODE_PORT, DIODE_PIN);
  GPIO_SetBits(CAP_PORT, CAP_PIN);
  GPIO_SetBits(BLUE_PORT, BLUE_PIN);
  Serial_begin(115200);

  initRTC();
  setupWakeup();
  setupADC();
  enableInterrupts();

  while(true) {
    wakeyWakey(); 

      
    readMoistureSensor(1);
    // RTC_GetTime(RTC_Format_BIN, &RTC_TimeRead);
    // printf("Time %02d:%02d:%02d\n", 
    //     RTC_TimeRead.RTC_Hours,
    //     RTC_TimeRead.RTC_Minutes,
    //     RTC_TimeRead.RTC_Seconds
    // );
    
    timeForBed();
    halt(); 
  }
}

void Delay(uint32_t tics){
    for (volatile uint32_t i = 0; i < tics; i++);
}


void initRTC(){
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

void setupWakeup(){
  // 38kHz LSI clock / 16 = 2317
  RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
  // Every 10s
  RTC_SetWakeUpCounter(SLEEP_SECONDS);
  RTC_ITConfig(RTC_IT_WUT, ENABLE);
  RTC_WakeUpCmd(ENABLE);
}

void timeForBed(){
    // GPIO_ResetBits(RED_PORT, RED_PIN);

    PWR_UltraLowPowerCmd(ENABLE);
    FLASH->CR1 |= (uint8_t)FLASH_CR1_EEPM; // Flash power down during sleep
    // CLK_MainRegulatorCmd(DISABLE);
    RTC_ClearFlag(RTC_FLAG_WUTF);
}

void wakeyWakey(){
    FLASH->CR1 &= (uint8_t)(~FLASH_CR1_EEPM); // Flash power up after sleep
    PWR_UltraLowPowerCmd(DISABLE); // Enable Ultra Low Power mode
    CLK_HSICmd(ENABLE); // Re-enable high speed clock after waking up 
    while(CLK_GetFlagStatus(CLK_FLAG_HSIRDY) == RESET);
    RTC_WaitForSynchro(); // Necessary for synchronization and proper readings
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

  Delay(50000);
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

void readMoistureSensor(bool enableSerial){
    uint16_t sensor_raw = readSensor();
    uint16_t vref_raw = readVREF();

    uint32_t actual_vdd = (1225UL * 1024) / vref_raw;
    // Low battery warning
    if(actual_vdd < 2900) GPIO_SetBits(RED_PORT, RED_PIN);
    else GPIO_ResetBits(RED_PORT, RED_PIN);
    
    uint32_t sensor_mV = ((uint32_t)sensor_raw * actual_vdd)  / 1024;

    uint32_t moist_pctg = 0;
    if (sensor_mV >= DRY_VAL) moist_pctg = 0;    
    else if (sensor_mV <= WET_VAL) moist_pctg = 100;    
    else moist_pctg = ((DRY_VAL - sensor_mV) * 100) / (DRY_VAL - WET_VAL);


    if(moist_pctg < 10){
      GPIO_SetBits(CAP_PORT, CAP_PIN); // Charging capacitor
      GPIO_ResetBits(BLUE_PORT, BLUE_PIN); // Selecting LED
      GPIO_ResetBits(DIODE_PORT, DIODE_PIN); // Discharging through diode
      GPIO_SetBits(DIODE_PORT, DIODE_PIN); // Discharging through diode
    } 
    else{
      GPIO_ResetBits(CAP_PORT, CAP_PIN); // Charging capacitor
      GPIO_SetBits(BLUE_PORT, BLUE_PIN); // Selecting LED
      GPIO_SetBits(DIODE_PORT, DIODE_PIN); // Discharging through diode
    }

    if(enableSerial == true){
      printf("\nsensor: %d\n\r", sensor_raw);
      printf("vref:     %d\n\r", vref_raw);
      printf("Vdd:      %lumV\n\r", actual_vdd);
      printf("Sensor:   %lumV\n\r", sensor_mV);
      printf("Moisture: %lu%%\n\r", moist_pctg);
      printf("----------------");
    }
}

// Interrupt vector for wakeup handling
void RTC_CSSLSE_IRQHandler(void) __interrupt(4) {
    if (RTC_GetITStatus(RTC_IT_WUT) != RESET) {
        RTC_ClearITPendingBit(RTC_IT_WUT);
    }
}
