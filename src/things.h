#ifndef THINGS_H
#define THINGS_H

#include "defs.h"

#define NIL 0
#define MAX_THINGS 256
#define MAX_ALARMS 4

/*
	the gba has no hardware Floating Point Unit, so any float usages at runtime will engage
	heavy CPU (50-100+) cycle instructions that will do the work of an FPU all in the software. so in this project we will
	use fixed point integers to represent fractional units. these are the macros we will use to handle the conversions.
	note that the macros labeled COMPTIME are only to be used at comptime. these float usages will get managed by GCC.
	as i said, any float usages at runtime will engage CPU heavy instructions that will make our game really sluggish.
	please try to understand the fixed point integers when developing for the game: en.wikipedia.org/wiki/Fixed-point_arithmetic
*/

// compile time
#define SCALE_4BIT 16.0f

// 16 bit; 12.4 fixed point
#define COMPTIME_FLOAT_TO_FIXED_16(f) ((i16)((f) * SCALE_4BIT))
#define COMPTIME_FIXED_16_TO_FLOAT(fx) ((float)(fx) / SCALE_4BIT)

// 8 bit; 4.4 fixed point
#define COMPTIME_FLOAT_TO_FIXED_8(f) ((i8)((f) * SCALE_4BIT))
#define COMPTIME_FIXED_8_TO_FLOAT(fx) ((float)(fx) / SCALE_4BIT)

// runtime
#define FIX_SHIFT_4 4

// integer <-> fixed point
#define INT_TO_FIXED_16(i) ((i16)((i) << FIX_SHIFT_4))
#define FIXED_16_TO_INT(fx) ((i16)((fx) >> FIX_SHIFT_4))
#define INT_TO_FIXED_8(i) ((i8)((i) << FIX_SHIFT_4))
#define FIXED_8_TO_INT(fx) ((i8)((fx) >> FIX_SHIFT_4))

typedef enum {
	NILKIND,
	UNITKIND,
	PARTICLEKIND,
	PROJECTILEKIND,
	KIND_AMOUNT
} Kind;

typedef struct {
	i16 alarms[MAX_ALARMS];
	u16 id;
	u16 denseId;
	i16 subX;
	i16 subY;
	u16 personalField1;
	u16 personalField2;
	u16 nextSibId;
	u16 prevSibId;
	i8 maskWidth;
	i8 maskHeight;
	i8 scaleX;
	i8 scaleY;
	u8 rotation;
	u8 kind;
	i8 spriteId;
	i8 health;
} __attribute__((aligned(4))) Thing; /* ordered by natural alignment, should be fine. but forcing 4 byte packing for any DMA/cacheline related oddities. */

// this will go onto EWRAM
typedef struct {
	Thing things[MAX_THINGS];
	u16 activeIds[MAX_THINGS];
	u16 kindHeads[KIND_AMOUNT];
	u16 activeCount;
	u16 nextEmptySlot;
} __attribute__((aligned(4))) State;

extern const i8 SINTABLE[256];
extern const i8 COSTABLE[256];

void init(State *state);
u16 add(State* state, Thing thing);
void rem(State* state, u16 id);
void kindLink(State* state, u16 id);
void kindUnlink(State* state, u16 id);

// mem helpers
void memcpy16(u16 *dst, const u16 *src, u32 count);
void memset16(u16 *dst, u16 value, u32 count);
void memcpy32(u32 *dst, const u32 *src, u32 count);
void memset32(u32 *dst, u32 value, u32 count);

#endif