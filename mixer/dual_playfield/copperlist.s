	include "hardware/custom.i"

	XDEF	_copperlist
	XDEF	_wait_for_mouse
	XDEF	_copperlist_blit_a_ptr
	XDEF	_copperlist_blit_d_ptr
	XDEF	_copperlist_blit_modulos

	SECTION CODE,code
;;; Wait for mouse button.
;;; Destroys: A0
_wait_for_mouse:
        lea     $BFE001,a0              ;CIA A
l$:     btst    #6,(a0)
        bne.b   l$
        rts

	SECTION	CHIP,data_c

_copperlist:
	dc.w	bplpt,0
	dc.w	bplpt+2,0
	dc.w	bplpt+4,0
	dc.w	bplpt+6,0
	dc.w	bplpt+8,0
	dc.w	bplpt+10,0
	dc.w	bplpt+12,0
	dc.w	bplpt+14,0
	dc.w	bplpt+16,0
	dc.w	bplpt+18,0
_copperlist_colors:
	dc.w	color+0,$0000
	dc.w	color+2,$0010
	dc.w	color+4,$0020
	dc.w	color+6,$0030
	dc.w	color+8,$0040
	dc.w	color+$a,$0050
	dc.w	color+$c,$0060
	dc.w	color+$e,$0070
	dc.w	color+$10,$0080
	dc.w	color+$12,$0090
	dc.w	color+$14,$00a0
	dc.w	color+$16,$00b0
	dc.w	color+$18,$00c0
	dc.w	color+$1a,$00d0
	dc.w	color+$1c,$00e0
	dc.w	color+$1e,$00f0
	dc.w	$2f07,$fffe
	dc.w	color+0,$0fee
	;; Wait for a visible line and wait for blitter!
	dc.w	$3007,$7ffe
	dc.w	color+0,$0f00	;Turn to red.
	;; Blitter activity!
	dc.w	bltcon0,$29f0
	dc.w	bltcon1,$0002
	dc.w	bltafwm,$fffe	;First word mask
	dc.w	bltalwm,$7fff	;Last word mask
_copperlist_blit_a_ptr:
	dc.w	bltapt,0
	dc.w	bltapt+2,0
_copperlist_blit_d_ptr:
	dc.w	bltdpt,0
	dc.w	bltdpt+2,0
_copperlist_blit_modulos:
	dc.w	bltamod,0
	dc.w	bltdmod,0
	;; H9-H0, W5-W0. Width is in words.
	dc.w	bltsize,((3*200)<<6)|(320/16)
	;; Wait for the next line and wait for blitter!.
	dc.w	$3107,$7ffe
	dc.w	color+0,$0fff	;Turn to white.
	dc.w	$fa07,$fffe
	dc.w	color+0,$0888
	dc.l	$FFFFFFFE
