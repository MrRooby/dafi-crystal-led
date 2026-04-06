#define GPIOB_BASE 0x500004
#define GPIOB      ((volatile struct { 
    unsigned char ODR; 
    unsigned char IDR; 
    unsigned char DDR; 
    unsigned char CR1; 
    unsigned char CR2; 
}) GPIOB_t *) GPIOB_BASE
