#include <stdio.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include "own.h"

#define BPLWIDTH 320
#define BPLHEIGHT 256
#define BPLNO 1

#define WAITBLIT while(custom.dmaconr & (1 << 14));

extern void set_irq(__reg("a0") void (*irqhandler)(void));
extern UWORD liberation_single_column_png[];

typedef struct Bitplaneinformation {
  unsigned char *bitplanedata[2];
  unsigned short bplidx; //!< Bitplane Index, which bitplane to use.
  unsigned char *row_addresses[2][BPLHEIGHT];
} Bitplaneinformation_t;

extern volatile struct Custom custom;
static UWORD __chip very_simple_copperlist[] = {
  0xe0, 0, /* Bitplane pointer */
  0xe2, 0,
  0xe4, 0,
  0xe6, 0,
  0x180, 0x0fff, // Background white
  0x182, 0x0125, // Foreground black/blue
  0xffff, 0xfffe
};
static unsigned char __chip bitplanedata1[BPLWIDTH/8 * BPLHEIGHT * BPLNO];
static unsigned char __chip bitplanedata2[BPLWIDTH/8 * BPLHEIGHT * BPLNO];
static Bitplaneinformation_t bplinfo = {
  { &bitplanedata1[0], &bitplanedata2[0] }
};


/*
GHCI:
let angles = [i*2*pi/4096 | i <- [0..4095]]
map (round . (\i -> 51 * sin i)) angles
*/
static signed char long_sinusdat[] = {0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,17,17,17,17,17,17,18,18,18,18,18,18,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,20,20,20,20,20,20,20,21,21,21,21,21,21,21,21,21,21,21,21,21,21,22,22,22,22,22,22,22,22,22,22,22,22,22,22,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,24,24,24,24,24,24,24,24,24,24,24,24,24,24,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,37,37,37,37,37,37,37,37,37,37,37,37,37,37,37,37,37,37,37,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,37,37,37,37,37,37,37,37,37,37,37,37,37,37,37,37,37,37,37,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,24,24,24,24,24,24,24,24,24,24,24,24,24,24,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,22,22,22,22,22,22,22,22,22,22,22,22,22,22,21,21,21,21,21,21,21,21,21,21,21,21,21,21,20,20,20,20,20,20,20,20,20,20,20,20,20,20,19,19,19,19,19,19,19,19,19,19,19,19,19,19,18,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-14,-14,-14,-14,-14,-14,-14,-14,-14,-14,-14,-14,-14,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-16,-16,-16,-16,-16,-16,-16,-16,-16,-16,-16,-16,-16,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-18,-18,-18,-18,-18,-18,-18,-18,-18,-18,-18,-18,-18,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-27,-27,-27,-27,-27,-27,-27,-27,-27,-27,-27,-27,-27,-27,-27,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-51,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-49,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-48,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-47,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-46,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-45,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-44,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-43,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-42,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-41,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-39,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-38,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-37,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-36,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-35,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-34,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-27,-27,-27,-27,-27,-27,-27,-27,-27,-27,-27,-27,-27,-27,-27,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-18,-18,-18,-18,-18,-18,-18,-18,-18,-18,-18,-18,-18,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-16,-16,-16,-16,-16,-16,-16,-16,-16,-16,-16,-16,-16,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-14,-14,-14,-14,-14,-14,-14,-14,-14,-14,-14,-14,-14,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,0,0,0,0,0};

static void set_bitplane_ptr(const Bitplaneinformation_t *bplinfo) {
  ULONG bplptr;

  bplptr = (ULONG)(&(bplinfo->row_addresses[bplinfo->bplidx][0][0]));
  very_simple_copperlist[1] = bplptr >> 16;
  very_simple_copperlist[3] = bplptr & 0xFFFF;
}

/*! \brief Setup the custom chip registers
 *
 * This will set up the display window and other stuff and it will set
 * the copper list one pointer.
 *
 * @param coplist pointer to copper list.
 */
static void setup_custom_chips(UWORD *coplist) {
  custom.cop1lc = (ULONG)coplist;
  custom.copjmp1 = 0;
  custom.diwstrt = 0x2c81;
  custom.diwstop = 0x2cc1;
  custom.ddfstrt = 0x0038;
  custom.ddfstop = 0x00d0;
  custom.bplcon0 = 0x0200 | (BPLNO * 0x1000);
  custom.bplcon1 = 0;
  custom.bplcon2 = 0;
  custom.bplcon3 = 0;
  custom.bpl1mod = 0;
  custom.bpl2mod = 0;
  custom.fmode = 0;
}

static void wait4mouse(void) {
  volatile UBYTE *cia = (UBYTE *)0xBFE001;

  while((*cia & (1 << 6)) != 0) ;
}

static void xor_pixel(Bitplaneinformation_t *bplinfo, short int x, short int y) {
  UBYTE *bplptr;
  unsigned short int bplidx = bplinfo->bplidx;
  
  // Multiply by the number of bytes in each row: bplptr += (y * (BPLWIDTH/8));
  bplptr = bplinfo->row_addresses[bplidx][y];
  // Advance in order to point the byte which contains the pixel.
  bplptr += x / 8;
  *bplptr ^= 1 << (7 - (x & 7));
}

static inline short int sinus(unsigned short int phi) {
  return long_sinusdat[phi & (sizeof(long_sinusdat) - 1)];
}

static void draw_vline(Bitplaneinformation_t *bplptr, unsigned short pattern, int x1, int y1) {
  UBYTE *rowaddr;
  unsigned short deltarow;
  unsigned short bltcon1 = ((0xf) << 12) | 1; // Bit0 = line drawing mode.
  /* 
   * The BSH? bits in BLTCON1 define where the line pattern starts
   * (first bit). If we set this to the MSB (aka bit 15) then the line
   * pattern will start immediately. This is what we want.
   *
   * See http://www.winnicki.net/amiga/memmap/LineMode.html.
   */
  const int dmax = 15; // Pixels down.
  const int dmin = 0;
  int slope;

  // Get the octant [http://www.winnicki.net/amiga/memmap/LineMode.html].
  // For a line straight down, the octant is 0 (or maybe 2?).
  bltcon1 |= 3 << 2;
  slope = (4 * dmin) - (2 * dmax);
  /* Calculate the position in the bitplane here.*/
  rowaddr = bplptr->row_addresses[bplptr->bplidx][y1]; // Get offset of row.
  deltarow = x1/8; // Advance to the corresponding byte (in X).
  rowaddr += deltarow & (-2); // We need the word.;
 WAITBLIT;
  /* The article in
   * http://www.stashofcode.fr/coder-une-cracktro-sur-amiga-1/ has the
   * right formula for bltamod, the text in
   * http://www.winnicki.net/amiga/memmap/LineMode.html seems to be
   * wrong! And
   * http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0128.html
   * has it right again...
   */
  custom.bltamod = 4 * (dmin - dmax);
  custom.bltbmod = 4 * dmin;
  custom.bltapt = (void *)slope;
  if(slope < 0) {
    bltcon1 |= 1 << 6;
  }
  custom.bltcon1 = bltcon1;
  custom.bltbdat = pattern; // 16 bits for the line pattern.
  custom.bltcmod = BPLWIDTH/8;
  custom.bltdmod = BPLWIDTH/8;
  custom.bltcpt = rowaddr;
  custom.bltdpt = rowaddr;
  /*
   * A: Bit to set in line
   * B: line pattern
   * C: original surface
   * D=AB + A̅C
   * D=AB + C
   * Venn
   * AB: 6 7
   * ~AC: 1 3 (0 from ~A?)
   * -> 11001011 
   */
  custom.bltcon0 = ((x1 & 0xF) << 12)
    | 0x0b00 /* Channels to use. */
    /* | 0x004a; /\* XOR *\/ */
    | 0x00CA; /* OR */
  custom.bltadat = 0x8000;
  custom.bltafwm = -1;
  custom.bltalwm = -1;
  custom.bltsize = dmax * 64 + 66;
}


static void clear_bitplane(unsigned char *btplptr, int width, int height) {
  WAITBLIT;
  custom.bltcon0 = 0x0100; // Only D channel, no logic function!
  custom.bltcon1 = 0x0000; // No B shift, normal mode.
  /* A first word mask; The first word in a line a seen by the
     blitter. */
  custom.bltafwm = 0;
  /* A last word mask */
  custom.bltalwm = 0;
  /* channel D pointer */
  custom.bltdpt = btplptr;
  custom.bltamod = 0; // No modulo.
  custom.bltdmod = 0;
  /* H9-H0, W5-W0; width is in words. By writing the size into the
     custom chip register the blit begins and continues while the cpu
     is still running. */
  custom.bltsize = (height << 6) | (width);
}


static void do_da_sinus(Bitplaneinformation_t *bplinfo) {
  unsigned short i;
  static unsigned short t = 0;

  clear_bitplane(bplinfo->row_addresses[bplinfo->bplidx][10],
		 BPLWIDTH/8, 100);
  custom.color[0] = 0x00ff;
  for(i = 0; i < 140; i += 1) {
    draw_vline(bplinfo, liberation_single_column_png[i], i, sinus(t+2*i) + 75);
  }
  t += 17;
  custom.color[0] = 0x000f;
}

static void irqhandler(void) {
  custom.color[0] = 0x09f9;
  do_da_sinus(&bplinfo);
  custom.color[0] = 0x0faa;
}

int main(int argc, char **argv) {
  puts("Sinus-Scroller by Pararaum / T7D");
  printf("bitplanedata[0] $%lX\n", (ULONG)bitplanedata1);
  printf("bitplanedata[1] $%lX\n", (ULONG)bitplanedata2);
  printf("xor_pixel $%lX\n", (ULONG)&xor_pixel);
  printf("sizeof(long_sinusdat) = %d\n", (int)sizeof(long_sinusdat));
  printf("seq_irq $%lX\n", (ULONG)&set_irq);
  for(int i = 0; i < BPLHEIGHT; ++i) {
    bplinfo.row_addresses[0][i] = bplinfo.bitplanedata[0] + i * BPLWIDTH/8;
    bplinfo.row_addresses[1][i] = bplinfo.bitplanedata[1] + i * BPLWIDTH/8;
  }
  own_machine(OWN_libraries|OWN_view|OWN_trap|OWN_interrupt);
  custom.dmacon = DMAF_SETCLR | DMAF_MASTER | DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER;
  set_irq(&irqhandler);
  set_bitplane_ptr(&bplinfo);
  setup_custom_chips(&very_simple_copperlist[0]);
  bplinfo.bitplanedata[0][40] = 0xff;
  for(short int x = 32; x < 256; ++x) {
    draw_vline(&bplinfo, x, x, 100);
  }
  draw_vline(&bplinfo, 0xffff, 160, 80);
  draw_vline(&bplinfo, 0xAAAA, 161, 80);
  draw_vline(&bplinfo, 0x5555, 162, 80);
  draw_vline(&bplinfo, 0xAAAA, 163, 80);
  draw_vline(&bplinfo, 0x5555, 164, 80);
  wait4mouse();
  disown_machine();
  return 0;
}
