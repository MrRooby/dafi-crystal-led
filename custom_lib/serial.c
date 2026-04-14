#include "serial.h"

void Serial_begin(uint32_t baud_rate) {
    // 1. Clock and Pin Routing
    CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);

    // Remap USART1 to Port A (PA2=TX, PA3=RX)
    SYSCFG->RMPCR1 |= 0x1C; 
    
    // Configure TX Pin
    GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast);

    // 2. Clear status to prevent hanging
    (void) USART1->SR;
    (void) USART1->DR;

    // 3. Calculate Baud Rate Divider
    uint16_t div = (uint16_t)(16000000L / baud_rate);

    // 4. Set BRR registers (BRR2 must be written BEFORE BRR1)
    USART1->BRR2 = (uint8_t)(((div & 0xF000) >> 8) | (div & 0x000F));
    USART1->BRR1 = (uint8_t)((div & 0x0FF0) >> 4);

    // 5. Finalize setup (Transmitter only for now)
    USART1->CR1 = 0x00;
    USART1->CR2 = 0x08; // TEN (Transmitter Enable)
    USART1->CR3 = 0x00;
}

void Serial_print_char(char value) {
    while (!(USART1->SR & 0x80)); // Wait for TXE
    USART1->DR = value;
    while (!(USART1->SR & 0x40)); // Wait for TC
}

char Serial_read_char(void) {
    while (!(USART1->SR & 0x20)); // Wait for RXNE (Bit 5)
    return (uint8_t)USART1->DR;
}

bool Serial_available(void) {
    return (USART1->SR & 0x20) ? TRUE : FALSE;
}

// Retarget printf to use Serial_print_char
int putchar(int c) {
    Serial_print_char((char)c);
    return c;
}
