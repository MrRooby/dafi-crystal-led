#ifndef STM8L_H
#define STM8L_H
// PORTA
#define GPIOA_ODR  (*(volatile unsigned char*)0x5000)
#define GPIOA_IDR  (*(volatile unsigned char*)0x5001)
#define GPIOA_DDR  (*(volatile unsigned char*)0x5002)
#define GPIOA_CR1  (*(volatile unsigned char*)0x5003)
#define GPIOA_CR2  (*(volatile unsigned char*)0x5004)

// PORTB
#define GPIOB_ODR  (*(volatile unsigned char*)0x5005)
#define GPIOB_IDR  (*(volatile unsigned char*)0x5006)
#define GPIOB_DDR  (*(volatile unsigned char*)0x5007)
#define GPIOB_CR1  (*(volatile unsigned char*)0x5008)
#define GPIOB_CR2  (*(volatile unsigned char*)0x5009)

// PORTC
#define GPIOC_ODR  (*(volatile unsigned char*)0x500A)
#define GPIOC_IDR  (*(volatile unsigned char*)0x500B)
#define GPIOC_DDR  (*(volatile unsigned char*)0x500C)
#define GPIOC_CR1  (*(volatile unsigned char*)0x500D)
#define GPIOC_CR2  (*(volatile unsigned char*)0x500E)

// PORTD
#define GPIOD_ODR  (*(volatile unsigned char*)0x500F)
#define GPIOD_IDR  (*(volatile unsigned char*)0x5010)
#define GPIOD_DDR  (*(volatile unsigned char*)0x5011)
#define GPIOD_CR1  (*(volatile unsigned char*)0x5012)
#define GPIOD_CR2  (*(volatile unsigned char*)0x5013)

#endif // STM8L_H
