#include "things.h"
#include <tonc_input.h>
#include <tonc_nocash.h>
#include <tonc_oam.h>
#include "spritesheet.h"
#include "tonc_core.h"
#include "tonc_types.h"
#include <tonc_memdef.h>

EWRAM_DATA State state = {0};

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
        .subX = INT_TO_FIXED_16(SCREEN_HALF_WIDTH),
        .subY = INT_TO_FIXED_16(SCREEN_HALF_HEIGHT),
        .spriteId = 23,
    });

    /*u16 rat =*/add(&state, (Thing){
        .kind = UNITKIND,
        .subX = INT_TO_FIXED_16(SCREEN_HALF_WIDTH),
        .subY = INT_TO_FIXED_16(SCREEN_HALF_HEIGHT),
        .spriteId = 1,
        .payload.unit.team = TEAM_RAT,
    });
	

	// after creation
	// populate the grid ONCE.
	for (size_t i = 0; i < state.activeCount; ++i) {
		if (state.things[i].kind != UNITKIND) continue;
		state.grid[GRID_INDEX(world2grid((Vec2_i16){state.things[i].subX, state.things[i].subY}))] = state.things[i].id;
	}

    while(1) {
        VBlankIntrWait();
       
        // read controls.
        key_poll();
        int dx = key_tri_horz();
        int dy = key_tri_vert();

        Thing *c = &state.things[cursor];
        //Thing *r = &state.things[rat];

        // update game logic.
		c->subX += dx * CURSOR_SPEED;
        c->subY += dy * CURSOR_SPEED;

		if (key_hit(KEY_A)) {
			Vec2_i16 gridpos = world2grid((Vec2_i16){c->subX, c->subY});
			u32 id = state.grid[GRID_INDEX(gridpos)];
			if (id != NIL) {
				// we've hit an occupied tile.
				// we need to select the unit at this tile.
				state.selectedUnit = get(&state, id);
				if (state.selectedUnit->payload.unit.team == TEAM_RAT) {
					// it's a player controlled unit.
					// we need to determine the possible positions we can move at.
					calculateMovementRange(gridpos, RANGE_INFANTRY, state.reachableTiles);
				}
			}
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
