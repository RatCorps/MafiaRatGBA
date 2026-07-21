#include "things.h"
#include "tonc_input.h"
#include "tonc_oam.h"
#include "tonc_video.h"
#include "spritesheet.h"
#include <tonc_memdef.h>

EWRAM_DATA State state;

int main(void) {
    // enable isr switchboard and VBlank interrupt
    irq_init(NULL);
    irq_enable(II_VBLANK);

    REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ | DCNT_OBJ_1D;

    memcpy16(pal_obj_mem, spritesheetPal, spritesheetPalLen / 2);
    memcpy32(&tile_mem[4][0], spritesheetTiles, spritesheetTilesLen / 4);

    oam_init(oam_mem, 128);

    init(&state);

    u16 cursor = add(&state, (Thing){
        .kind = CURSORKIND,
        .subX = INT_TO_FIXED_16(SCREEN_WIDTH / 2),
        .subY = INT_TO_FIXED_16(SCREEN_HEIGHT / 2),
        .spriteId = 23,
    });

    while (1) {
        VBlankIntrWait();

        // read controls.
        key_poll();
        int dx = key_tri_horz();
        int dy = key_tri_vert();

        // update game logic.
        Thing *c = &state.things[cursor];
        c->subX += dx * CURSOR_SPEED;
        c->subY += dy * CURSOR_SPEED;

        // render entities.
        for (u16 i = 0; i < state.activeCount; i++) {
            u16 id = state.activeIds[i];
            Thing *t = &state.things[id];

            // get screen position
            i16 screenX = FIXED_16_TO_INT(t->subX);
            i16 screenY = FIXED_16_TO_INT(t->subY);

            // sprite position and appearance.
            OBJ_ATTR *obj = &oam_mem[i];
            obj_set_attr(obj, ATTR0_SQUARE, ATTR1_SIZE_16, ATTR2_PALBANK(0) | SPRITE_ID(t));
            obj_set_pos(obj, screenX, screenY);
        }

        oam_copy(obj_mem, oam_mem, state.activeCount);
    }
    return 0;
}
