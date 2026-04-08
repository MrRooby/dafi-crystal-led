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
      008007 CD 80 90         [ 4]   54 	call	___sdcc_external_startup
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
                                     90 ;	main.c: 10: void delay(void) {
                                     91 ;	-----------------------------------------
                                     92 ;	 function delay
                                     93 ;	-----------------------------------------
      00802D                         94 _delay:
      00802D 52 02            [ 2]   95 	sub	sp, #2
                                     96 ;	main.c: 12: for(i=0; i<50000; i++){;} // crude delay
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
                                    108 ;	main.c: 13: }
      008040 5B 02            [ 2]  109 	addw	sp, #2
      008042 81               [ 4]  110 	ret
                                    111 ;	main.c: 15: void main() {
                                    112 ;	-----------------------------------------
                                    113 ;	 function main
                                    114 ;	-----------------------------------------
      008043                        115 _main:
                                    116 ;	main.c: 17: PA_DDR |= (1 << LED_R);
      008043 72 16 50 02      [ 1]  117 	bset	0x5002, #3
                                    118 ;	main.c: 18: PA_CR1 |= (1 << LED_R);
      008047 72 16 50 03      [ 1]  119 	bset	0x5003, #3
                                    120 ;	main.c: 21: PD_DDR |= (1 << LED_G);
      00804B 72 10 50 11      [ 1]  121 	bset	0x5011, #0
                                    122 ;	main.c: 22: PD_CR1 |= (1 << LED_G);
      00804F 72 10 50 12      [ 1]  123 	bset	0x5012, #0
                                    124 ;	main.c: 25: PB_DDR |= (1 << LED_B);
      008053 72 10 50 07      [ 1]  125 	bset	0x5007, #0
                                    126 ;	main.c: 26: PB_CR1 |= (1 << LED_B);
      008057 72 10 50 08      [ 1]  127 	bset	0x5008, #0
                                    128 ;	main.c: 29: PB_DDR |= (1 << LED_DIODE);
      00805B 72 12 50 07      [ 1]  129 	bset	0x5007, #1
                                    130 ;	main.c: 30: PB_CR1 |= (1 << LED_DIODE);
      00805F 72 12 50 08      [ 1]  131 	bset	0x5008, #1
                                    132 ;	main.c: 33: PA_DDR |= (1 << LED_CAP);
      008063 72 10 50 02      [ 1]  133 	bset	0x5002, #0
                                    134 ;	main.c: 34: PA_CR1 |= (1 << LED_CAP);
      008067 72 10 50 03      [ 1]  135 	bset	0x5003, #0
                                    136 ;	main.c: 36: while(1) {
      00806B                        137 00102$:
                                    138 ;	main.c: 37: PA_ODR |= (1 << LED_CAP); // charging the cap
      00806B 72 10 50 00      [ 1]  139 	bset	0x5000, #0
                                    140 ;	main.c: 39: PD_ODR |= (1 << LED_B); // selecting diode
      00806F 72 10 50 0F      [ 1]  141 	bset	0x500f, #0
                                    142 ;	main.c: 40: PB_ODR &= ~(1 << LED_G); // selecting diode
      008073 72 11 50 05      [ 1]  143 	bres	0x5005, #0
                                    144 ;	main.c: 42: PB_ODR &= ~(1 << LED_DIODE); // discharging through diode
      008077 72 13 50 05      [ 1]  145 	bres	0x5005, #1
                                    146 ;	main.c: 44: PB_ODR |= (1 << LED_DIODE); 
      00807B 72 12 50 05      [ 1]  147 	bset	0x5005, #1
                                    148 ;	main.c: 45: delay();
      00807F CD 80 2D         [ 4]  149 	call	_delay
                                    150 ;	main.c: 46: PB_ODR |= (1 << LED_B); // selecting diode
      008082 72 10 50 05      [ 1]  151 	bset	0x5005, #0
                                    152 ;	main.c: 47: PD_ODR &= ~(1 << LED_G); // selecting diode
      008086 72 11 50 0F      [ 1]  153 	bres	0x500f, #0
                                    154 ;	main.c: 48: delay();
      00808A CD 80 2D         [ 4]  155 	call	_delay
      00808D 20 DC            [ 2]  156 	jra	00102$
                                    157 ;	main.c: 50: }
      00808F 81               [ 4]  158 	ret
                                    159 	.area CODE
                                    160 	.area CONST
                                    161 	.area INITIALIZER
                                    162 	.area CABS (ABS)
