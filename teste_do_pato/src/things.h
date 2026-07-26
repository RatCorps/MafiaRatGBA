#ifndef THINGS_H
#define THINGS_H

#include "bn_fixed.h" // ponto fixo do butano
#include "bn_fixed_point.h" // ponto 2d do butano
#include "bn_fixed_point_fwd.h"
#include "bn_point.h" // ponto 2d dos inteiros
#include "bn_regular_bg_map_cell.h" // celula de mapa (uint16_t) do background
#include "bn_regular_bg_map_item.h"// descreve o array de celulas + tamanho
#include "bn_pool.h" // alocador de capacidade fixa
#include "bn_intrusive_list.h" // lista de elos
#include <cstddef>
#include <cstdint>

constexpr int screen_width = 240;
constexpr int screen_height = 160;
constexpr int tile_size = 16;

constexpr int grid_width = screen_width/tile_size;//15
constexpr int grid_height = screen_height/tile_size;//10
constexpr int grid_size = grid_width * grid_height;//150

// Butano usa (0,0) = CENTRO da tela. Estes offsets convertem entre o
// espaco do tabuleiro (canto superior-esquerdo) e o espaco do Butano.
constexpr int screen_half_width = screen_width / 2;   // 120
constexpr int screen_half_height = screen_height / 2; // 80
constexpr int half_tile = tile_size / 2;              // 8

constexpr int max_things = 256;// capacidade total de things
constexpr int max_alarms = 4;//contadores das entidades

constexpr int path_max_length = 32; // tamanho maximo do caminho da seta
constexpr int range_cap = 5;        // maior range possivel + 1 (trava de seguranca)

enum class Kind : uint8_t
{
    nil, //slot vazio
    cursor,
    unit,
    particle,
    projectile,
    count // igual o numero de kinds seila to escrevendo so pra manter documentado depois tu ajeita piscadinha web ^_O

};

enum class Team : uint8_t
{
    rat,
    cat
};

enum class Rank : uint8_t
{
    infantry,
    made_man,
    meccanizata,
    cecchino,
    underboss,
    caporegime
};

// ATENCAO: a ordem importa. o Passo 7 vai indexar tabelas de tiles por Direction.
enum class Direction : uint8_t
{
    none,
    north,
    south,
    west,
    east,
    count
};

// o caminho da seta: uma sequencia de casas, do heroi ate o cursor.
struct Path
{
    bn::point tiles[path_max_length];
    int count = 0;
};

// next e previous nome auto explicativo
struct Thing : public bn::intrusive_list_node_type {

    bn::fixed_point pos;

    int16_t alarms[max_alarms];

    Kind kind = Kind::nil;
    int8_t sprite_id = 0;
    int8_t health = 0;

    union Payload
    {

    struct Unit
    {
        uint8_t morale = 0;
        Rank rank = Rank::infantry;
        Team team = Team::rat;
    } unit;
        Payload() : unit() {}
    } payload;
};

struct State
{

    bn::pool<Thing, max_things> pool; // lugar onde fica as things

    //uma lista que vai indo por kind
    bn::intrusive_list<Thing> kind_list[int(Kind::count)];

    uint8_t reachable_tiles[grid_size];
    Path path;

    Thing* grid[grid_size] = {};
    Thing* selected_unit = nullptr;
};

// tabelas de dados de jogo (definidas em things.cpp)
extern const int unit_ranges[6];       // alcance de movimento por Rank

void init(State* state);
Thing& create(State* state, Kind kind);
void destroy(State* state, Thing& thing);

bn::point world2grid(bn::fixed_point world_pos); // pixels -> casa do tabuleiro
bn::fixed_point grid2world(bn::point grid_pos);   // casa do tabuleiro -> pixels
int grid_index(bn::point grid_pos);               // casa (x,y) -> indice 1D

// BFS: marca em reachable_tiles quais casas a unidade alcanca com max_range passos
void calculateMovementRange(bn::point start, int max_range, uint8_t reachable_tiles[grid_size]);

bool sameTile(bn::point a, bn::point b);      // as duas casas sao a mesma?
bool isAdjacent(bn::point a, bn::point b);    // as casas sao vizinhas (1 passo)?
Direction getDirection(bn::point from, bn::point to); // direcao de from -> to
void pathSnapToShortest(Path* path, bn::point target, uint8_t reachable_tiles[grid_size]);
void pathUpdate(Path* path, bn::point cursor_grid_pos, uint8_t reachable_tiles[grid_size]);

// preenche uma layer inteira com o tile de chao (usada como background por baixo do overlay)
void drawFloor(bn::regular_bg_map_cell* cells, const bn::regular_bg_map_item& item);

// desenha o overlay (casas alcancaveis) e a seta no mapa de celulas do background
void drawMovementOverlay(bn::regular_bg_map_cell* cells, const bn::regular_bg_map_item& item,
                         uint8_t reachable_tiles[grid_size]);
void renderPathArrow(bn::regular_bg_map_cell* cells, const bn::regular_bg_map_item& item,
                     Path* path);



#endif
