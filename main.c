#include "stm8l15x.h"
#include "stm8l15x_clk.h"
#include "stm8l15x_gpio.h"
#include "stm8l15x_rtc.h"
#include "stm8l15x_usart.h"
#include "stm8l15x_wfe.h"
#include <stdio.h>
#include <stdbool.h>
#include "serial.h"

// On LED
#define RED_PORT GPIOA
#define RED_PIN GPIO_Pin_3

void Delay(uint32_t tics);
void initRTC();
void setupWakeup();

RTC_InitTypeDef RTC_ConfigStruct;
RTC_TimeTypeDef RTC_TimeInit;
RTC_TimeTypeDef RTC_TimeRead;

int main(void) {
  GPIO_Init(RED_PORT, RED_PIN, GPIO_Mode_Out_PP_High_Fast);
  Serial_begin(115200);

  initRTC();
  setupWakeup();
  enableInterrupts();

  while(true) {
    // Re-enable high speed clock after waking up 
    CLK_HSICmd(ENABLE);
    while(CLK_GetFlagStatus(CLK_FLAG_HSIRDY) == RESET);

    RTC_GetTime(RTC_Format_BIN, &RTC_TimeRead);
    printf("Time %02d:%02d:%02d\n", 
        RTC_TimeRead.RTC_Hours,
        RTC_TimeRead.RTC_Minutes,
        RTC_TimeRead.RTC_Seconds
    );
    
    // Wait for Serial to finish sending before sleeping
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    
    // Go into Deep Sleep
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
  RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
  // Every 10s
  RTC_SetWakeUpCounter(23750);
  RTC_ITConfig(RTC_IT_WUT, ENABLE);
  RTC_WakeUpCmd(ENABLE);
}

void RTC_CSSLSE_IRQHandler(void) __interrupt(4) {
    if (RTC_GetITStatus(RTC_IT_WUT) != RESET) {
        RTC_ClearITPendingBit(RTC_IT_WUT);
    }
}
