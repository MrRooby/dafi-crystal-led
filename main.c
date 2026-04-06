#include "stm8l.h"


#define LED_R 3
#define LED_G 0
#define LED_B 0
#define BUTTON 2


void delay(void) {
  volatile unsigned int i;
  for(i=0; i<50000; i++){;} // crude delay
}

void main() {
  // Configure PA3 as output push-pull
  GPIOA_DDR |= (1 << LED_R);
  GPIOA_CR1 |= (1 << LED_R);

  GPIOD_DDR |= (1 << LED_G);
  GPIOD_CR1 |= (1 << LED_G);

  GPIOB_DDR |= (1 << LED_B);
  GPIOB_CR1 |= (1 << LED_B);

  // BUTTON as input
  GPIOB_DDR &= ~(1 << BUTTON);
  GPIOB_CR1 |= (1 << BUTTON);
    
  while(1) {
    if(GPIOB_IDR & (1 << BUTTON)){
      GPIOA_ODR |= (1 << LED_R);
    }
    else {
      GPIOA_ODR &= ~(1 << LED_R);
    }
  }
}
