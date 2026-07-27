package rat

NIL :: 0
MAX_THINGS :: 1024
MAX_ALARMS :: 4

Kind :: enum u8 {
	NILKIND,
	CURSORKIND,
	UNITKIND,
	PARTICLEKIND,
	PROJECTILEKIND,
	KIND_AMOUNT
}

Thing :: struct {
	alarms : [MAX_ALARMS]i16,
	id : u16,
	denseId : u16,
	subX : i16,
	subY : i16,
	personalField1 : u16,
	personalField2 : u16,
	personalField3 : u16,
	personalField4 : u16,
	nextSibId : u16,
	prevSibId : u16,
	personalField5 : u8,
	kind : Kind,
	spriteId : i8,
	health : i8,
}

State :: struct {
	things : [MAX_THINGS]Thing,
	activeIds : [MAX_THINGS]u16,
	kindHeads : [MAX_THINGS]u16,
	activeCount : u16,
	nextEmptySlot : u16
}

init :: proc(state : ^State) {
	state.activeCount = 0
	for &id in state.activeIds {
		id = 0
	}

	state.things[NIL].id = NIL
	state.things[NIL].kind = .NILKIND

	for i := 1; i <= MAX_THINGS; i+=1 {
		thing : ^Thing = &state.things[i]
		thing.id = u16(i)
		thing.kind = .NILKIND
		thing.nextSibId = u16(i + 1)

		for &alarm in thing.alarms {
			alarm = -1
		}
	}

	state.things[MAX_THINGS - 1].id = MAX_THINGS - 1
	state.things[MAX_THINGS - 1].kind = .NILKIND
	state.things[MAX_THINGS - 1].nextSibId = NIL

	for &kh in state.kindHeads {
		kh = NIL
	}
}

add :: proc(state : ^State, thing : Thing) -> u16 {
	if state.nextEmptySlot == NIL do return NIL

	slot : u16 = state.nextEmptySlot
	state.nextEmptySlot = state.things[slot].nextSibId

	state.things[slot].subX = thing.subX
	state.things[slot].subY = thing.subY
	state.things[slot].kind = thing.kind
	state.things[slot].spriteId = thing.spriteId
	state.things[slot].health = thing.health

	for i := 0; i <= MAX_ALARMS; i+=1 {
		state.things[slot].alarms[i] = thing.alarms[i]
	}

	state.things[slot].id = slot
	state.things[slot].personalField1 = NIL
	state.things[slot].personalField2 = NIL
	state.things[slot].nextSibId = NIL
	state.things[slot].prevSibId = NIL
	state.things[slot].denseId = state.activeCount
	state.activeIds[state.activeCount] = slot
	state.activeCount+=1
	kindLink(state, slot)

	return slot
}

get :: proc(state : ^State, id : u16) -> ^Thing {
	// lol
	assert(!(id <= NIL || id >= MAX_THINGS), "out of bounds")
	return &state.things[id]
}

rem :: proc(state : ^State, id : u16) {
	if id <= NIL || id >= MAX_THINGS || state.things[id].kind == .NILKIND do return

	deadDenseId : u16 = state.things[id].denseId
	if deadDenseId < state.activeCount - 1 {
	    lastEntityId : u16  = state.activeIds[state.activeCount - 1]
	    state.activeIds[deadDenseId] = lastEntityId
	    state.things[lastEntityId].denseId = deadDenseId
	}

	state.activeCount-=1
	state.things[id].denseId = NIL
	kindUnlink(state, id)

	for &alarm in state.things[id].alarms {
		alarm = -1
	}
	state.things[id].kind = .NILKIND
	state.things[id].nextSibId = state.nextEmptySlot
	state.nextEmptySlot = id
}

kindLink :: proc(state : ^State, id : u16) {
	if state.things[id].kind == .NILKIND do return

	k : Kind = state.things[id].kind

	head : u16 = state.kindHeads[k]

	if (head == NIL) {
	    state.kindHeads[k] = id
	    state.things[id].nextSibId = id
	    state.things[id].prevSibId = id
	} else {
	    tail : u16 = state.things[head].prevSibId
	    state.things[tail].nextSibId = id
	    state.things[id].prevSibId = tail

	    state.things[id].nextSibId = head
	    state.things[head].prevSibId = id
	}
}

kindUnlink :: proc(state : ^State, id : u16) {
	if state.things[id].kind == .NILKIND do return

	k : Kind = state.things[id].kind

	next : u16 = state.things[id].nextSibId
	prev : u16 = state.things[id].prevSibId

	if (next == id) {
	    state.kindHeads[k] = NIL
	} else {
	    state.things[prev].nextSibId = next
	    state.things[next].prevSibId = prev

	    if (state.kindHeads[k] == id) {
	        state.kindHeads[k] = next
	    }
	}
}
