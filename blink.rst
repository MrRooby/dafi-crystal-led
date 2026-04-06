                                      1 ;--------------------------------------------------------
                                      2 ; File Created by SDCC : free open source ISO C Compiler 
                                      3 ; Version 4.4.0 #14620 (Linux)
                                      4 ;--------------------------------------------------------
                                      5 	.module main
                                      6 	.optsdcc -mstm8
                                      7 	
                                      8 ;--------------------------------------------------------
                                      9 ; Public variables in this module
                                     10 ;--------------------------------------------------------
                                     11 	.globl _main
                                     12 	.globl _delay
                                     13 ;--------------------------------------------------------
                                     14 ; ram data
                                     15 ;--------------------------------------------------------
                                     16 	.area DATA
                                     17 ;--------------------------------------------------------
                                     18 ; ram data
                                     19 ;--------------------------------------------------------
                                     20 	.area INITIALIZED
                                     21 ;--------------------------------------------------------
                                     22 ; Stack segment in internal ram
                                     23 ;--------------------------------------------------------
                                     24 	.area SSEG
      000001                         25 __start__stack:
      000001                         26 	.ds	1
                                     27 
                                     28 ;--------------------------------------------------------
                                     29 ; absolute external ram data
                                     30 ;--------------------------------------------------------
                                     31 	.area DABS (ABS)
                                     32 
                                     33 ; default segment ordering for linker
                                     34 	.area HOME
                                     35 	.area GSINIT
                                     36 	.area GSFINAL
                                     37 	.area CONST
                                     38 	.area INITIALIZER
                                     39 	.area CODE
                                     40 
                                     41 ;--------------------------------------------------------
                                     42 ; interrupt vector
                                     43 ;--------------------------------------------------------
                                     44 	.area HOME
      008000                         45 __interrupt_vect:
      008000 82 00 80 07             46 	int s_GSINIT ; reset
                                     47 ;--------------------------------------------------------
                                     48 ; global & static initialisations
                                     49 ;--------------------------------------------------------
                                     50 	.area HOME
                                     51 	.area GSINIT
                                     52 	.area GSFINAL
                                     53 	.area GSINIT
      008007 CD 80 80         [ 4]   54 	call	___sdcc_external_startup
      00800A 4D               [ 1]   55 	tnz	a
      00800B 27 03            [ 1]   56 	jreq	__sdcc_init_data
      00800D CC 80 04         [ 2]   57 	jp	__sdcc_program_startup
      008010                         58 __sdcc_init_data:
                                     59 ; stm8_genXINIT() start
      008010 AE 00 00         [ 2]   60 	ldw x, #l_DATA
      008013 27 07            [ 1]   61 	jreq	00002$
      008015                         62 00001$:
      008015 72 4F 00 00      [ 1]   63 	clr (s_DATA - 1, x)
      008019 5A               [ 2]   64 	decw x
      00801A 26 F9            [ 1]   65 	jrne	00001$
      00801C                         66 00002$:
      00801C AE 00 00         [ 2]   67 	ldw	x, #l_INITIALIZER
      00801F 27 09            [ 1]   68 	jreq	00004$
      008021                         69 00003$:
      008021 D6 80 2C         [ 1]   70 	ld	a, (s_INITIALIZER - 1, x)
      008024 D7 00 00         [ 1]   71 	ld	(s_INITIALIZED - 1, x), a
      008027 5A               [ 2]   72 	decw	x
      008028 26 F7            [ 1]   73 	jrne	00003$
      00802A                         74 00004$:
                                     75 ; stm8_genXINIT() end
                                     76 	.area GSFINAL
      00802A CC 80 04         [ 2]   77 	jp	__sdcc_program_startup
                                     78 ;--------------------------------------------------------
                                     79 ; Home
                                     80 ;--------------------------------------------------------
                                     81 	.area HOME
                                     82 	.area HOME
      008004                         83 __sdcc_program_startup:
      008004 CC 80 43         [ 2]   84 	jp	_main
                                     85 ;	return from main will return to caller
                                     86 ;--------------------------------------------------------
                                     87 ; code
                                     88 ;--------------------------------------------------------
                                     89 	.area CODE
                                     90 ;	main.c: 37: void delay(void) {
                                     91 ;	-----------------------------------------
                                     92 ;	 function delay
                                     93 ;	-----------------------------------------
      00802D                         94 _delay:
      00802D 52 02            [ 2]   95 	sub	sp, #2
                                     96 ;	main.c: 39: for(i=0; i<50000; i++){;} // crude delay
      00802F 5F               [ 1]   97 	clrw	x
      008030 1F 01            [ 2]   98 	ldw	(0x01, sp), x
      008032                         99 00103$:
      008032 1E 01            [ 2]  100 	ldw	x, (0x01, sp)
      008034 A3 C3 50         [ 2]  101 	cpw	x, #0xc350
      008037 24 07            [ 1]  102 	jrnc	00105$
      008039 1E 01            [ 2]  103 	ldw	x, (0x01, sp)
      00803B 5C               [ 1]  104 	incw	x
      00803C 1F 01            [ 2]  105 	ldw	(0x01, sp), x
      00803E 20 F2            [ 2]  106 	jra	00103$
      008040                        107 00105$:
                                    108 ;	main.c: 40: }
      008040 5B 02            [ 2]  109 	addw	sp, #2
      008042 81               [ 4]  110 	ret
                                    111 ;	main.c: 42: void main() {
                                    112 ;	-----------------------------------------
                                    113 ;	 function main
                                    114 ;	-----------------------------------------
      008043                        115 _main:
                                    116 ;	main.c: 44: GPIOA_DDR |= (1 << LED_R);
      008043 72 16 50 02      [ 1]  117 	bset	0x5002, #3
                                    118 ;	main.c: 45: GPIOA_CR1 |= (1 << LED_R);
      008047 72 16 50 03      [ 1]  119 	bset	0x5003, #3
                                    120 ;	main.c: 47: GPIOD_DDR |= (1 << LED_G);
      00804B 72 10 50 11      [ 1]  121 	bset	0x5011, #0
                                    122 ;	main.c: 48: GPIOD_CR1 |= (1 << LED_G);
      00804F 72 10 50 12      [ 1]  123 	bset	0x5012, #0
                                    124 ;	main.c: 50: GPIOB_DDR |= (1 << LED_B);
      008053 72 10 50 07      [ 1]  125 	bset	0x5007, #0
                                    126 ;	main.c: 51: GPIOB_CR1 |= (1 << LED_B);
      008057 72 10 50 08      [ 1]  127 	bset	0x5008, #0
                                    128 ;	main.c: 54: GPIOB_DDR &= ~(1 << BUTTON);
      00805B 72 15 50 07      [ 1]  129 	bres	0x5007, #2
                                    130 ;	main.c: 55: GPIOB_CR1 |= (1 << BUTTON);
      00805F 72 14 50 08      [ 1]  131 	bset	0x5008, #2
                                    132 ;	main.c: 57: while(1) {
      008063                        133 00105$:
                                    134 ;	main.c: 58: if(GPIOB_IDR & (1 << BUTTON)){
      008063 C6 50 06         [ 1]  135 	ld	a, 0x5006
      008066 97               [ 1]  136 	ld	xl, a
                                    137 ;	main.c: 59: GPIOA_ODR |= (1 << LED_R);
      008067 C6 50 00         [ 1]  138 	ld	a, 0x5000
                                    139 ;	main.c: 58: if(GPIOB_IDR & (1 << BUTTON)){
      00806A 88               [ 1]  140 	push	a
      00806B 9F               [ 1]  141 	ld	a, xl
      00806C A5 04            [ 1]  142 	bcp	a, #0x04
      00806E 84               [ 1]  143 	pop	a
      00806F 27 07            [ 1]  144 	jreq	00102$
                                    145 ;	main.c: 59: GPIOA_ODR |= (1 << LED_R);
      008071 AA 08            [ 1]  146 	or	a, #0x08
      008073 C7 50 00         [ 1]  147 	ld	0x5000, a
      008076 20 EB            [ 2]  148 	jra	00105$
      008078                        149 00102$:
                                    150 ;	main.c: 62: GPIOA_ODR &= ~(1 << LED_R);
      008078 A4 F7            [ 1]  151 	and	a, #0xf7
      00807A C7 50 00         [ 1]  152 	ld	0x5000, a
      00807D 20 E4            [ 2]  153 	jra	00105$
                                    154 ;	main.c: 65: }
      00807F 81               [ 4]  155 	ret
                                    156 	.area CODE
                                    157 	.area CONST
                                    158 	.area INITIALIZER
                                    159 	.area CABS (ABS)
