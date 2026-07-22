#include "things.h"
#include "tonc_input.h"
#include "tonc_nocash.h"
#include "tonc_oam.h"
#include "tonc_video.h"
#include "spritesheet.h"
#include <tonc_memdef.h>

EWRAM_DATA State state;

#define GRID_HEIGHT 16;
#define GRID_WIDTH 16;
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
        .ownedByPlayer = 1,
    });

    u16 rat = add(&state, (Thing){
        .kind = UNITKIND,
        .subX = INT_TO_FIXED_16(SCREEN_WIDTH/2),
        .subY = INT_TO_FIXED_16(SCREEN_HEIGHT / 2),
        .spriteId = 1,
        .ownedByPlayer = 1,
    });
    int row;
    int collumn; 
    while(1) {
        VBlankIntrWait();
       
        // read controls.
        key_poll();
        int dx = key_tri_horz();
        int dy = key_tri_vert();

        Thing *c = &state.things[cursor];
        Thing *r = &state.things[rat];
     if  (KEY_DOWN_NOW(KEY_A)){
        
         for(row = 1; row < 15; row++){
             for (collumn = 1; collumn < 10; collumn++){
                 int min_x = row * 16 - 16;
                 int max_x = row*16;
                 int min_y = collumn *16 - 16;
                 int max_y = collumn * 16;


                 if (c->subX >= min_x && c->subX < max_x && c->subY >= min_y && c->subY < max_y){
                     nocash_puts("teste");
                     return 0;
                 }
             }
         }
      }        
 if (c->ownedByPlayer == 1){

        // update game logic.
        c->subX += dx * CURSOR_SPEED;
        
        c->subY += dy * CURSOR_SPEED;
}
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
