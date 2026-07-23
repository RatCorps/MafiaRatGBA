#ifndef THINGS_H
#define THINGS_H

#include "tonc_video.h"
#include <stdint.h>
#include <tonc.h>
#include <string.h>

typedef int32_t  i32;
typedef uint32_t u32;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int8_t   i8;
typedef uint8_t  u8;

#define OVERLAY_SBB 28

#define NIL 0
#define MAX_THINGS 256
#define MAX_ALARMS 4
#define TILE_SIZE 16
#define SCREEN_HALF_WIDTH 120
#define SCREEN_HALF_HEIGHT 80
#define GRID_WIDTH (SCREEN_WIDTH / TILE_SIZE)
#define GRID_HEIGHT (SCREEN_HEIGHT / TILE_SIZE)
#define GRID_SIZE (GRID_WIDTH * GRID_HEIGHT)

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

#define SPRITE_ID(t) ((t->spriteId) * 4)

#define CURSOR_SPEED INT_TO_FIXED_16(TILE_SIZE)

typedef enum {
	NILKIND,
	CURSORKIND,
	UNITKIND,
	PARTICLEKIND,
	PROJECTILEKIND,
	KIND_AMOUNT
} Kind;

// this is a semantic convention. this could be easily be a bool but i prefer it to be an enum because it is way more readable.
typedef enum {
	TEAM_RAT,
	TEAM_CAT
} Team;

typedef enum {
    RANK_INFANTRY,
    RANK_MADE_MAN,
    RANK_MECCANIZATA,
    RANK_CECCHINO,
    RANK_UNDERBOSS,
    RANK_CAPOREGIME
} RANK;

// this is balancing.
typedef enum {
	RANGE_INFANTRY = 3,
	RANGE_MADE_MAN = 2,
    RANGE_MECCANIZATA = 2,
    RANGE_UNDERBOSS = 4,
    RANGE_CAPOREGIME = 3,
} UnitRange;

typedef struct {
	i16 x, y;
} Vec2_i16;

/*
ideally this struct should stay at 32 bytes. the IWRAM has a 32-bit bus and the EWRAM a 16-bit one.
when this struct stays at 32 bytes maximum, it takes 8 cpu cycles to fetch it when in IWRAM and 16 if it is in EWRAM.
anymore and we're using more cycles than what is sensible.
any less and we're underutilizing block space.
*/
typedef struct {
	i16 alarms[MAX_ALARMS];
	u16 id;
	u16 denseId;
	i16 subX;
	i16 subY;
	// personal fields are leftovers of the Things engine which are not relevant here, and i
	// didn't remove them to keep the alignment.
	// the personal fields can and should be abstracted onto a union for the specific usage
	// of the different Kinds.
	union {
		struct {
			u16 personalField1; // unused for now, can be replaced
			u16 personalField2; // unused for now, can be replaced
			u16 personalField3;	 // unused for now, can be replaced
			u8 rank; // this unit's rank in the mafia.
			u8 team;
		} unit;
	} payload;
	u16 nextSibId;
	u16 prevSibId;
	u8 personalField5; // unused for now, can be replaced
	u8 kind;
	i8 spriteId;
	i8 health;
/* ordered by natural alignment, should be fine. but forcing 4 byte packing for any DMA/cacheline related oddities. */
} __attribute__((aligned(4))) Thing;

// this will go onto EWRAM
typedef struct {
	Thing things[MAX_THINGS];
	Thing* selectedUnit;
	u16 grid[GRID_SIZE];
	u16 activeIds[MAX_THINGS];
	u16 kindHeads[KIND_AMOUNT];
	u16 activeCount;
	u16 nextEmptySlot;
	u8 reachableTiles[GRID_SIZE];
} __attribute__((aligned(4))) State;

#define MAX_FRAMES 2

typedef enum {
    ANIM_RAT,
    ANIM_BIGRAT
} AnimName;

typedef struct {
    i8 frames[MAX_FRAMES];
    i8 ticksPerFrame; // fixed point 4.4
    i8 loops;
} __attribute__((aligned(4))) Animation;

extern const i8 SINTABLE[256];
extern const i8 COSTABLE[256];
extern const Vec2_i16 DIRECTIONS[4];
extern const Animation ANIMATIONS[];
extern const UnitRange UNIT_RANGES[];

void init(State *state);
u16 add(State* state, Thing thing);
Thing* get(State* state, u16 id);
void rem(State* state, u16 id);
void animate(Thing* thing, const Animation* anim);
void kindLink(State* state, u16 id);
void kindUnlink(State* state, u16 id);
Vec2_i16 world2grid(Vec2_i16 worldPos);
Vec2_i16 grid2world(Vec2_i16 gridPos);
u32 GRID_INDEX(Vec2_i16 gridpos);
void calculateMovementRange(Vec2_i16 startGridPos, u32 maxRange, u8 reachableTiles[GRID_SIZE]);
void drawMovementOverlay(u8 reachableTiles[GRID_SIZE]);
u32 isSelectedPositionReachable(u8 reachableTiles[GRID_SIZE], Vec2_i16 gridPos);

#endif
