#ifndef STM8L_H
#define STM8L_H
// PORTA
#define PA_ODR  (*(volatile unsigned char*)0x5000)
#define PA_IDR  (*(volatile unsigned char*)0x5001)
#define PA_DDR  (*(volatile unsigned char*)0x5002)
#define PA_CR1  (*(volatile unsigned char*)0x5003)
#define PA_CR2  (*(volatile unsigned char*)0x5004)

// PORTB
#define PB_ODR  (*(volatile unsigned char*)0x5005)
#define PB_IDR  (*(volatile unsigned char*)0x5006)
#define PB_DDR  (*(volatile unsigned char*)0x5007)
#define PB_CR1  (*(volatile unsigned char*)0x5008)
#define PB_CR2  (*(volatile unsigned char*)0x5009)

// PORTC
#define PC_ODR  (*(volatile unsigned char*)0x500A)
#define PC_IDR  (*(volatile unsigned char*)0x500B)
#define PC_DDR  (*(volatile unsigned char*)0x500C)
#define PC_CR1  (*(volatile unsigned char*)0x500D)
#define PC_CR2  (*(volatile unsigned char*)0x500E)

// PORTD
#define PD_ODR  (*(volatile unsigned char*)0x500F)
#define PD_IDR  (*(volatile unsigned char*)0x5010)
#define PD_DDR  (*(volatile unsigned char*)0x5011)
#define PD_CR1  (*(volatile unsigned char*)0x5012)
#define PD_CR2  (*(volatile unsigned char*)0x5013)

#endif // STM8L_H
