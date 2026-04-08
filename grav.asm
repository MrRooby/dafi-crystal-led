;--------------------------------------------------------
; File Created by SDCC : free open source ISO C Compiler 
; Version 4.4.0 #14620 (Linux)
;--------------------------------------------------------
	.module grav
	.optsdcc -mstm8
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _main
	.globl _write_reg
	.globl _read_reg
	.globl _spi_xfer
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
;	grav.c: 23: void delay(uint32_t n) {
;	-----------------------------------------
;	 function delay
;	-----------------------------------------
_delay:
	sub	sp, #8
;	grav.c: 24: while (n--) { __asm__("nop"); }
	ldw	y, (0x0d, sp)
	ldw	(0x07, sp), y
	ldw	x, (0x0b, sp)
00101$:
	ldw	(0x01, sp), x
	ld	a, (0x07, sp)
	ld	(0x03, sp), a
	ld	a, (0x08, sp)
	ldw	y, (0x07, sp)
	subw	y, #0x0001
	ldw	(0x07, sp), y
	jrnc	00121$
	decw	x
00121$:
	tnz	a
	jrne	00122$
	ldw	y, (0x02, sp)
	jrne	00122$
	tnz	(0x01, sp)
	jreq	00104$
00122$:
	nop
	jra	00101$
00104$:
;	grav.c: 25: }
	ldw	x, (9, sp)
	addw	sp, #14
	jp	(x)
;	grav.c: 27: uint8_t spi_xfer(uint8_t data) {
;	-----------------------------------------
;	 function spi_xfer
;	-----------------------------------------
_spi_xfer:
;	grav.c: 28: SPI1_DR = data;
	ld	0x5204, a
;	grav.c: 29: while (!(SPI1_SR & 0x02)); // Wait for TXE (Transmit buffer empty)
00101$:
	btjf	0x5203, #1, 00101$
;	grav.c: 30: while (!(SPI1_SR & 0x01)); // Wait for RXNE (Receive buffer not empty)
00104$:
	btjf	0x5203, #0, 00104$
;	grav.c: 31: return SPI1_DR;
	ld	a, 0x5204
;	grav.c: 32: }
	ret
;	grav.c: 34: uint8_t read_reg(uint8_t reg) {
;	-----------------------------------------
;	 function read_reg
;	-----------------------------------------
_read_reg:
	ld	xl, a
;	grav.c: 36: PB_ODR &= ~(1 << 4);        // CS LOW
	bres	0x5005, #4
;	grav.c: 37: spi_xfer(reg | 0x80);       // Bit 7 is 1 for Read
	ld	a, xl
	or	a, #0x80
	call	_spi_xfer
;	grav.c: 38: val = spi_xfer(0x00);       // Dummy byte to get data
	clr	a
	call	_spi_xfer
	ld	xl, a
;	grav.c: 39: PB_ODR |= (1 << 4);         // CS HIGH
	bset	0x5005, #4
;	grav.c: 40: return val;
	ld	a, xl
;	grav.c: 41: }
	ret
;	grav.c: 43: void write_reg(uint8_t reg, uint8_t val) {
;	-----------------------------------------
;	 function write_reg
;	-----------------------------------------
_write_reg:
	ld	xl, a
;	grav.c: 44: PB_ODR &= ~(1 << 4);        // CS LOW
	bres	0x5005, #4
;	grav.c: 45: spi_xfer(reg & 0x7F);       // Bit 7 is 0 for Write
	ld	a, xl
	and	a, #0x7f
	call	_spi_xfer
;	grav.c: 46: spi_xfer(val);
	ld	a, (0x03, sp)
	call	_spi_xfer
;	grav.c: 47: PB_ODR |= (1 << 4);         // CS HIGH
	ld	a, 0x5005
	or	a, #0x10
	ld	0x5005, a
;	grav.c: 48: }
	popw	x
	pop	a
	jp	(x)
;	grav.c: 50: void main(void) {
;	-----------------------------------------
;	 function main
;	-----------------------------------------
_main:
	push	a
;	grav.c: 52: CLK_PCKENR1 |= (1 << 4);
	bset	0x50c7, #4
;	grav.c: 55: PA_DDR |= (1 << 3);
	bset	0x5002, #3
;	grav.c: 56: PA_CR1 |= (1 << 3);
	bset	0x5003, #3
;	grav.c: 60: PB_DDR |= (1 << 4) | (1 << 5) | (1 << 6);
	ld	a, 0x5007
	or	a, #0x70
	ld	0x5007, a
;	grav.c: 61: PB_CR1 |= (1 << 4) | (1 << 5) | (1 << 6);
	ld	a, 0x5008
	or	a, #0x70
	ld	0x5008, a
;	grav.c: 62: PB_ODR |= (1 << 4); // CS High
	bset	0x5005, #4
;	grav.c: 67: SPI1_CR1 = (1 << 2) | (0x04 << 3) | (1 << 6);
	mov	0x5200+0, #0x64
;	grav.c: 68: SPI1_CR2 = (1 << 1) | (1 << 0); // SSM = 1, SSI = 1
	mov	0x5201+0, #0x03
;	grav.c: 70: delay(200000); // Wait for sensor to wake up (~20ms)
	push	#0x40
	push	#0x0d
	push	#0x03
	push	#0x00
	call	_delay
;	grav.c: 75: write_reg(LIS2_CTRL1, 0x44);
	push	#0x44
	ld	a, #0x20
	call	_write_reg
;	grav.c: 77: while (1) {
00106$:
;	grav.c: 79: uint8_t temp = read_reg(LIS2_OUT_T_H);
	ld	a, #0x27
	call	_read_reg
	ld	(0x01, sp), a
	ld	xl, a
;	grav.c: 84: PA_ODR |= (1 << 3);  // Success!
	ld	a, 0x5000
;	grav.c: 83: if (temp != 0x00 && temp != 0xFF) {
	tnz	(0x01, sp)
	jreq	00102$
	push	a
	ld	a, xl
	inc	a
	pop	a
	jreq	00102$
;	grav.c: 84: PA_ODR |= (1 << 3);  // Success!
	or	a, #0x08
	ld	0x5000, a
	jra	00103$
00102$:
;	grav.c: 86: PA_ODR &= ~(1 << 3); // Fail
	and	a, #0xf7
	ld	0x5000, a
00103$:
;	grav.c: 89: delay(100000);
	push	#0xa0
	push	#0x86
	push	#0x01
	push	#0x00
	call	_delay
	jra	00106$
;	grav.c: 91: }
	pop	a
	ret
	.area CODE
	.area CONST
	.area INITIALIZER
	.area CABS (ABS)
