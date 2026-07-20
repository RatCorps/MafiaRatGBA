#include <stdint.h>

typedef uint16_t u16;
typedef int16_t i16;
typedef uint8_t u8;
typedef int8_t i8;

#define REG_DISPCNT *(volatile u16 *)0x04000000
#define VRAM ((volatile u16 *)0x06000000)
#define EWRAM ((volatile u16 *)0x02000000)
#define IWRAM ((volatile u16 *)0x03000000)
#define REG_VCOUNT *(volatile u16 *)0x04000006

#define MODE3 0x0003
#define BG2_ENABLE 0x0400

void wait_vblank();
static inline u16 make_color(u8 r, u8 g, u8 b);

int main(void) {
  REG_DISPCNT = MODE3 | BG2_ENABLE;

  while (1) {
    wait_vblank();
  }
  return 0;
}

void wait_vblank() {
  while (REG_VCOUNT >= 160)
    ;
  while (REG_VCOUNT < 160)
    ;
}

static inline u16 make_color(u8 r, u8 g, u8 b) {
  return (r & 0x1F) | ((g & 0x1F) << 5) | ((b & 0x1F) << 10);
}
