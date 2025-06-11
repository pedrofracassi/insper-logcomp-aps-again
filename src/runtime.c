#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Canvas canvas = {NULL, 0, 0};

#define ANSI_RESET "\033[0m"
#define ANSI_BOLD "\033[1m"

void init_canvas(int width, int height) {
    if (width > MAX_CANVAS_WIDTH) width = MAX_CANVAS_WIDTH;
    if (height > MAX_CANVAS_HEIGHT) height = MAX_CANVAS_HEIGHT;
    if (width <= 0) width = DEFAULT_CANVAS_WIDTH;
    if (height <= 0) height = DEFAULT_CANVAS_HEIGHT;
    
    canvas.width = width;
    canvas.height = height;
    
    canvas.buffer = (Color*)calloc(width * height, sizeof(Color));
    if (!canvas.buffer) {
        fprintf(stderr, "Failed to allocate canvas buffer\n");
        exit(1);
    }
    
    clear_canvas();
}

void cleanup_canvas() {
    if (canvas.buffer) {
        free(canvas.buffer);
        canvas.buffer = NULL;
    }
    canvas.width = 0;
    canvas.height = 0;
}

void paint_pixel(int x, int y, int color) {
    if (x < 0 || x >= canvas.width || y < 0 || y >= canvas.height) {
        return;
    }
    
    int index = y * canvas.width + x;
    
    canvas.buffer[index] = value_to_color(color);
}

int get_canvas_width() {
    return canvas.width;
}

int get_canvas_height() {
    return canvas.height;
}

Color value_to_color(int value) {
    Color color;
    
    int ansi_color = value % 8;
    if (ansi_color < 0) ansi_color += 8;

    color.code = ansi_color;
    
    return color;
}

void get_ansi_color(Color color, char* buffer) {
    // 0=preto
    // 1=vermelho
    // 2=verde
    // 3=amarelo
    // 4=azul
    // 5=magenta
    // 6=ciano
    // 7=branco
    sprintf(buffer, "\033[%dm", 40 + color.code);
}

void render_pixel(Color color) {
    char ansi_color[32];
    get_ansi_color(color, ansi_color);
    printf("%s  %s", ansi_color, ANSI_RESET);
}

void render_canvas() {
    for (int y = 0; y < canvas.height; y++) {
        for (int x = 0; x < canvas.width; x++) {
            int index = y * canvas.width + x;
            render_pixel(canvas.buffer[index]);
        }
        printf("\n");
    }
}

void clear_canvas() {
    if (canvas.buffer) {
        memset(canvas.buffer, 0, canvas.width * canvas.height * sizeof(Color));
    }
}