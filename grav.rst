                                      1 ;--------------------------------------------------------
                                      2 ; File Created by SDCC : free open source ISO C Compiler 
                                      3 ; Version 4.4.0 #14620 (Linux)
                                      4 ;--------------------------------------------------------
                                      5 	.module grav
                                      6 	.optsdcc -mstm8
                                      7 	
                                      8 ;--------------------------------------------------------
                                      9 ; Public variables in this module
                                     10 ;--------------------------------------------------------
                                     11 	.globl _main
                                     12 	.globl _write_reg
                                     13 	.globl _read_reg
                                     14 	.globl _spi_xfer
                                     15 	.globl _delay
                                     16 ;--------------------------------------------------------
                                     17 ; ram data
                                     18 ;--------------------------------------------------------
                                     19 	.area DATA
                                     20 ;--------------------------------------------------------
                                     21 ; ram data
                                     22 ;--------------------------------------------------------
                                     23 	.area INITIALIZED
                                     24 ;--------------------------------------------------------
                                     25 ; Stack segment in internal ram
                                     26 ;--------------------------------------------------------
                                     27 	.area SSEG
      000001                         28 __start__stack:
      000001                         29 	.ds	1
                                     30 
                                     31 ;--------------------------------------------------------
                                     32 ; absolute external ram data
                                     33 ;--------------------------------------------------------
                                     34 	.area DABS (ABS)
                                     35 
                                     36 ; default segment ordering for linker
                                     37 	.area HOME
                                     38 	.area GSINIT
                                     39 	.area GSFINAL
                                     40 	.area CONST
                                     41 	.area INITIALIZER
                                     42 	.area CODE
                                     43 
                                     44 ;--------------------------------------------------------
                                     45 ; interrupt vector
                                     46 ;--------------------------------------------------------
                                     47 	.area HOME
      008000                         48 __interrupt_vect:
      008000 82 00 80 07             49 	int s_GSINIT ; reset
                                     50 ;--------------------------------------------------------
                                     51 ; global & static initialisations
                                     52 ;--------------------------------------------------------
                                     53 	.area HOME
                                     54 	.area GSINIT
                                     55 	.area GSFINAL
                                     56 	.area GSINIT
      008007 CD 81 08         [ 4]   57 	call	___sdcc_external_startup
      00800A 4D               [ 1]   58 	tnz	a
      00800B 27 03            [ 1]   59 	jreq	__sdcc_init_data
      00800D CC 80 04         [ 2]   60 	jp	__sdcc_program_startup
      008010                         61 __sdcc_init_data:
                                     62 ; stm8_genXINIT() start
      008010 AE 00 00         [ 2]   63 	ldw x, #l_DATA
      008013 27 07            [ 1]   64 	jreq	00002$
      008015                         65 00001$:
      008015 72 4F 00 00      [ 1]   66 	clr (s_DATA - 1, x)
      008019 5A               [ 2]   67 	decw x
      00801A 26 F9            [ 1]   68 	jrne	00001$
      00801C                         69 00002$:
      00801C AE 00 00         [ 2]   70 	ldw	x, #l_INITIALIZER
      00801F 27 09            [ 1]   71 	jreq	00004$
      008021                         72 00003$:
      008021 D6 80 2C         [ 1]   73 	ld	a, (s_INITIALIZER - 1, x)
      008024 D7 00 00         [ 1]   74 	ld	(s_INITIALIZED - 1, x), a
      008027 5A               [ 2]   75 	decw	x
      008028 26 F7            [ 1]   76 	jrne	00003$
      00802A                         77 00004$:
                                     78 ; stm8_genXINIT() end
                                     79 	.area GSFINAL
      00802A CC 80 04         [ 2]   80 	jp	__sdcc_program_startup
                                     81 ;--------------------------------------------------------
                                     82 ; Home
                                     83 ;--------------------------------------------------------
                                     84 	.area HOME
                                     85 	.area HOME
      008004                         86 __sdcc_program_startup:
      008004 CC 80 9D         [ 2]   87 	jp	_main
                                     88 ;	return from main will return to caller
                                     89 ;--------------------------------------------------------
                                     90 ; code
                                     91 ;--------------------------------------------------------
                                     92 	.area CODE
                                     93 ;	grav.c: 23: void delay(uint32_t n) {
                                     94 ;	-----------------------------------------
                                     95 ;	 function delay
                                     96 ;	-----------------------------------------
      00802D                         97 _delay:
      00802D 52 08            [ 2]   98 	sub	sp, #8
                                     99 ;	grav.c: 24: while (n--) { __asm__("nop"); }
      00802F 16 0D            [ 2]  100 	ldw	y, (0x0d, sp)
      008031 17 07            [ 2]  101 	ldw	(0x07, sp), y
      008033 1E 0B            [ 2]  102 	ldw	x, (0x0b, sp)
      008035                        103 00101$:
      008035 1F 01            [ 2]  104 	ldw	(0x01, sp), x
      008037 7B 07            [ 1]  105 	ld	a, (0x07, sp)
      008039 6B 03            [ 1]  106 	ld	(0x03, sp), a
      00803B 7B 08            [ 1]  107 	ld	a, (0x08, sp)
      00803D 16 07            [ 2]  108 	ldw	y, (0x07, sp)
      00803F 72 A2 00 01      [ 2]  109 	subw	y, #0x0001
      008043 17 07            [ 2]  110 	ldw	(0x07, sp), y
      008045 24 01            [ 1]  111 	jrnc	00121$
      008047 5A               [ 2]  112 	decw	x
      008048                        113 00121$:
      008048 4D               [ 1]  114 	tnz	a
      008049 26 08            [ 1]  115 	jrne	00122$
      00804B 16 02            [ 2]  116 	ldw	y, (0x02, sp)
      00804D 26 04            [ 1]  117 	jrne	00122$
      00804F 0D 01            [ 1]  118 	tnz	(0x01, sp)
      008051 27 03            [ 1]  119 	jreq	00104$
      008053                        120 00122$:
      008053 9D               [ 1]  121 	nop
      008054 20 DF            [ 2]  122 	jra	00101$
      008056                        123 00104$:
                                    124 ;	grav.c: 25: }
      008056 1E 09            [ 2]  125 	ldw	x, (9, sp)
      008058 5B 0E            [ 2]  126 	addw	sp, #14
      00805A FC               [ 2]  127 	jp	(x)
                                    128 ;	grav.c: 27: uint8_t spi_xfer(uint8_t data) {
                                    129 ;	-----------------------------------------
                                    130 ;	 function spi_xfer
                                    131 ;	-----------------------------------------
      00805B                        132 _spi_xfer:
                                    133 ;	grav.c: 28: SPI1_DR = data;
      00805B C7 52 04         [ 1]  134 	ld	0x5204, a
                                    135 ;	grav.c: 29: while (!(SPI1_SR & 0x02)); // Wait for TXE (Transmit buffer empty)
      00805E                        136 00101$:
      00805E 72 03 52 03 FB   [ 2]  137 	btjf	0x5203, #1, 00101$
                                    138 ;	grav.c: 30: while (!(SPI1_SR & 0x01)); // Wait for RXNE (Receive buffer not empty)
      008063                        139 00104$:
      008063 72 01 52 03 FB   [ 2]  140 	btjf	0x5203, #0, 00104$
                                    141 ;	grav.c: 31: return SPI1_DR;
      008068 C6 52 04         [ 1]  142 	ld	a, 0x5204
                                    143 ;	grav.c: 32: }
      00806B 81               [ 4]  144 	ret
                                    145 ;	grav.c: 34: uint8_t read_reg(uint8_t reg) {
                                    146 ;	-----------------------------------------
                                    147 ;	 function read_reg
                                    148 ;	-----------------------------------------
      00806C                        149 _read_reg:
      00806C 97               [ 1]  150 	ld	xl, a
                                    151 ;	grav.c: 36: PB_ODR &= ~(1 << 4);        // CS LOW
      00806D 72 19 50 05      [ 1]  152 	bres	0x5005, #4
                                    153 ;	grav.c: 37: spi_xfer(reg | 0x80);       // Bit 7 is 1 for Read
      008071 9F               [ 1]  154 	ld	a, xl
      008072 AA 80            [ 1]  155 	or	a, #0x80
      008074 CD 80 5B         [ 4]  156 	call	_spi_xfer
                                    157 ;	grav.c: 38: val = spi_xfer(0x00);       // Dummy byte to get data
      008077 4F               [ 1]  158 	clr	a
      008078 CD 80 5B         [ 4]  159 	call	_spi_xfer
      00807B 97               [ 1]  160 	ld	xl, a
                                    161 ;	grav.c: 39: PB_ODR |= (1 << 4);         // CS HIGH
      00807C 72 18 50 05      [ 1]  162 	bset	0x5005, #4
                                    163 ;	grav.c: 40: return val;
      008080 9F               [ 1]  164 	ld	a, xl
                                    165 ;	grav.c: 41: }
      008081 81               [ 4]  166 	ret
                                    167 ;	grav.c: 43: void write_reg(uint8_t reg, uint8_t val) {
                                    168 ;	-----------------------------------------
                                    169 ;	 function write_reg
                                    170 ;	-----------------------------------------
      008082                        171 _write_reg:
      008082 97               [ 1]  172 	ld	xl, a
                                    173 ;	grav.c: 44: PB_ODR &= ~(1 << 4);        // CS LOW
      008083 72 19 50 05      [ 1]  174 	bres	0x5005, #4
                                    175 ;	grav.c: 45: spi_xfer(reg & 0x7F);       // Bit 7 is 0 for Write
      008087 9F               [ 1]  176 	ld	a, xl
      008088 A4 7F            [ 1]  177 	and	a, #0x7f
      00808A CD 80 5B         [ 4]  178 	call	_spi_xfer
                                    179 ;	grav.c: 46: spi_xfer(val);
      00808D 7B 03            [ 1]  180 	ld	a, (0x03, sp)
      00808F CD 80 5B         [ 4]  181 	call	_spi_xfer
                                    182 ;	grav.c: 47: PB_ODR |= (1 << 4);         // CS HIGH
      008092 C6 50 05         [ 1]  183 	ld	a, 0x5005
      008095 AA 10            [ 1]  184 	or	a, #0x10
      008097 C7 50 05         [ 1]  185 	ld	0x5005, a
                                    186 ;	grav.c: 48: }
      00809A 85               [ 2]  187 	popw	x
      00809B 84               [ 1]  188 	pop	a
      00809C FC               [ 2]  189 	jp	(x)
                                    190 ;	grav.c: 50: void main(void) {
                                    191 ;	-----------------------------------------
                                    192 ;	 function main
                                    193 ;	-----------------------------------------
      00809D                        194 _main:
      00809D 88               [ 1]  195 	push	a
                                    196 ;	grav.c: 52: CLK_PCKENR1 |= (1 << 4);
      00809E 72 18 50 C7      [ 1]  197 	bset	0x50c7, #4
                                    198 ;	grav.c: 55: PA_DDR |= (1 << 3);
      0080A2 72 16 50 02      [ 1]  199 	bset	0x5002, #3
                                    200 ;	grav.c: 56: PA_CR1 |= (1 << 3);
      0080A6 72 16 50 03      [ 1]  201 	bset	0x5003, #3
                                    202 ;	grav.c: 60: PB_DDR |= (1 << 4) | (1 << 5) | (1 << 6);
      0080AA C6 50 07         [ 1]  203 	ld	a, 0x5007
      0080AD AA 70            [ 1]  204 	or	a, #0x70
      0080AF C7 50 07         [ 1]  205 	ld	0x5007, a
                                    206 ;	grav.c: 61: PB_CR1 |= (1 << 4) | (1 << 5) | (1 << 6);
      0080B2 C6 50 08         [ 1]  207 	ld	a, 0x5008
      0080B5 AA 70            [ 1]  208 	or	a, #0x70
      0080B7 C7 50 08         [ 1]  209 	ld	0x5008, a
                                    210 ;	grav.c: 62: PB_ODR |= (1 << 4); // CS High
      0080BA 72 18 50 05      [ 1]  211 	bset	0x5005, #4
                                    212 ;	grav.c: 67: SPI1_CR1 = (1 << 2) | (0x04 << 3) | (1 << 6);
      0080BE 35 64 52 00      [ 1]  213 	mov	0x5200+0, #0x64
                                    214 ;	grav.c: 68: SPI1_CR2 = (1 << 1) | (1 << 0); // SSM = 1, SSI = 1
      0080C2 35 03 52 01      [ 1]  215 	mov	0x5201+0, #0x03
                                    216 ;	grav.c: 70: delay(200000); // Wait for sensor to wake up (~20ms)
      0080C6 4B 40            [ 1]  217 	push	#0x40
      0080C8 4B 0D            [ 1]  218 	push	#0x0d
      0080CA 4B 03            [ 1]  219 	push	#0x03
      0080CC 4B 00            [ 1]  220 	push	#0x00
      0080CE CD 80 2D         [ 4]  221 	call	_delay
                                    222 ;	grav.c: 75: write_reg(LIS2_CTRL1, 0x44);
      0080D1 4B 44            [ 1]  223 	push	#0x44
      0080D3 A6 20            [ 1]  224 	ld	a, #0x20
      0080D5 CD 80 82         [ 4]  225 	call	_write_reg
                                    226 ;	grav.c: 77: while (1) {
      0080D8                        227 00106$:
                                    228 ;	grav.c: 79: uint8_t temp = read_reg(LIS2_OUT_T_H);
      0080D8 A6 27            [ 1]  229 	ld	a, #0x27
      0080DA CD 80 6C         [ 4]  230 	call	_read_reg
      0080DD 6B 01            [ 1]  231 	ld	(0x01, sp), a
      0080DF 97               [ 1]  232 	ld	xl, a
                                    233 ;	grav.c: 84: PA_ODR |= (1 << 3);  // Success!
      0080E0 C6 50 00         [ 1]  234 	ld	a, 0x5000
                                    235 ;	grav.c: 83: if (temp != 0x00 && temp != 0xFF) {
      0080E3 0D 01            [ 1]  236 	tnz	(0x01, sp)
      0080E5 27 0D            [ 1]  237 	jreq	00102$
      0080E7 88               [ 1]  238 	push	a
      0080E8 9F               [ 1]  239 	ld	a, xl
      0080E9 4C               [ 1]  240 	inc	a
      0080EA 84               [ 1]  241 	pop	a
      0080EB 27 07            [ 1]  242 	jreq	00102$
                                    243 ;	grav.c: 84: PA_ODR |= (1 << 3);  // Success!
      0080ED AA 08            [ 1]  244 	or	a, #0x08
      0080EF C7 50 00         [ 1]  245 	ld	0x5000, a
      0080F2 20 05            [ 2]  246 	jra	00103$
      0080F4                        247 00102$:
                                    248 ;	grav.c: 86: PA_ODR &= ~(1 << 3); // Fail
      0080F4 A4 F7            [ 1]  249 	and	a, #0xf7
      0080F6 C7 50 00         [ 1]  250 	ld	0x5000, a
      0080F9                        251 00103$:
                                    252 ;	grav.c: 89: delay(100000);
      0080F9 4B A0            [ 1]  253 	push	#0xa0
      0080FB 4B 86            [ 1]  254 	push	#0x86
      0080FD 4B 01            [ 1]  255 	push	#0x01
      0080FF 4B 00            [ 1]  256 	push	#0x00
      008101 CD 80 2D         [ 4]  257 	call	_delay
      008104 20 D2            [ 2]  258 	jra	00106$
                                    259 ;	grav.c: 91: }
      008106 84               [ 1]  260 	pop	a
      008107 81               [ 4]  261 	ret
                                    262 	.area CODE
                                    263 	.area CONST
                                    264 	.area INITIALIZER
                                    265 	.area CABS (ABS)
