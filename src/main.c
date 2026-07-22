#include "things.h"
#include <tonc_input.h>
#include <tonc_nocash.h>
#include <tonc_oam.h>
#include "spritesheet.h"
#include "tonc_core.h"
#include "tonc_types.h"
#include <tonc_memdef.h>
#include "overlay.h"

EWRAM_DATA State state = {0};

int main(void) {
    // enable isr switchboard and VBlank interrupt
    irq_init(NULL);
    irq_enable(II_VBLANK);

    REG_DISPCNT = DCNT_MODE0 | DCNT_BG1 | DCNT_OBJ | DCNT_OBJ_1D;

	REG_BG1CNT = BG_PRIO(1) | BG_CBB(0) | BG_SBB(OVERLAY_SBB) | BG_REG_32x32;

	memcpy16(pal_bg_mem, overlayPal, overlayPalLen / 2);
    memcpy32(&tile_mem[0][1], overlayTiles, overlayTilesLen / 4);

    memcpy16(pal_obj_mem, spritesheetPal, spritesheetPalLen / 2);
    memcpy32(&tile_mem[4][0], spritesheetTiles, spritesheetTilesLen / 4);

    oam_init(oam_mem, 128);

    init(&state);

	u16 cursor = add(&state, (Thing){
        .kind = CURSORKIND,
        .subX = INT_TO_FIXED_16(112),
        .subY = INT_TO_FIXED_16(80),
        .spriteId = 23,
    });

    /*u16 rat =*/add(&state, (Thing){
        .kind = UNITKIND,
        .subX = INT_TO_FIXED_16(112),
        .subY = INT_TO_FIXED_16(80),
        .spriteId = 1,
        .payload.unit.team = TEAM_RAT,
    });
	
	// after creation
	// populate the grid ONCE.
	for (size_t i = 0; i < state.activeCount; ++i) {
		u16 id = state.activeIds[i];
		Thing *t = &state.things[id];
		if (t->kind != UNITKIND) continue;
		state.grid[GRID_INDEX(world2grid((Vec2_i16){t->subX, t->subY}))] = t->id;
	}

	// cursor movement delay
	key_repeat_limits(30, 4);
	key_repeat_mask(KEY_DIR);

    while(1) {
        VBlankIntrWait();
       
        // read controls.
        key_poll();

		u32 repeatedKeys = key_repeat(KEY_DIR);
		int dx = 0, dy = 0;
		if (repeatedKeys & KEY_RIGHT) dx = 1;
		else if (repeatedKeys & KEY_LEFT) dx = -1;
		if (repeatedKeys & KEY_DOWN) dy = 1;
		else if (repeatedKeys & KEY_UP) dy = -1;

        Thing *c = &state.things[cursor];
        //Thing *r = &state.things[rat];

        // update game logic.
		c->subX += dx * CURSOR_SPEED;
        c->subY += dy * CURSOR_SPEED;

		if (key_hit(KEY_A)) {
			Vec2_i16 gridPos = world2grid((Vec2_i16){c->subX, c->subY});

			u32 id = state.grid[GRID_INDEX(gridPos)];
			if (state.selectedUnit == NULL) {
				if (id != NIL) {
					// we've hit an occupied tile.
					// we need to select the unit at this tile.
					Thing* selectedUnit = get(&state, id);
					if (selectedUnit->payload.unit.team == TEAM_RAT) {
						// it's a player controlled unit.
						state.selectedUnit = selectedUnit;
						// we need to determine the possible positions we can move at.
						calculateMovementRange(gridPos, RANGE_INFANTRY, state.reachableTiles);
					}
				}	
			} else { // we already have a selected unit.
				if (isSelectedPositionReachable(state.reachableTiles, gridPos))	{
					// update grid
					Vec2_i16 unitGridPos = world2grid((Vec2_i16){state.selectedUnit->subX, state.selectedUnit->subY});
					state.grid[GRID_INDEX(unitGridPos)] = NIL;
					state.grid[GRID_INDEX(gridPos)] = state.selectedUnit->id;

					//move
					Vec2_i16 targetPos = grid2world(gridPos);
					state.selectedUnit->subX = targetPos.x;
					state.selectedUnit->subY = targetPos.y;

					state.selectedUnit = NULL;
					//clearReachableTiles
					memset(&state.reachableTiles, 0, GRID_SIZE);
				}
			}
		}

		// render reachable tiles
		drawMovementOverlay(state.reachableTiles);

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
