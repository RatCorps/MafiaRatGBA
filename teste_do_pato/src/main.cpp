#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_point.h"
#include "bn_size.h"
#include "bn_sprite_ptr.h"
#include "bn_bg_tiles.h"
#include "bn_regular_bg_ptr.h"
#include "bn_regular_bg_item.h"
#include "bn_regular_bg_map_ptr.h"
#include "bn_sprite_animate_actions.h"

#include "things.h"
#include "bn_sprite_items_sheet.h"
#include "bn_regular_bg_tiles_items_overlay.h"
#include "bn_bg_palettes.h"
#include "bn_bg_palette_items_overlay_pal.h"

namespace
{
    // mapa do overlay: 32x32 celulas de 8x8 (minimo do background regular do GBA).
    constexpr int map_cols = 32;
    constexpr int map_rows = 32;
    alignas(int) bn::regular_bg_map_cell overlay_cells[map_cols * map_rows];
    alignas(int) bn::regular_bg_map_cell floor_cells[map_cols * map_rows]; // layer de chao (atras)

    // desloca o bg pra casa (0,0) do overlay coincidir com o grid2world (canto sup-esq).
    constexpr int map_center_x = map_cols * 8 / 2; // 128
    constexpr int map_center_y = map_rows * 8 / 2; // 128
    constexpr int bg_offset_x = map_center_x - screen_half_width;  // 8
    constexpr int bg_offset_y = map_center_y - screen_half_height; // 48
}

// State e grande -> vive em EWRAM (variavel global). init() zera o resto.
State state;

int main()
{
    bn::core::init();

    // ---- dois backgrounds dinamicos (mesmos tiles/paleta): chao atras, overlay na frente ----
    bn::regular_bg_map_item overlay_map_item(overlay_cells[0], bn::size(map_cols, map_rows));
    bn::regular_bg_map_item floor_map_item(floor_cells[0], bn::size(map_cols, map_rows));
    bn::bg_palettes::set_transparent_color(bn::color(0, 0, 0));

    bn::bg_tiles::set_allow_offset(false); // indices de tile absolutos no mapa dinamico
    bn::regular_bg_item overlay_item(
            bn::regular_bg_tiles_items::overlay,
            bn::bg_palette_items::overlay_pal,
            overlay_map_item);
    bn::regular_bg_item floor_item(
            bn::regular_bg_tiles_items::overlay,
            bn::bg_palette_items::overlay_pal,
            floor_map_item);
    bn::regular_bg_ptr overlay_bg = overlay_item.create_bg(0, 0);
    bn::regular_bg_ptr floor_bg = floor_item.create_bg(0, 0);
    bn::regular_bg_map_ptr overlay_bg_map = overlay_bg.map();
    bn::regular_bg_map_ptr floor_bg_map = floor_bg.map();
    bn::bg_tiles::set_allow_offset(true);

    overlay_bg.set_position(bg_offset_x, bg_offset_y);
    floor_bg.set_position(bg_offset_x, bg_offset_y);
    overlay_bg.set_priority(0); // frente (transparencia revela o chao)
    floor_bg.set_priority(1);   // atras

    // enche a layer de chao uma vez (nao muda depois)
    drawFloor(floor_cells, floor_map_item);
    floor_bg_map.reload_cells_ref();

    // ---- entidades ----
    init(&state);

    Thing& cursor = create(&state, Kind::cursor);
    cursor.pos = grid2world(bn::point (7,5));
    cursor.sprite_id = 17;

    Thing& rat = create(&state, Kind::unit);
    rat.pos =  grid2world(bn::point (5,5));
    rat.sprite_id = 2;
    rat.payload.unit.rank = Rank::caporegime;
    rat.payload.unit.team = Team::rat;

    // popula o grid de ocupacao uma vez: cada unidade marca a sua casa.
    for (Thing& t : state.kind_list[int(Kind::unit)])
    {
        state.grid[grid_index(world2grid(t.pos))] = &t;
    }

    // ---- sprites ----
    bn::sprite_ptr cursor_sprite =
            bn::sprite_items::sheet.create_sprite(cursor.pos.x(), cursor.pos.y(), cursor.sprite_id);
    bn::sprite_ptr rat_sprite =
            bn::sprite_items::sheet.create_sprite(rat.pos.x(), rat.pos.y(), rat.sprite_id);

    // sprite nasce com bg_priority 3 (ATRAS dos backgrounds). como os bgs estao em 0/1,
    // preciso trazer os sprites pra frente (bg_priority 0) senao o chao opaco os cobre.
    cursor_sprite.set_bg_priority(0);
    rat_sprite.set_bg_priority(0);

    rat_sprite.set_z_order(1); // z_order = ordem entre sprites (cursor por cima do rato)

    // animacao do rat pelo sistema do Butano (caporegime = bigrat: frames 2 e 3).
    auto rat_anim = bn::create_sprite_animate_action_forever(
            rat_sprite, 16, bn::sprite_items::sheet.tiles_item(), 2, 3);

    // desenha o overlay inicial (vazio) e sobe pra VRAM.
    drawMovementOverlay(overlay_cells, overlay_map_item, state.reachable_tiles);
    overlay_bg_map.reload_cells_ref();

    while(true)
    {
        // ---- input: cursor anda uma casa por toque (clampado ao tabuleiro) ----
        int dx = bn::keypad::right_pressed() - bn::keypad::left_pressed();
        int dy = bn::keypad::down_pressed() - bn::keypad::up_pressed();

        bn::point cg = world2grid(cursor.pos);
        int nx = cg.x() + dx;
        int ny = cg.y() + dy;
        if(nx < 0) nx = 0; else if(nx >= grid_width)  nx = grid_width - 1;
        if(ny < 0) ny = 0; else if(ny >= grid_height) ny = grid_height - 1;
        cursor.pos = grid2world(bn::point(nx, ny));

        bn::point gpos = world2grid(cursor.pos);

        // ---- se tem unidade selecionada, atualiza a seta ----
        if(state.selected_unit != nullptr)
            pathUpdate(&state.path, gpos, state.reachable_tiles);

        // ---- A: seleciona / move ----
        if(bn::keypad::a_pressed())
        {
            Thing* occ = state.grid[grid_index(gpos)];

            if(state.selected_unit == nullptr)
            {
                // nada selecionado: tenta selecionar a unidade sob o cursor
                if(occ != nullptr)
                {
                    if(occ->payload.unit.team == Team::rat)
                    {
                        state.selected_unit = occ;
                        calculateMovementRange(gpos, unit_ranges[int(occ->payload.unit.rank)],
                                               state.reachable_tiles);
                        state.path.count = 1;
                        state.path.tiles[0] = gpos;
                    }
                }
            }
            else
            {
                // ja selecionada: move se a casa alvo e alcancavel e esta vazia
                if(state.reachable_tiles[grid_index(gpos)] > 0 && state.grid[grid_index(gpos)] == 0)
                {
                    bn::point unit_g = world2grid(state.selected_unit->pos);
                    state.grid[grid_index(unit_g)] = nullptr;
                    state.grid[grid_index(gpos)] = state.selected_unit;
                    state.selected_unit->pos = grid2world(gpos);

                    state.selected_unit = nullptr;
                    for(int i = 0; i < grid_size; ++i) state.reachable_tiles[i] = 0;
                    state.path.count = 0;
                }
            }
        }

        // ---- render ----
        drawMovementOverlay(overlay_cells, overlay_map_item, state.reachable_tiles);
        renderPathArrow(overlay_cells, overlay_map_item, &state.path);
        overlay_bg_map.reload_cells_ref();

        cursor_sprite.set_position(cursor.pos);
        rat_sprite.set_position(rat.pos);
        rat_anim.update();

        bn::core::update();
    }
}
