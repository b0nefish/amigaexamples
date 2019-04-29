        INCLUDE "hardware/custom.i"
        INCLUDE "hardware/intbits.i"
        INCLUDE "hardware/dmabits.i"
	include	"own.i"

;;; A6 contains the $DFF000 custom chip base.

_start:	jmp	main(pc)
	dc.b	"Plasma",10
	dc.b	"Code: Pararaum / T7D",0
	align	4
	cmp.l	#"main",main
	cmp.l	#"seco",setup_copper
	cmp.l	#"COPL",copper_list
	align	4
main:	nop
	moveq	#OWN_libraries|OWN_view|OWN_trap,d0
	jsr	own_machine
	lea.l	$DFF000,a6
	bsr	setup_system
l3$:	btst	#6,$bfe001	; Left mouse clicked?
	bne.s	l3$
	jsr	disown_machine
	moveq	#0,d0
	rts
	jmp	_start


wait_frame:
l1$:	btst.b	#0,vposr+1(a6)
	bne.s	l1$
l2$:	btst.b	#0,vposr+1(a6)
	beq.s	l2$
	rts


setup_system:
	move.w	#$7fff,dmacon(a6)
	bsr	setup_copper
	move.w	#DMAF_SETCLR|DMAF_BLITTER|DMAF_COPPER|DMAF_COPPER|DMAF_RASTER|DMAF_MASTER,dmacon(a6)
	rts

setup_copper:
	;; Setup copper pointer.
	move.l	#copper_list,cop1lc(a6)
	clr.w	copjmp1(a6)	 ; Strobe copper
	move.l	#bitplane,d0	 ; Address of bitplane
	move.w	d0,copper_bplptr+6 ; Lower 16 bits.
	swap	d0
	move.w	d0,copper_bplptr+2 ; Upper 16 bits.
	rts

	SECTION	CHIP,data_c
copper_list:
	;; Copper List
	;; see: http://www.theflatnet.de/pub/cbm/amiga/AmigaDevDocs/hard_2.html
	;; AGA compatible setup?
	dc.w 	fmode,$0000
	;; Setting up display.
	;; see also: http://cyberpingui.free.fr/tuto_graphics.htm
	dc.w	$008e,$2c81,$0090,$2cc1		; DIWSTRT/DIWSTOP
	dc.w	$0092,$0038,$0094,$00d0		; DDFSTRT/DDFSTOP
	;; http://amiga-dev.wikidot.com/hardware:bplcon0
	dc.w	bplcon0,$1200			; 1 Bitplane, output enabled
copper_scrollcon:
	;; http://www.winnicki.net/amiga/memmap/BPLCON1.html, scrolling
	;; Lower nibbles are for scrolling, PF 1 and PF2.
	dc.w	bplcon1,$0000
	;; http://www.theflatnet.de/pub/cbm/amiga/AmigaDevDocs/hard_4.html
	;; http://www.winnicki.net/amiga/memmap/BPLCON2.html
	;; http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0159.html
	;; $104          FEDCBA9876543210
	dc.w	bplcon2,%0000000000000000
	;; http://www.winnicki.net/amiga/memmap/BPLCON3.html
	dc.w	bplcon3,$0000
	;; Bitplane modulos
copper_bitplane_modulos:
	dc.w	bpl1mod,$0000
	dc.w	bpl2mod,$0000
copper_bplptr:
	dc.w	$00e0,$0000,$00e2,$0000		; Bitplaneptrs
	dc.w	$00e4,$0000,$00e6,$0000
	dc.w	$00e8,$0000,$00ea,$0000
	dc.w	$00ec,$0000,$00ee,$0000
	dc.w	$00f0,$0000,$00f2,$0000
	dc.w	color+2*0,$0000	;Colour 0
	dc.w	color+2*1,$05b5	;Colour 1
	;; Each line has 752 pixels, the copper needs 4 pixel clocks per word, two words are needed, therefore every 8 pixels can be changed.
	;; Per line we have (/ 752 8)94 color changes.
	;; (* 6 15)90: we will do 6 blocks are 15 colour changes.
	;; Horizontal blanking? There is a problem there... The calculation does not seem to work... Therefore we advised the following pattern to count.
	dc.w	$ffdf,$fffe	; End of NTSC.
	dc.w	$0107,$fffe	;Wait for line 257.
	;; White marker for easy spottin.
	dc.w	color,$fff
;;; (format "%02x %02x %02x" 255 51 136)"ff 33 88"
	REPT	10		; Lines
	dc.w	color, $00FF,  color, $0001,  color, $0F02,  color, $0003,  color, $0F04,  color, $0005,  color, $0F06,  color, $0007,  color, $0F08,  color, $0009,  color, $0F0A,  color, $000B,  color, $0F0C,  color, $000D,  color, $0F0E,  color, $000F,  color, $0F10,  color, $0011,  color, $0F12,  color, $0013,  color, $0F14,  color, $0015,  color, $0F16,  color, $0017,  color, $0F18,  color, $0019,  color, $0F1A,  color, $001B,  color, $0F1C,  color, $001D,  color, $0F1E,  color, $001F,  color, $0F20,  color, $0021,  color, $0F22,  color, $0023,  color, $0F24,  color, $0025,  color, $0F26,  color, $0027,  color, $0F28,  color, $0029,  color, $0F2A,  color, $002B,  color, $0F2C,  color, $002D,  color, $0F2E,  color, $002F,  color, $0F30,  color, $0031,  color, $0F32,  color, $0033,  color, $0F34,  color, $0035,  color, $0F36,  color, $0037,  color, $0F38
	ENDR			; Lines
	;; Now make a zig-zag pattern.
	dc.w	$0f41,$fffe
	;; White marker for easy spottin.
	dc.w	color,$fff
	rept	10		; Lines
	dc.w	color, $00FF,  color, $0001,  color, $0F02,  color, $0003,  color, $0F04,  color, $0005,  color, $0F06,  color, $0007,  color, $0F08,  color, $0009,  color, $0F0A,  color, $000B,  color, $0F0C,  color, $000D,  color, $0F0E,  color, $000F,  color, $0F10,  color, $0011,  color, $0F12,  color, $0013,  color, $0F14,  color, $0015,  color, $0F16,  color, $0017,  color, $0F18,  color, $0019,  color, $0F1A,  color, $001B,  color, $0F1C,  color, $001D,  color, $0F1E,  color, $001F,  color, $0F20,  color, $0021,  color, $0F22,  color, $0023,  color, $0F24,  color, $0025,  color, $0F26,  color, $0027,  color, $0F28,  color, $0029,  color, $0F2A,  color, $002B,  color, $0F2C,  color, $002D,  color, $0F2E,  color, $002F,  color, $0F30,  color, $0031,  color, $0F32,  color, $0033,  color, $0F34,  color, $0035,  color, $0F36,  color, $0037,  color, $0F38
	dc.w	color, $0F00,  color, $0001,  color, $0F02,  color, $0003,  color, $0F04,  color, $0005,  color, $0F06,  color, $0007,  color, $0F08,  color, $0009,  color, $0F0A,  color, $000B,  color, $0F0C,  color, $000D,  color, $0F0E,  color, $000F,  color, $0F10,  color, $0011,  color, $0F12,  color, $0013,  color, $0F14,  color, $0015,  color, $0F16,  color, $0017,  color, $0F18,  color, $0019,  color, $0F1A,  color, $001B,  color, $0F1C,  color, $001D,  color, $0F1E,  color, $001F,  color, $0F20,  color, $0021,  color, $0F22,  color, $0023,  color, $0F24,  color, $0025,  color, $0F26,  color, $0027,  color, $0F28,  color, $0029,  color, $0F2A,  color, $002B,  color, $0F2C,  color, $002D,  color, $0F2E,  color, $002F,  color, $0F30,  color, $0031,  color, $0F32,  color, $0033,  color, $0F34,  color, $0035,  color, $0F36,  color, $0037
	endr			; Lines
;;; This means we have $37 (and a half) copper commands per rasterline. #x37 55. 55 commands are (* 55 8)440 440 Pixels, plus the half command this is 444 Pixels per rasterline (?).
	rept	32
	dc.l	$01feEAEA
	endr
	;; End of Copper List
	dc.w	$ffff,$fffe

bitplane:
	REPT	14
	dcb.b	40,$55
	dcb.b	40,$AA
	ENDR
	REPT	100
	dc.b	$55
	dcb.b	38
	dc.b	$55
	dc.b	$AA
	dcb.b	38
	dc.b	$AA
	ENDR
	REPT	14
	dcb.b	40,$55
	dcb.b	40,$AA
	ENDR

	SECTION BSS,bss
debugstorage:
	ds.l	1<<10
