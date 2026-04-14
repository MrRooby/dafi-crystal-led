#include "stm8l15x.h"
#include "stm8l15x_gpio.h"
#include <stdio.h>
#include "serial.h"

void setupADC(void);
int readADC(void);
int putchar(int c);

int main(void) {
  GPIO_Init(GPIOA, GPIO_Pin_3, GPIO_Mode_Out_PP_High_Fast);

  Serial_begin(115200);

  int counter = 0;

  while(1) {
    GPIO_SetBits(GPIOA, GPIO_Pin_3);
    printf("Hello STM8! Count: %d\r\n", counter++);
    for (volatile uint32_t i = 0; i < 50000; i++);
    GPIO_ResetBits(GPIOA, GPIO_Pin_3);
    for (volatile uint32_t i = 0; i < 50000; i++);
  }
}


int putchar(int c) {
    Serial_print_char((char)c);
    return c;
}
