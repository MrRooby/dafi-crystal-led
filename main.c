#include "stm8l.h"

#define LED_R 3 // PA
#define LED_G 0 // PD
#define LED_B 0 // PB
#define BUTTON 2
#define LED_CAP 0 //PA
#define LED_DIODE 1 //PB

void delay(void) {
  volatile unsigned int i;
  for(i=0; i<50000; i++){;} // crude delay
}

void main() {
  // PA3 => LED Red
  PA_DDR |= (1 << LED_R);
  PA_CR1 |= (1 << LED_R);

  // PD0 => LED Green
  PD_DDR |= (1 << LED_G);
  PD_CR1 |= (1 << LED_G);

  // PB0 => LED Blue
  PB_DDR |= (1 << LED_B);
  PB_CR1 |= (1 << LED_B);

  // PB1 => LED Diode
  PB_DDR |= (1 << LED_DIODE);
  PB_CR1 |= (1 << LED_DIODE);

  // PA0 => LED Capacitor
  PA_DDR |= (1 << LED_CAP);
  PA_CR1 |= (1 << LED_CAP);

  while(1) {
    PA_ODR |= (1 << LED_CAP); // charging the cap
    // delay();
    PD_ODR |= (1 << LED_B); // selecting diode
    PB_ODR &= ~(1 << LED_G); // selecting diode

    PB_ODR &= ~(1 << LED_DIODE); // discharging through diode
    // delay();
    PB_ODR |= (1 << LED_DIODE); 
    delay();
    PB_ODR |= (1 << LED_B); // selecting diode
    PD_ODR &= ~(1 << LED_G); // selecting diode
    delay();
  }
}

