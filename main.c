#include "stm8l15x.h"

int main() {
  GPIO_Init(GPIOA, GPIO_Pin_3, GPIO_Mode_Out_PP_Low_Fast);

  while(1) {
    GPIO_ToggleBits(GPIOA, GPIO_Pin_3);
    for(volatile long i = 0; i < 50000; i++);
  }
}

