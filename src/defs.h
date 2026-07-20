#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>

typedef int32_t  i32;
typedef uint32_t u32;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int8_t   i8;
typedef uint8_t  u8;

#define REG_DISPCNT *(volatile u16 *)0x04000000
#define VRAM ((volatile u16 *)0x06000000)
#define EWRAM ((volatile u16 *)0x02000000)
#define IWRAM ((volatile u16 *)0x03000000)
#define REG_VCOUNT *(volatile u16 *)0x04000006

#define MODE0 0x0000
#define ENABLE_SPRITES 0x1000
#define MAPPING_1D_MODE 0x0040

#endif