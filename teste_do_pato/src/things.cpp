#include "things.h"
#include <cstdint>

#include "bn_regular_bg_map_cell_info.h"

namespace
{
    // os 4 vizinhos cardeais, na ordem N, S, O, L. usado pelo BFS e pelo pathfinding.
    constexpr bn::point directions[4] = {
        { 0, -1}, // norte
        { 0,  1}, // sul
        {-1,  0}, // oeste
        { 1,  0}  // leste
    };
}

// alcance de movimento por Rank (ordem do enum: infantry, made_man, meccanizata,
// cecchino, underboss, caporegime). cecchino sem valor ainda -> 0.
const int unit_ranges[6] = { 3, 2, 2, 0, 4, 3 };

void init(State* state)
{

    state->selected_unit = nullptr;
    state->path.count = 0;
    for (int i = 0; i < grid_size; i++)
    {
        state->reachable_tiles[i] = 0;
        state->grid[i] = nullptr;
    }

}

Thing& create(State* state, Kind kind)
{
    Thing& t = state->pool.create(); // slot novo default nulo
    t.kind = kind;
    state->kind_list[int(kind)].push_back(t); //entra na lista
    return t; // preenhe pos , sprite id etc
}

void destroy(State* state, Thing& thing)
{
    state->kind_list[int(thing.kind)].erase(thing);
    state->pool.destroy(thing);
}

bn::point world2grid(bn::fixed_point world_pos)
{
    // world_pos e coordenada do Butano (centro = 0). Somo a meia-tela pra voltar
    // ao espaco 0..240 / 0..160, ai divido pelo tile pra achar a casa.
    return bn::point(
        (world_pos.x().integer() + screen_half_width)  / tile_size,
        (world_pos.y().integer() + screen_half_height) / tile_size);
}

bn::fixed_point grid2world(bn::point grid_pos)
{
    // devolve o CENTRO da casa em coordenadas do Butano (pronto pro set_position).
    return bn::fixed_point(
        grid_pos.x() * tile_size + half_tile - screen_half_width,
        grid_pos.y() * tile_size + half_tile - screen_half_height);
}

int grid_index(bn::point grid_pos)
{
    // achata (x,y) num indice unico do array 1D. mesma conta do GRID_INDEX antigo.
    return grid_pos.y() * grid_width + grid_pos.x();
}

void calculateMovementRange(bn::point start, int max_range, uint8_t reachable_tiles[grid_size])
{
    // 0 = casa nao visitada. limpa tudo antes de comecar.
    for(int i = 0; i < grid_size; ++i)
        reachable_tiles[i] = 0;

    // fila da BFS. static: fica fora da pilha (150 pontos = espaco relevante no GBA).
    static bn::point queue[grid_size];
    int head = 0;
    int tail = 0;

    // truque do "+1": guardamos (passos restantes + 1) para que 0 continue significando
    // "nao visitado". a casa inicial recebe o budget cheio.
    reachable_tiles[grid_index(start)] = max_range + 1;
    queue[tail++] = start;

    while(head < tail)
    {
        bn::point current = queue[head++];
        int remaining = reachable_tiles[grid_index(current)];

        if(remaining <= 0)
            continue;

        uint8_t next_move = remaining - 1; // o que sobra ao dar mais um passo

        for(int i = 0; i < 4; ++i)
        {
            bn::point next = current + directions[i];

            // dentro dos limites do tabuleiro?
            if(next.x() >= 0 && next.x() < grid_width &&
               next.y() >= 0 && next.y() < grid_height)
            {
                // so melhora se este caminho chega com MAIS movimento sobrando
                if(reachable_tiles[grid_index(next)] < next_move)
                {
                    reachable_tiles[grid_index(next)] = next_move;
                    queue[tail++] = next;
                }
            }
        }
    }
}

bool sameTile(bn::point a, bn::point b)
{
    return a == b; // bn::point ja tem operator== (era comparado campo a campo no original)
}

bool isAdjacent(bn::point a, bn::point b)
{
    // CORRIGIDO: o original comparava indices 1D (aScalar+-1), que "vaza" nas bordas
    // das linhas. o certo e distancia de Manhattan == 1 (exatamente um passo cardeal).
    int dx = a.x() - b.x();
    int dy = a.y() - b.y();
    if(dx < 0) dx = -dx;
    if(dy < 0) dy = -dy;
    return (dx + dy) == 1;
}

Direction getDirection(bn::point from, bn::point to)
{
    if(to.x() > from.x()) return Direction::east;
    if(to.x() < from.x()) return Direction::west;
    if(to.y() > from.y()) return Direction::south;
    if(to.y() < from.y()) return Direction::north;
    return Direction::none;
}

void pathSnapToShortest(Path* path, bn::point target, uint8_t reachable_tiles[grid_size])
{
    bn::point unit_pos = path->tiles[0];   // o heroi esta sempre em tiles[0]
    bn::point backwards[path_max_length];  // caminho do alvo de volta ao heroi
    int backwards_count = 0;

    bn::point current = target;
    int remaining = reachable_tiles[grid_index(current)];

    // "sobe a colina": no mapa do BFS, valor MAIOR = mais perto do heroi.
    // andamos sempre para o vizinho com valor exatamente +1 ate chegar no heroi.
    while(!sameTile(current, unit_pos) && backwards_count < path_max_length)
    {
        backwards[backwards_count++] = current;

        bool stepped = false;
        for(int i = 0; i < 4; ++i)
        {
            bn::point next = current + directions[i];
            if(next.x() < 0 || next.x() >= grid_width)  continue;
            if(next.y() < 0 || next.y() >= grid_height) continue;

            if(reachable_tiles[grid_index(next)] == remaining + 1)
            {
                current = next;
                remaining++;
                stepped = true;
                break;
            }
        }
        if(!stepped) break; // sem saida (nao deveria acontecer se o alvo e alcancavel)
    }

    // remonta o caminho na ordem certa: heroi primeiro, depois o resto invertido.
    path->count = 0;
    path->tiles[path->count++] = unit_pos;
    while(backwards_count > 0)
        path->tiles[path->count++] = backwards[--backwards_count];
}

void pathUpdate(Path* path, bn::point cursor_grid_pos, uint8_t reachable_tiles[grid_size])
{
    if(path->count == 0) return;
    if(reachable_tiles[grid_index(cursor_grid_pos)] == 0) return; // fora de alcance

    // 1) cursor voltou por cima da seta existente -> encurta ate ali (backtrack)
    for(int n = 0; n < path->count; ++n)
    {
        if(sameTile(path->tiles[n], cursor_grid_pos))
        {
            path->count = n + 1;
            return;
        }
    }

    // 2) cursor andou 1 casa a partir da ponta -> so estica o caminho
    bn::point tail = path->tiles[path->count - 1];
    int steps_used = path->count - 1;

    if(isAdjacent(tail, cursor_grid_pos) && steps_used < range_cap)
    {
        path->tiles[path->count++] = cursor_grid_pos;
        return;
    }

    // 3) pulo grande (cursor teleportou) -> recalcula o menor caminho ate la
    pathSnapToShortest(path, cursor_grid_pos, reachable_tiles);
}

// -------------------------------------------------------------------------------------
// Render do overlay via map cells (Passo 7)
// -------------------------------------------------------------------------------------
namespace
{
    // frames do atlas overlay.bmp (128x128, tiles 16x16 na linha 0 do atlas)
    constexpr int f_reachable = 0;
    constexpr int f_corner    = 1;
    constexpr int f_hor       = 2;
    constexpr int f_ver       = 3;
    constexpr int f_arrow_r   = 4;
    constexpr int f_arrow_u   = 5;
    constexpr int f_floor     = 6;
    constexpr int f_blank     = 7; // frame vazio -> tile transparente

    struct OverlayTile { int frame; bool hflip; bool vflip; };

    // indice do sub-tile 8x8 (sx,sy) dentro de um frame 16x16.
    // o atlas tem 16 tiles de 8x8 por linha; cada frame ocupa 2x2 tiles na linha 0.
    int overlay_tile(int frame, int sx, int sy) { return sy * 16 + frame * 2 + sx; }

    // escreve uma casa 16x16 = 2x2 celulas 8x8 no mapa.
    void set_meta(bn::regular_bg_map_cell* cells, const bn::regular_bg_map_item& item,
                  int casa_x, int casa_y, int frame, bool hflip, bool vflip)
    {
        int cx = casa_x * 2, cy = casa_y * 2; // casa -> celula do canto superior-esquerdo
        for(int sy = 0; sy < 2; ++sy)
            for(int sx = 0; sx < 2; ++sx)
            {
                // ao espelhar, troca-se qual sub-tile vai em cada canto E liga-se o flip
                int tsx = sx ^ (hflip ? 1 : 0);
                int tsy = sy ^ (vflip ? 1 : 0);

                bn::regular_bg_map_cell& cell = cells[item.cell_index(cx + sx, cy + sy)];
                bn::regular_bg_map_cell_info info(cell);
                info.set_tile_index(overlay_tile(frame, tsx, tsy));
                info.set_horizontal_flip(hflip);
                info.set_vertical_flip(vflip);
                cell = info.cell();
            }
    }

    // tabelas Direction -> tile, indexadas por int(Direction): none, north, south, west, east
    constexpr OverlayTile head_tiles[5] = {
        { f_blank,   false, false }, // none
        { f_arrow_u, false, false }, // north: seta pra cima
        { f_arrow_u, false, true  }, // south: seta pra cima espelhada na vertical
        { f_arrow_r, true,  false }, // west:  seta pra direita espelhada na horizontal
        { f_arrow_r, false, false }, // east:  seta pra direita
    };

    constexpr OverlayTile pipe_tiles[5] = {
        { f_blank, false, false }, // none
        { f_ver,   false, false }, // north
        { f_ver,   false, false }, // south
        { f_hor,   false, false }, // west
        { f_hor,   false, false }, // east
    };

    // canto: depende da direcao de ENTRADA e de SAIDA (mesma logica do CORNER_TILES antigo)
    OverlayTile corner_tile(Direction in, Direction out)
    {
        using D = Direction;
        if((in == D::north && out == D::east) || (in == D::west && out == D::south))
            return { f_corner, false, false };
        if((in == D::north && out == D::west) || (in == D::east && out == D::south))
            return { f_corner, true,  false };
        if((in == D::south && out == D::east) || (in == D::west && out == D::north))
            return { f_corner, false, true  };
        if((in == D::south && out == D::west) || (in == D::east && out == D::north))
            return { f_corner, true,  true  };
        return { f_blank, false, false };
    }
}

// preenche o mapa inteiro (15x10 casas) com o tile de chao. usado pela layer de baixo.
void drawFloor(bn::regular_bg_map_cell* cells, const bn::regular_bg_map_item& item)
{
    for(int y = 0; y < grid_height; ++y)
        for(int x = 0; x < grid_width; ++x)
            set_meta(cells, item, x, y, f_floor, false, false);
}

void drawMovementOverlay(bn::regular_bg_map_cell* cells, const bn::regular_bg_map_item& item,
                         uint8_t reachable_tiles[grid_size])
{
    // reconstroi TODO o overlay: casa alcancavel = quadrado, senao = transparente.
    for(int y = 0; y < grid_height; ++y)
        for(int x = 0; x < grid_width; ++x)
        {
            int frame = reachable_tiles[grid_index(bn::point(x, y))] > 0 ? f_reachable : f_blank;
            set_meta(cells, item, x, y, frame, false, false);
        }
}

void renderPathArrow(bn::regular_bg_map_cell* cells, const bn::regular_bg_map_item& item,
                     Path* path)
{
    if(path->count == 0)
        return;

    for(int i = 0; i < path->count; ++i)
    {
        // direcao de onde veio (in) e pra onde vai (out) neste ponto do caminho
        Direction dir_in  = (i > 0)               ? getDirection(path->tiles[i - 1], path->tiles[i])     : Direction::none;
        Direction dir_out = (i < path->count - 1) ? getDirection(path->tiles[i],     path->tiles[i + 1]) : Direction::none;

        OverlayTile t;
        if(i == 0)                        t = pipe_tiles[int(dir_out)];   // inicio: cano reto
        else if(i == path->count - 1)     t = head_tiles[int(dir_in)];    // fim: cabeca da seta
        else if(dir_in == dir_out)        t = pipe_tiles[int(dir_out)];   // meio RETO: cano
        else                              t = corner_tile(dir_in, dir_out); // meio: curva

        set_meta(cells, item, path->tiles[i].x(), path->tiles[i].y(), t.frame, t.hflip, t.vflip);
    }
}
