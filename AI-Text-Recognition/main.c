#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "textrecognition.h"

void trainModel(int argc, char *argv[])
{
    // Train the csv
    textrecognition_train_csv("./training/train.csv");
}

void playModel(int argc, char *argv[])
{

    // Create and show SDL surface in a window
    SDL_Window *window = SDL_CreateWindow("Text Recognition", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 128, 128, SDL_WINDOW_SHOWN);
    if (!window)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return;
    }

    // Create image with white background 128x128
    SDL_Surface *image = SDL_CreateRGBSurface(0, 128, 128, 32, 0, 0, 0, 0);
    if (!image)
    {
        printf("Unable to create SDL surface! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return;
    }

    SDL_FillRect(image, NULL, SDL_MapRGB(image->format, 255, 255, 255));

    // Create a texture from the surface
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image);
    if (!texture)
    {
        printf("Unable to create texture from surface! SDL_Error: %s\n", SDL_GetError());
        SDL_FreeSurface(image);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return;
    }

    // Have it so when you click and drag it will draw black pixels
    SDL_bool done = SDL_FALSE;
    SDL_Event event;
    struct textrecognition_grayscale_bitmap *bitmap = NULL;

    while (!done)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                done = SDL_TRUE;
                break;

            case SDL_MOUSEBUTTONUP:
                int res = textrecognition_image_convert_sdl_surface_to_pixel_bitmap(image, &bitmap);
                if (res)
                {
                    printf("Unable to convert SDL surface to pixel bitmap!\n");
                    SDL_FreeSurface(image);
                    return;
                }

                struct textrecognition_process *process = textrecognition_load_model("./model.bin");
                double *input = bitmap->pixels;
                char predicted_char;
                textrecognition_predict(process, input, &predicted_char);
                printf("predicted char=%c\n", predicted_char);
                // Clear the image with white background
                SDL_FillRect(image, NULL, SDL_MapRGB(image->format, 255, 255, 255));
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    SDL_Rect rect = {event.button.x, event.button.y, 8, 8};
                    SDL_FillRect(image, &rect, SDL_MapRGB(image->format, 0, 0, 0));
                }
                break;
            case SDL_MOUSEMOTION:
                if (event.motion.state & SDL_BUTTON_LMASK)
                {
                    SDL_Rect rect = {event.motion.x, event.motion.y, 8, 8};
                    SDL_FillRect(image, &rect, SDL_MapRGB(image->format, 0, 0, 0));
                }
                break;
            }
        }

        // Update the texture with the new image
        SDL_UpdateTexture(texture, NULL, image->pixels, image->pitch);

        // Clear the renderer
        SDL_RenderClear(renderer);

        // Copy the texture to the renderer
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        // Present the renderer
        SDL_RenderPresent(renderer);
    }

    SDL_FreeSurface(image);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void testSingleImage(int argc, char* argv[])
{
    const char* filename = argv[2];
    struct textrecognition_process *process = textrecognition_load_model("./model.bin");
    SDL_Surface *surface = IMG_Load(filename);
    if (!surface)
    {
        printf("Unable to load image! SDL_Error: %s\n", SDL_GetError());
        return;
    }

    struct textrecognition_grayscale_bitmap *bitmap = NULL;

    int res = textrecognition_image_convert_sdl_surface_to_pixel_bitmap(surface, &bitmap);
    if (res)
    {
        printf("Unable to convert SDL surface to pixel bitmap!\n");
        SDL_FreeSurface(surface);
        return;
    }

    double *input = bitmap->pixels;

    char predicted_char;
    textrecognition_predict(process, input, &predicted_char);
    printf("predicted char=%c\n", predicted_char);
    
}
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: <mode> \n\n train or play");
        return -1;
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        printf("SDL_image could not initialize! IMG_Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    if (strncmp(argv[1], "train", 5) == 0)
    {
        trainModel(argc, argv);
    }
    else if (strncmp(argv[1], "play", 4) == 0)
    {
        playModel(argc, argv);
    }
    else if(strncmp(argv[1], "test", 4) == 0)
    {
        testSingleImage(argc, argv);
    }
    else
    {
        printf("Usage: <mode> \n\n train or play");
        return -1;
    }

    // Quit SDL_image
    IMG_Quit();

    // Quit SDL
    SDL_Quit();

    return 0;
}
