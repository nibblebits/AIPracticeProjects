#ifndef TEXTRECOGNITION_H
#define TEXTRECOGNITION_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stddef.h>

// 128 pixels width and height
#define TOTAL_NETWORK_INPUT_NODES 128*128
#define TOTAL_NEURONS_PER_HIDDEN_LAYER 128
#define TOTAL_NETWORK_OUTPUT_NODES 62
#define TOTAL_HIDDEN_LAYERS 1

struct textrecognition_grayscale_bitmap {
    // 1 = black 0 = white
    double *pixels;
    size_t width;
    size_t height;
};


struct textrecognition_training_process_neuron {
    double* weights;
    size_t total_weights_with_last_bias;

    // Our current output, to be passed as input to the next hidden layer.
    double output;

    double delta;  // the error term for backpropagation

};

struct textrecognition_training_process_layer {
    struct textrecognition_training_process_layer* previous_layer;
    struct textrecognition_training_process_layer* next_layer;

    struct textrecognition_training_process_neuron* neurons;
    size_t total_neurons_in_layer;
};

struct textrecognition_training_process {
    double* inputs[TOTAL_NETWORK_INPUT_NODES];
    struct textrecognition_training_process_layer *layers;
    size_t total_layers;

    double learning_rate;
};

int textrecognition_predict(struct textrecognition_training_process *process, double *input, char* gussed_char);
void textrecognition_train_character_for_file(struct textrecognition_training_process* process, const char *filename, char character);
int textrecognition_train_process_forward_pass(struct textrecognition_training_process *process, double *input);
int textrecognition_image_convert_sdl_surface_to_pixel_bitmap(SDL_Surface *surface, struct textrecognition_grayscale_bitmap **bitmap_out);
void textrecognition_train_csv(const char *filename);

struct textrecognition_training_process *textrecognition_load_model(const char *filename);

#endif