#include "stm8l.h"

#define LED_R 3 // PA
#define LED_G 0 // PD
#define LED_B 0 // PB
#define BUTTON 2
#define LED_CAP 0 //PA
#define LED_DIODE 1 //PB

void delay(void) {
  volatile unsigned int i;
  for(i=0; i<50; i++){;} // crude delay
}

void main() {
  // PA3 => LED Red
  GPIOA_DDR |= (1 << LED_R);
  GPIOA_CR1 |= (1 << LED_R);

  // PD0 => LED Green
  GPIOD_DDR |= (1 << LED_G);
  GPIOD_CR1 |= (1 << LED_G);

  // PB0 => LED Blue
  GPIOB_DDR |= (1 << LED_B);
  GPIOB_CR1 |= (1 << LED_B);

  // PB1 => LED Diode
  GPIOB_DDR |= (1 << LED_DIODE);
  GPIOB_CR1 |= (1 << LED_DIODE);
 
  // PA0 => LED Capacitor
  GPIOA_DDR |= (1 << LED_CAP);
  GPIOA_CR1 |= (1 << LED_CAP);

  while(1) {
    GPIOA_ODR |= (1 << LED_CAP); // charging the cap
    // delay();
    GPIOB_ODR |= (1 << LED_B); // selecting diode
    GPIOD_ODR &= ~(1 << LED_G); // selecting diode
    GPIOB_ODR &= ~(1 << LED_DIODE); // discharging through diode
    // delay();
    GPIOB_ODR |= (1 << LED_DIODE); 
  }
}
