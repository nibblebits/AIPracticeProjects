#include "textrecognition.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


// Function to create a new grayscale bitmap
struct textrecognition_grayscale_bitmap *textrecognition_grayscale_bitmap_create(size_t width, size_t height) {
    struct textrecognition_grayscale_bitmap *bitmap = (struct textrecognition_grayscale_bitmap *)malloc(sizeof(struct textrecognition_grayscale_bitmap));
    if (!bitmap) {
        return NULL;
    }
    bitmap->pixels = (double *)malloc(width * height * sizeof(double));
    if (!bitmap->pixels) {
        free(bitmap);
        return NULL;
    }
    bitmap->width = width;
    bitmap->height = height;
    return bitmap;
}

// Function to convert a pixel color to black or white
int textrecognition_image_convert_pixel_to_black_or_white(SDL_Color color) {
    // If the color is not solid white, set it to black
    if (color.r < 255 || color.g < 255 || color.b < 255) {
        return 1;
    }
    return 0;
}

// Function to set a pixel in the bitmap to black or white
void textrecognition_image_set_pixel_to_black_or_white(SDL_Color color, int x, int y, double *pixels, size_t width, size_t height) {
    pixels[y * width + x] = (double) textrecognition_image_convert_pixel_to_black_or_white(color);
}

// Function to convert an SDL surface to a black and white bitmap no grayscale or inbetween
int textrecognition_image_convert_sdl_surface_to_pixel_bitmap(SDL_Surface *surface, struct textrecognition_grayscale_bitmap **bitmap_out) {
    int width = surface->w;
    int height = surface->h;
    struct textrecognition_grayscale_bitmap *bitmap = textrecognition_grayscale_bitmap_create(width, height);
    if (!bitmap) {
        return 1;
    }

    Uint8 *pixels = (Uint8 *)surface->pixels;
    int pitch = surface->pitch;
    SDL_Palette *palette = surface->format->palette;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Uint8 pixel_value = pixels[y * pitch + x];
            if (palette) {
                SDL_Color color = palette->colors[pixel_value];
                textrecognition_image_set_pixel_to_black_or_white(color, x, y, bitmap->pixels, width, height);
            } else {
                Uint32 pixel;
                memcpy(&pixel, &pixels[y * pitch + x * surface->format->BytesPerPixel], surface->format->BytesPerPixel);
                SDL_Color color;
                SDL_GetRGB(pixel, surface->format, &color.r, &color.g, &color.b);
                textrecognition_image_set_pixel_to_black_or_white(color, x, y, bitmap->pixels, width, height);
            }
        }
    }

    *bitmap_out = bitmap;
    return 0;
}
