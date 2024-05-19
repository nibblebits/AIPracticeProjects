#include "textrecognition.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define MAX_LINE_LENGTH 1024
#define MAX_FIELDS 100
struct textrecognition_training_process *textrecognition_load_model(const char *filename);

void textrecognition_train_process_randomize_weights_and_bias(struct textrecognition_training_process_neuron *neuron)
{
    for (int i = 0; i < neuron->total_weights_with_last_bias; i++)
    {
        neuron->weights[i] = ((double)rand() / RAND_MAX) - 0.5;
    }
}

struct textrecognition_training_process *textrecognition_train_process_create(double learning_rate, size_t total_layers)
{
    if (total_layers < 2)
    {
        return NULL;
    }
    struct textrecognition_training_process *process = calloc(1, sizeof(struct textrecognition_training_process));
    process->learning_rate = learning_rate;
    process->total_layers = total_layers;
    process->layers = calloc(process->total_layers, sizeof(struct textrecognition_training_process_layer));
    struct textrecognition_training_process_layer *previous_layer = NULL;
    for (size_t i = 0; i < process->total_layers; i++)
    {
        struct textrecognition_training_process_layer *hidden_layer = &process->layers[i];
        hidden_layer->previous_layer = previous_layer;
        hidden_layer->next_layer = NULL; // Initialize next_layer as NULL
        if (previous_layer)
        {
            previous_layer->next_layer = hidden_layer; // Link to the next layer
        }
        size_t total_neurons_in_this_layer = (i == process->total_layers - 1) ? TOTAL_NETWORK_OUTPUT_NODES : TOTAL_NEURONS_PER_HIDDEN_LAYER;
        hidden_layer->total_neurons_in_layer = total_neurons_in_this_layer;
        hidden_layer->neurons = calloc(hidden_layer->total_neurons_in_layer, sizeof(struct textrecognition_training_process_neuron));

        for (size_t b = 0; b < hidden_layer->total_neurons_in_layer; b++)
        {
            struct textrecognition_training_process_neuron *neuron = &hidden_layer->neurons[b];
            neuron->total_weights_with_last_bias = (i == 0) ? TOTAL_NETWORK_INPUT_NODES : previous_layer->total_neurons_in_layer + 1;
            neuron->weights = calloc(neuron->total_weights_with_last_bias, sizeof(double));
            textrecognition_train_process_randomize_weights_and_bias(neuron);
            neuron->output = 0;
        }

        previous_layer = hidden_layer;
    }

    return process;
}

// Sigmoid function definition
double sigmoid(double x)
{
    return 1.0 / (1.0 + exp(-x));
}

struct textrecognition_training_process_layer *textrecognition_train_process_get_output_layer(struct textrecognition_training_process *process)
{
    return &process->layers[process->total_layers - 1];
}

void calculate_hidden_layer_error(struct textrecognition_training_process_layer *layer)
{
    struct textrecognition_training_process_layer *next_layer = layer->next_layer;
    if (next_layer == NULL)
    {
        fprintf(stderr, "Error: next_layer is NULL in calculate_hidden_layer_error\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < layer->total_neurons_in_layer; i++)
    {
        struct textrecognition_training_process_neuron *neuron = &layer->neurons[i];
        double error = 0.0;
        for (size_t j = 0; j < next_layer->total_neurons_in_layer; j++)
        {
            error += next_layer->neurons[j].weights[i] * next_layer->neurons[j].delta;
        }
        neuron->delta = error * neuron->output * (1 - neuron->output);

        // Print the error for debugging
        // printf("Hidden Layer Neuron %zu Error: %f, Delta: %f\n", i, error, neuron->delta);
    }
}

void update_weights(struct textrecognition_training_process *process, double *input)
{
    for (size_t l = 0; l < process->total_layers; l++)
    {
        struct textrecognition_training_process_layer *layer = &process->layers[l];
        for (size_t j = 0; j < layer->total_neurons_in_layer; j++)
        {
            struct textrecognition_training_process_neuron *neuron = &layer->neurons[j];
            for (size_t k = 0; k < neuron->total_weights_with_last_bias - 1; k++)
            {
                double input_d = (l == 0) ? input[k] : layer->previous_layer->neurons[k].output;
                neuron->weights[k] += process->learning_rate * neuron->delta * input_d;

                // Print updated weights for debugging
                //   printf("Layer %zu Neuron %zu Weight %zu Updated Weight: %f\n", l, j, k, neuron->weights[k]);
            }
            neuron->weights[neuron->total_weights_with_last_bias - 1] += process->learning_rate * neuron->delta; // Update bias
        }
    }
}

void calculate_output_layer_error(struct textrecognition_training_process *process, double *target_output)
{
    struct textrecognition_training_process_layer *output_layer = &process->layers[process->total_layers - 1];
    for (size_t i = 0; i < output_layer->total_neurons_in_layer; i++)
    {
        struct textrecognition_training_process_neuron *neuron = &output_layer->neurons[i];
        double error = target_output[i] - neuron->output;
        neuron->delta = error * neuron->output * (1 - neuron->output);

        // Print the delta for debugging
        //  printf("Output Layer Neuron %zu Error: %f, Delta: %f\n", i, error, neuron->delta);
    }
}

void train_single(struct textrecognition_training_process *process, double *input, double *target_output)
{
    // printf("Starting forward pass\n");
    textrecognition_train_process_forward_pass(process, input);
    // printf("Forward pass completed\n");

    //  printf("Calculating output layer error\n");
    calculate_output_layer_error(process, target_output);
    // printf("Output layer error calculated\n");

    // Calculate error for hidden layers
    for (int l = process->total_layers - 2; l >= 0; l--)
    {
        //        printf("Calculating hidden layer error for layer %d\n", l);
        calculate_hidden_layer_error(&process->layers[l]);
    }

    // Update weights
    update_weights(process, input);
}

void train_single_character(struct textrecognition_training_process *process, double *input, char character)
{
    double target_output[TOTAL_NETWORK_OUTPUT_NODES] = {0};
    if (character >= '0' && character <= '9')
    {
        target_output[character - '0'] = 1;
    }
    else if (character >= 'A' && character <= 'Z')
    {
        target_output[character - 'A' + 10] = 1;
    }
    else if (character >= 'a' && character <= 'z')
    {
        target_output[character - 'a' + 36] = 1;
    }

    train_single(process, input, target_output);
}

int textrecognition_train_process_forward_pass(struct textrecognition_training_process *process, double *input)
{
    for (size_t l = 0; l < process->total_layers; l++)
    {
        struct textrecognition_training_process_layer *layer = &process->layers[l];
        for (size_t b = 0; b < layer->total_neurons_in_layer; b++)
        {
            struct textrecognition_training_process_neuron *neuron = &layer->neurons[b];
            double neuron_output = 0.0;
            for (size_t c = 0; c < neuron->total_weights_with_last_bias - 1; c++)
            {
                double input_d = (l == 0) ? input[c] : layer->previous_layer->neurons[c].output;
                neuron_output += input_d * neuron->weights[c];
            }
            neuron_output += neuron->weights[neuron->total_weights_with_last_bias - 1];
            neuron->output = sigmoid(neuron_output);

            // Print neuron output for debugging
            // printf("Layer %zu Neuron %zu Output: %f\n", l, b, neuron->output);
        }
    }
    return 0;
}

char output_index_to_character(int index)
{
    if (index < 10)
    {
        return '0' + index;
    }
    else if (index < 36)
    {
        return 'A' + index - 10;
    }
    else
    {
        return 'a' + index - 36;
    }

    return '@';
}

int textrecognition_predict(struct textrecognition_training_process *process, double *input, char *gussed_char)
{
    textrecognition_train_process_forward_pass(process, input);
    struct textrecognition_training_process_layer *output_layer = textrecognition_train_process_get_output_layer(process);
    // Get the highest output node that will be the guessed character
    double max_output = 0;
    int max_output_index = 0;
    for (size_t i = 0; i < output_layer->total_neurons_in_layer; i++)
    {
        struct textrecognition_training_process_neuron *neuron = &output_layer->neurons[i];
        if (neuron->output > max_output)
        {
            max_output = neuron->output;
            max_output_index = i;
        }
    }
    *gussed_char = output_index_to_character(max_output_index);
    return 0;
}

void textrecognition_train_character_for_file(struct textrecognition_training_process *process, const char *filename, char character)
{
    // printf("Loading image file: %s\n", filename);
    SDL_Surface *image = IMG_Load(filename);
    if (!image)
    {
        printf("Unable to load PNG image %s! IMG_Error: %s\n", filename, IMG_GetError());
        return;
    }

    struct textrecognition_grayscale_bitmap *bitmap = NULL;
    int res = textrecognition_image_convert_sdl_surface_to_pixel_bitmap(image, &bitmap);
    if (res)
    {
        printf("Unable to convert SDL surface to pixel bitmap for file %s!\n", filename);
        SDL_FreeSurface(image);
        return;
    }

    train_single_character(process, bitmap->pixels, character);

    // Free the loaded image
    SDL_FreeSurface(image);

    // Free the bitmap
    free(bitmap->pixels);
    free(bitmap);
}

double calculate_accuracy(FILE *file, struct textrecognition_training_process *process)
{
    // Get current position in file

    long offset = ftell(file);
    fseek(file, 0, SEEK_SET);
    char line[MAX_LINE_LENGTH];
    int correct_predictions = 0;
    int total_predictions = 0;

    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n")] = '\0';
        char *fields[MAX_FIELDS];
        int fieldCount = 0;
        char *token = strtok(line, ",");
        while (token)
        {
            fields[fieldCount++] = token;
            token = strtok(NULL, ",");
        }

        if (fieldCount != 2)
        {
            //  printf("Invalid line: %s\n", line);
            continue;
        }

        SDL_Surface *image = IMG_Load(fields[0]);
        if (!image)
        {
            // printf("Unable to load PNG image %s! IMG_Error: %s\n", fields[0], IMG_GetError());
            continue;
        }
        struct textrecognition_grayscale_bitmap *bitmap = NULL;
        int res = textrecognition_image_convert_sdl_surface_to_pixel_bitmap(image, &bitmap);
        if (res)
        {
            printf("Unable to convert SDL surface to pixel bitmap for file %s!\n", fields[0]);
            SDL_FreeSurface(image);
            continue;
        }

        double *input = bitmap->pixels;
        char predicted_char;
        textrecognition_predict(process, input, &predicted_char);

        if (predicted_char == fields[1][0])
        {
            correct_predictions++;
        }
        total_predictions++;

        SDL_FreeSurface(image);
        free(bitmap->pixels);
        free(bitmap);
    }

    // Reset file position
    fseek(file, offset, SEEK_SET);

    return (double)correct_predictions / total_predictions;
}

void textrecognition_save_model(struct textrecognition_training_process *process, const char *filename)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("Unable to open file for writing");
        return;
    }

    fwrite(&process->learning_rate, sizeof(double), 1, file);
    fwrite(&process->total_layers, sizeof(size_t), 1, file);

    for (size_t l = 0; l < process->total_layers; l++)
    {
        struct textrecognition_training_process_layer *layer = &process->layers[l];
        fwrite(&layer->total_neurons_in_layer, sizeof(size_t), 1, file);

        for (size_t n = 0; n < layer->total_neurons_in_layer; n++)
        {
            struct textrecognition_training_process_neuron *neuron = &layer->neurons[n];
            fwrite(&neuron->total_weights_with_last_bias, sizeof(size_t), 1, file);
            fwrite(neuron->weights, sizeof(double), neuron->total_weights_with_last_bias, file);
        }
    }

    fclose(file);
}
struct textrecognition_training_process *textrecognition_load_model(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Unable to open file for reading");
        return NULL;
    }

    double learning_rate;
    size_t total_layers;

    // Read learning_rate and total_layers from the file
    if (fread(&learning_rate, sizeof(double), 1, file) != 1)
    {
        fprintf(stderr, "Error reading learning_rate\n");
        fclose(file);
        return NULL;
    }

    if (fread(&total_layers, sizeof(size_t), 1, file) != 1)
    {
        fprintf(stderr, "Error reading total_layers\n");
        fclose(file);
        return NULL;
    }

    printf("Loaded learning_rate: %f, total_layers: %zu\n", learning_rate, total_layers);

    struct textrecognition_training_process *process = textrecognition_train_process_create(learning_rate, total_layers);
    if (!process)
    {
        fprintf(stderr, "Error creating training process\n");
        fclose(file);
        return NULL;
    }

    for (size_t l = 0; l < process->total_layers; l++)
    {
        struct textrecognition_training_process_layer *layer = &process->layers[l];

        // Read the number of neurons in this layer
        if (fread(&layer->total_neurons_in_layer, sizeof(size_t), 1, file) != 1)
        {
            fprintf(stderr, "Error reading total_neurons_in_layer for layer %zu\n", l);
            fclose(file);
            return NULL;
        }

        printf("Layer %zu: total_neurons_in_layer = %zu\n", l, layer->total_neurons_in_layer);

        layer->neurons = calloc(layer->total_neurons_in_layer, sizeof(struct textrecognition_training_process_neuron));
        if (!layer->neurons)
        {
            fprintf(stderr, "Error allocating memory for neurons in layer %zu\n", l);
            fclose(file);
            return NULL;
        }

        for (size_t n = 0; n < layer->total_neurons_in_layer; n++)
        {
            struct textrecognition_training_process_neuron *neuron = &layer->neurons[n];

            // Read the total weights with last bias for this neuron
            if (fread(&neuron->total_weights_with_last_bias, sizeof(size_t), 1, file) != 1)
            {
                fprintf(stderr, "Error reading total_weights_with_last_bias for neuron %zu in layer %zu\n", n, l);
                fclose(file);
                return NULL;
            }

            printf("Neuron %zu in Layer %zu: total_weights_with_last_bias = %zu\n", n, l, neuron->total_weights_with_last_bias);

            neuron->weights = calloc(neuron->total_weights_with_last_bias, sizeof(double));
            if (!neuron->weights)
            {
                fprintf(stderr, "Error allocating memory for weights of neuron %zu in layer %zu\n", n, l);
                fclose(file);
                return NULL;
            }

            // Read the weights
            if (fread(neuron->weights, sizeof(double), neuron->total_weights_with_last_bias, file) != neuron->total_weights_with_last_bias)
            {
                fprintf(stderr, "Error reading weights for neuron %zu in layer %zu\n", n, l);
                fclose(file);
                return NULL;
            }
        }
    }

    fclose(file);
    return process;
}

void textrecognition_train_csv(const char *filename)
{
    // struct textrecognition_training_process *process = textrecognition_train_process_create(0.001, TOTAL_HIDDEN_LAYERS + 1);
    struct textrecognition_training_process *process = textrecognition_load_model("./model.bin");
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Unable to open file");
        return;
    }
    for (int i = 0; i < 100; i++)
    {
        printf("Training epoch %i\n", i);
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), file))
        {
            // Remove newline character
            line[strcspn(line, "\n")] = '\0';

            // Split line into fields
            char *fields[MAX_FIELDS];
            int fieldCount = 0;
            char *token = strtok(line, ",");
            while (token)
            {
                fields[fieldCount++] = token;
                token = strtok(NULL, ",");
            }

            if (fieldCount != 2)
            {
                // printf("Invalid line: %s\n", line);
                continue;
            }

            textrecognition_train_character_for_file(process, fields[0], fields[1][0]);
        }

        textrecognition_save_model(process, "./model.bin");
        if (i % 5 == 0)
        {
            printf("Model Accuracy: %f\n", calculate_accuracy(file, process));
        }
        fseek(file, 0, SEEK_SET);
    }

    fclose(file);
}
