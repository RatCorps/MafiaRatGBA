#include "defs.h"
#include "things.h"

void wait_vblank();
static inline u16 make_color(u8 r, u8 g, u8 b);

int main(void) {
  REG_DISPCNT = MODE0 | ENABLE_SPRITES | MAPPING_1D_MODE;
  State state;
  init(&state);

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
