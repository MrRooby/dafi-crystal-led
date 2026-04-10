#include "stm8l15x.h"
#include "stm8l15x_usart.h"
#include "stm8l15x_clk.h"

int main() {
  CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);
  GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast);

  USART_Init(USART1, 
            (uint32_t)115200, 
            USART_WordLength_8b, 
            USART_StopBits_1, 
            USART_Parity_No, 
            USART_Mode_Tx);

  USART_Cmd(USART1, ENABLE);

  char* str = "ping\r\n";

  while(1) {
    // GPIO_ToggleBits(GPIOA, GPIO_Pin_3);
    while (*str) {
      // Wait until Transmit Data Register is empty
      while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
      USART_SendData8(USART1, *str++);
    }
    for(volatile long i = 0; i < 50000; i++);
  }
}

