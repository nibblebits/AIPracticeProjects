#ifndef GAME_H
#define GAME_H
#include <stdint.h>
#include <stddef.h>
#include <SDL2/SDL.h>

#define GAME_QUIT 1
#define GAME_CONTINUE 0

// Total tiles in either direction for the game
#define GAME_SIZE_WIDTH 10
#define GAME_SIZE_HEIGHT 10

// Tile sizes in pixels
#define TILE_SIZE_WIDTH 10
#define TILE_SIZE_HEIGHT 10

enum
{
    GAME_TILE_EMPTY,
    GAME_TILE_WALL,
    GAME_TILE_HAMBURGER,
};

typedef int MAP_TILE;

struct player
{
    size_t x;
    size_t y;
};
struct game
{
    size_t width;
    size_t height;


    // 2D Array of map tiles based on width and height
    MAP_TILE* tiles;
    struct player* player;
    
    SDL_Renderer* renderer;
};

// game create
struct game* game_create(SDL_Renderer* renderer);
void game_draw(struct game* game);
int game_process(struct game* game);


#endif
