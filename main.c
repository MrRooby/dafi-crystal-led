#include "stm8l15x.h"
#include "stm8l15x_clk.h"
#include "stm8l15x_gpio.h"
#include "stm8l15x_rtc.h"
#include "stm8l15x_pwr.h"
#include "stm8l15x_usart.h"
#include <stdio.h>
#include <stdbool.h>
#include "serial.h"

#define RED_PORT GPIOA
#define RED_PIN GPIO_Pin_3

#define SLEEP_SECONDS 300 // 5 min wakeup routine

void Delay(uint32_t tics);
void initRTC();
void setupWakeup();
void timeForBed();
void wakeyWakey();

RTC_InitTypeDef RTC_ConfigStruct;
RTC_TimeTypeDef RTC_TimeInit;
RTC_TimeTypeDef RTC_TimeRead;

int main(void) {
  GPIO_Init(RED_PORT, RED_PIN, GPIO_Mode_Out_PP_Low_Slow);
  Serial_begin(115200);

  initRTC();
  setupWakeup();
  enableInterrupts();

  while(true) {
    wakeyWakey();
    GPIO_SetBits(RED_PORT, RED_PIN);

    RTC_GetTime(RTC_Format_BIN, &RTC_TimeRead);
    printf("Time %02d:%02d:%02d\n", 
        RTC_TimeRead.RTC_Hours,
        RTC_TimeRead.RTC_Minutes,
        RTC_TimeRead.RTC_Seconds
    );
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET); // Wait for Serial to finish
    
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

// Interrupt vector for wakeup handling
void RTC_CSSLSE_IRQHandler(void) __interrupt(4) {
    if (RTC_GetITStatus(RTC_IT_WUT) != RESET) {
        RTC_ClearITPendingBit(RTC_IT_WUT);
    }
}

void timeForBed(){
    GPIO_ResetBits(RED_PORT, RED_PIN);

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
