#include "game.h"
#include <memory.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <time.h>

void game_clear_screen(struct game* game){
    SDL_Renderer* renderer = game->renderer;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void game_draw_title(struct game* game, size_t x, size_t y){
    SDL_Renderer* renderer = game->renderer;
    MAP_TILE tile = game->tiles[y * game->width + x];
    SDL_Rect rect = {x * TILE_SIZE_WIDTH, y * TILE_SIZE_HEIGHT, TILE_SIZE_WIDTH, TILE_SIZE_HEIGHT};
    switch(tile){
        case GAME_TILE_EMPTY:
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            break;
        case GAME_TILE_WALL:
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            break;
    
        case GAME_TILE_HAMBURGER:
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            break;
    }
    SDL_RenderFillRect(renderer, &rect);
}

void game_draw_player(struct game* game, struct player* player){
    size_t x = player->x;
    size_t y = player->y;
    MAP_TILE tile = game->tiles[y * game->width + x];
    SDL_Rect rect = {x * TILE_SIZE_WIDTH, y * TILE_SIZE_HEIGHT, TILE_SIZE_WIDTH, TILE_SIZE_HEIGHT};

    SDL_SetRenderDrawColor(game->renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(game->renderer, &rect);
}

void game_draw(struct game* game){
    game_clear_screen(game);
    for(size_t y = 0; y < game->height; y++){
        for(size_t x = 0; x < game->width; x++){
            game_draw_title(game, x, y);
        }
    }

    // Draw the player
    game_draw_player(game, game->player);
    SDL_RenderPresent(game->renderer);
}

void game_set_title(struct game* game, size_t x, size_t y, MAP_TILE tile){
    game->tiles[y * game->width + x] = tile;
}

void game_create_initialize_map(struct game* game){
    size_t total_tiles = game->width * game->height;
    memset(game->tiles, GAME_TILE_EMPTY, total_tiles * sizeof(MAP_TILE));
    // Set hamburger in random location
    size_t hamburger_x = rand() % game->width;
    size_t hamburger_y = rand() % game->height;
    game_set_title(game, hamburger_x, hamburger_y, GAME_TILE_HAMBURGER);
}

void game_create_initialize_player(struct game* game){
    game->player = calloc(1, sizeof(struct player));
    game->player->x = 0;
    game->player->y = 0;
}


struct game* game_create(struct SDL_Renderer* renderer){
    // Initialize random
    srand(time(NULL));

    struct game* game = calloc(1, sizeof(struct game));
    game->width = GAME_SIZE_WIDTH;
    game->height = GAME_SIZE_HEIGHT;

    size_t total_tiles = game->width * game->height;
    game->tiles = calloc(total_tiles, sizeof(MAP_TILE));
    game_create_initialize_map(game);

    game_create_initialize_player(game);
    game->renderer = renderer;
    return game;
}

int game_process(struct game* game)
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_QUIT)
        {
            return GAME_QUIT;
        }
    }
    return GAME_CONTINUE;
}