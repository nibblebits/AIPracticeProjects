#include <stdio.h>
#include <SDL2/SDL.h>
#include "game.h"
int main()
{
    SDL_Init(0);

    // Create SDL window 256x256
    SDL_Window *window = SDL_CreateWindow("Game window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 100, 100, 0);

    // Get the renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Create the game
    struct game *game = game_create(renderer);

    // Game loop
    while (1)
    {
        if (game_process(game) == GAME_QUIT)
        {
            //End of game..
            break;
        }
        game_draw(game);
        SDL_Delay(1000);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}