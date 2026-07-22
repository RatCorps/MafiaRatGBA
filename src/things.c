#include "things.h"

const i8 SINTABLE[256] = {
    0,    3,    6,    9,   13,   16,   19,   22,   25,   28,   31,   34,   37,   40,   43,   46,
    49,   52,   55,   58,   60,   63,   66,   68,   71,   74,   76,   79,   81,   84,   86,   88,
    91,   93,   95,   97,   99,  101,  103,  105,  106,  108,  110,  111,  113,  114,  116,  117,
    118,  119,  121,  122,  122,  123,  124,  125,  126,  126,  127,  127,  127,  127,  127,  127,
    127,  127,  127,  127,  127,  127,  127,  126,  126,  125,  124,  123,  122,  122,  121,  119,
    118,  117,  116,  114,  113,  111,  110,  108,  106,  105,  103,  101,   99,   97,   95,   93,
    91,   88,   86,   84,   81,   79,   76,   74,   71,   68,   66,   63,   60,   58,   55,   52,
    49,   46,   43,   40,   37,   34,   31,   28,   25,   22,   19,   16,   13,    9,    6,    3,
    0,   -3,   -6,   -9,  -13,  -16,  -19,  -22,  -25,  -28,  -31,  -34,  -37,  -40,  -43,  -46,
    -49,  -52,  -55,  -58,  -60,  -63,  -66,  -68,  -71,  -74,  -76,  -79,  -81,  -84,  -86,  -88,
    -91,  -93,  -95,  -97,  -99, -101, -103, -105, -106, -108, -110, -111, -113, -114, -116, -117,
    -118, -119, -121, -122, -122, -123, -124, -125, -126, -126, -127, -127, -127, -128, -128, -128,
    -128, -128, -128, -128, -127, -127, -127, -126, -126, -125, -124, -123, -122, -122, -121, -119,
    -118, -117, -116, -114, -113, -111, -110, -108, -106, -105, -103, -101,  -99,  -97,  -95,  -93,
    -91,  -88,  -86,  -84,  -81,  -79,  -76,  -74,  -71,  -68,  -66,  -63,  -60,  -58,  -55,  -52,
    -49,  -46,  -43,  -40,  -37,  -34,  -31,  -28,  -25,  -22,  -19,  -16,  -13,   -9,   -6,   -3,
};

const i8 COSTABLE[256]= {
    127,  127,  127,  127,  127,  127,  127,  126,  126,  125,  124,  123,  122,  122,  121,  119,
    118,  117,  116,  114,  113,  111,  110,  108,  106,  105,  103,  101,   99,   97,   95,   93,
    91,   88,   86,   84,   81,   79,   76,   74,   71,   68,   66,   63,   60,   58,   55,   52,
    49,   46,   43,   40,   37,   34,   31,   28,   25,   22,   19,   16,   13,    9,    6,    3,
    0,   -3,   -6,   -9,  -13,  -16,  -19,  -22,  -25,  -28,  -31,  -34,  -37,  -40,  -43,  -46,
    -49,  -52,  -55,  -58,  -60,  -63,  -66,  -68,  -71,  -74,  -76,  -79,  -81,  -84,  -86,  -88,
    -91,  -93,  -95,  -97,  -99, -101, -103, -105, -106, -108, -110, -111, -113, -114, -116, -117,
    -118, -119, -121, -122, -122, -123, -124, -125, -126, -126, -127, -127, -127, -128, -128, -128,
    -128, -128, -128, -128, -127, -127, -127, -126, -126, -125, -124, -123, -122, -122, -121, -119,
    -118, -117, -116, -114, -113, -111, -110, -108, -106, -105, -103, -101,  -99,  -97,  -95,  -93,
    -91,  -88,  -86,  -84,  -81,  -79,  -76,  -74,  -71,  -68,  -66,  -63,  -60,  -58,  -55,  -52,
    -49,  -46,  -43,  -40,  -37,  -34,  -31,  -28,  -25,  -22,  -19,  -16,  -13,   -9,   -6,   -3,
    0,    3,    6,    9,   13,   16,   19,   22,   25,   28,   31,   34,   37,   40,   43,   46,
    49,   52,   55,   58,   60,   63,   66,   68,   71,   74,   76,   79,   81,   84,   86,   88,
    91,   93,   95,   97,   99,  101,  103,  105,  106,  108,  110,  111,  113,  114,  116,  117,
    118,  119,  121,  122,  122,  123,  124,  125,  126,  126,  127,  127,  127,  127,  127,  127,
};

const Vec2_i16 DIRECTIONS[4] = {
	{ 0, -1}, // NORTH
    { 0,  1}, // SORTH
    {-1,  0}, // WORTH
    { 1,  0}  // EORTH
};

void init(State *state) {
    state->activeCount = 0;
	state->selectedUnit = NULL;
    memset(state->activeIds, NIL, MAX_THINGS * sizeof(u16));

    state->things[NIL].id = NIL;
    state->things[NIL].kind = NILKIND;

    for (int i = 1; i < MAX_THINGS - 1; ++i) {
        state->things[i].id = i;
        state->things[i].kind = NILKIND;
        state->things[i].nextSibId = i + 1;

        memset(state->things[i].alarms, -1, sizeof(i16) * MAX_ALARMS);
    }

    state->things[MAX_THINGS - 1].id = MAX_THINGS - 1;
    state->things[MAX_THINGS - 1].kind = NILKIND;
    state->things[MAX_THINGS - 1].nextSibId = NIL;

    state->nextEmptySlot = 1;

    memset(&state->kindHeads, NIL, sizeof(state->kindHeads));
}

u16 add(State* state, Thing thing) {
    if (state->nextEmptySlot == NIL) return NIL;

    u16 slot = state->nextEmptySlot;
    state->nextEmptySlot = state->things[slot].nextSibId;

    state->things[slot] = thing;

    memcpy(state->things[slot].alarms, thing.alarms, sizeof(thing.alarms));

    state->things[slot].id = slot;

    state->things[slot].nextSibId = NIL;
    state->things[slot].prevSibId = NIL;

    state->things[slot].denseId = state->activeCount;
    state->activeIds[state->activeCount] = slot;
    state->activeCount++;

    kindLink(state, slot);

    return slot;
}

Thing* get(State* state, u16 id) {
	return (id > 0 && id < MAX_THINGS) ? &state->things[id] : &state->things[NIL];
}

void rem(State *state, u16 id) {
    if (id <= NIL || id >= MAX_THINGS || state->things[id].kind == NILKIND) {
        return;
    }

    u16 deadDenseId = state->things[id].denseId;
    if (deadDenseId < state->activeCount - 1) {
        u16 lastEntityId = state->activeIds[state->activeCount - 1];
        state->activeIds[deadDenseId] = lastEntityId;
        state->things[lastEntityId].denseId = deadDenseId;
    }

    state->activeCount--;
    state->things[id].denseId = NIL;
    kindUnlink(state, id);

    memset(state->things[id].alarms, -1, sizeof(state->things[id].alarms));
    state->things[id].kind = NILKIND;
    state->things[id].nextSibId = state->nextEmptySlot;
    state->nextEmptySlot = id;
}

void kindLink(State *state, u16 id) {
    if (state->things[id].kind == NILKIND) return;

    Kind k = state->things[id].kind;

    u16 head = state->kindHeads[k];

    if (head == NIL) {
        state->kindHeads[k] = id;
        state->things[id].nextSibId = id;
        state->things[id].prevSibId = id;
    } else {
        u16 tail = state->things[head].prevSibId;
        state->things[tail].nextSibId = id;
        state->things[id].prevSibId = tail;

        state->things[id].nextSibId = head;
        state->things[head].prevSibId = id;
    }
}

void kindUnlink(State *state, u16 id) {
    if (state->things[id].kind == NILKIND) return;

    Kind k = state->things[id].kind;

    u16 next = state->things[id].nextSibId;
    u16 prev = state->things[id].prevSibId;

    if (next == id) {
        state->kindHeads[k] = NIL;
    } else {
        state->things[prev].nextSibId = next;
        state->things[next].prevSibId = prev;

        if (state->kindHeads[k] == id) {
            state->kindHeads[k] = next;
        }
    }
}

// assumes the values being passed are fixed point.
Vec2_i16 world2grid(Vec2_i16 worldPos) {
    return (Vec2_i16){
        .x = FIXED_16_TO_INT(worldPos.x) / TILE_SIZE,
        .y = FIXED_16_TO_INT(worldPos.y) / TILE_SIZE
    };
}

// returns fixed point, top left origin position.
Vec2_i16 grid2world(Vec2_i16 gridPos) {
	return (Vec2_i16){
        .x = INT_TO_FIXED_16(gridPos.x * TILE_SIZE),
        .y = INT_TO_FIXED_16(gridPos.y * TILE_SIZE)
    };
}

u32 GRID_INDEX(Vec2_i16 gridpos) {
	return gridpos.y * GRID_WIDTH + gridpos.x;
}

/* https://www.geeksforgeeks.org/dsa/breadth-first-search-or-bfs-for-a-graph/ */
void calculateMovementRange(Vec2_i16 startGridPos, u32 maxRange, u8 reachableTiles[GRID_SIZE]) {
	// 0 = unvisited
	memset(reachableTiles, 0, GRID_SIZE);
	static Vec2_i16 queue[GRID_SIZE];
	u32 head = 0;
	u32 tail = 0;

    reachableTiles[GRID_INDEX(startGridPos)] = maxRange + 1; // offset the zero
    queue[tail++] = startGridPos;

    while (head < tail) {
        Vec2_i16 current = queue[head++];
        int remainingMoves = reachableTiles[GRID_INDEX(current)];

        if (remainingMoves <= 0) continue;

        u8 nextMove = remainingMoves - 1;

        // check cardinal neighbors
        for (int i = 0; i < 4; i++) {
            Vec2_i16 nextGridPos = {current.x + DIRECTIONS[i].x, current.y + DIRECTIONS[i].y};

            // check bounds
            if (nextGridPos.x >= 0 && nextGridPos.x < GRID_WIDTH && nextGridPos.y >= 0 && nextGridPos.y < GRID_HEIGHT) {
                if (reachableTiles[GRID_INDEX(nextGridPos)] < nextMove) {
                    reachableTiles[GRID_INDEX(nextGridPos)] = nextMove;
                    queue[tail++] = nextGridPos;
                }
            }
        }
    }
}

void drawMovementOverlay(u8 reachableTiles[GRID_SIZE]) {
    SCR_ENTRY *map = se_mem[OVERLAY_SBB];

    for (u32 y = 0; y < GRID_HEIGHT; y++) {
        for (u32 x = 0; x < GRID_WIDTH; x++) {
            u16 tile = 0;

            if (reachableTiles[GRID_INDEX((Vec2_i16){x, y})] > 0) tile = 1;

            int mapX = x * 2;
            int mapY = y * 2;

            map[(mapY + 0) * 32 + (mapX + 0)] = tile;
            map[(mapY + 0) * 32 + (mapX + 1)] = tile;
            map[(mapY + 1) * 32 + (mapX + 0)] = tile;
            map[(mapY + 1) * 32 + (mapX + 1)] = tile;
        }
    }
}

u32 isSelectedPositionReachable(u8 reachableTiles[GRID_SIZE], Vec2_i16 gridPos) {
	if (reachableTiles[GRID_INDEX(gridPos)] > 0) return 1;
	return 0;
}