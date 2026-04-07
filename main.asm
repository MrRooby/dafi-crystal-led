;--------------------------------------------------------
; File Created by SDCC : free open source ISO C Compiler 
; Version 4.4.0 #14620 (Linux)
;--------------------------------------------------------
	.module main
	.optsdcc -mstm8
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _main
	.globl _delay
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area DATA
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area INITIALIZED
;--------------------------------------------------------
; Stack segment in internal ram
;--------------------------------------------------------
	.area SSEG
__start__stack:
	.ds	1

;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area DABS (ABS)

; default segment ordering for linker
	.area HOME
	.area GSINIT
	.area GSFINAL
	.area CONST
	.area INITIALIZER
	.area CODE

;--------------------------------------------------------
; interrupt vector
;--------------------------------------------------------
	.area HOME
__interrupt_vect:
	int s_GSINIT ; reset
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area HOME
	.area GSINIT
	.area GSFINAL
	.area GSINIT
	call	___sdcc_external_startup
	tnz	a
	jreq	__sdcc_init_data
	jp	__sdcc_program_startup
__sdcc_init_data:
; stm8_genXINIT() start
	ldw x, #l_DATA
	jreq	00002$
00001$:
	clr (s_DATA - 1, x)
	decw x
	jrne	00001$
00002$:
	ldw	x, #l_INITIALIZER
	jreq	00004$
00003$:
	ld	a, (s_INITIALIZER - 1, x)
	ld	(s_INITIALIZED - 1, x), a
	decw	x
	jrne	00003$
00004$:
; stm8_genXINIT() end
	.area GSFINAL
	jp	__sdcc_program_startup
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area HOME
	.area HOME
__sdcc_program_startup:
	jp	_main
;	return from main will return to caller
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area CODE
;	main.c: 10: void delay(void) {
;	-----------------------------------------
;	 function delay
;	-----------------------------------------
_delay:
	sub	sp, #2
;	main.c: 12: for(i=0; i<50; i++){;} // crude delay
	clrw	x
	ldw	(0x01, sp), x
00103$:
	ldw	x, (0x01, sp)
	cpw	x, #0x0032
	jrnc	00105$
	ldw	x, (0x01, sp)
	incw	x
	ldw	(0x01, sp), x
	jra	00103$
00105$:
;	main.c: 13: }
	addw	sp, #2
	ret
;	main.c: 15: void main() {
;	-----------------------------------------
;	 function main
;	-----------------------------------------
_main:
;	main.c: 17: GPIOA_DDR |= (1 << LED_R);
	bset	0x5002, #3
;	main.c: 18: GPIOA_CR1 |= (1 << LED_R);
	bset	0x5003, #3
;	main.c: 21: GPIOD_DDR |= (1 << LED_G);
	bset	0x5011, #0
;	main.c: 22: GPIOD_CR1 |= (1 << LED_G);
	bset	0x5012, #0
;	main.c: 25: GPIOB_DDR |= (1 << LED_B);
	bset	0x5007, #0
;	main.c: 26: GPIOB_CR1 |= (1 << LED_B);
	bset	0x5008, #0
;	main.c: 29: GPIOB_DDR |= (1 << LED_DIODE);
	bset	0x5007, #1
;	main.c: 30: GPIOB_CR1 |= (1 << LED_DIODE);
	bset	0x5008, #1
;	main.c: 33: GPIOA_DDR |= (1 << LED_CAP);
	bset	0x5002, #0
;	main.c: 34: GPIOA_CR1 |= (1 << LED_CAP);
	bset	0x5003, #0
;	main.c: 36: while(1) {
00102$:
;	main.c: 37: GPIOA_ODR |= (1 << LED_CAP); // charging the cap
	bset	0x5000, #0
;	main.c: 39: GPIOB_ODR |= (1 << LED_B); // selecting diode
	bset	0x5005, #0
;	main.c: 40: GPIOD_ODR &= ~(1 << LED_G); // selecting diode
	bres	0x500f, #0
;	main.c: 41: GPIOB_ODR &= ~(1 << LED_DIODE); // discharging through diode
	bres	0x5005, #1
;	main.c: 43: GPIOB_ODR |= (1 << LED_DIODE); 
	bset	0x5005, #1
	jra	00102$
;	main.c: 45: }
	ret
	.area CODE
	.area CONST
	.area INITIALIZER
	.area CABS (ABS)
