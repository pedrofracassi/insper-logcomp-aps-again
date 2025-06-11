#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdint.h>

#define DEFAULT_CANVAS_WIDTH 100
#define DEFAULT_CANVAS_HEIGHT 100
#define MAX_CANVAS_WIDTH 200
#define MAX_CANVAS_HEIGHT 200

typedef struct {
    uint8_t code;
} Color;

typedef struct {
    Color* buffer;
    int width;
    int height;
} Canvas;

void init_canvas(int width, int height);
void cleanup_canvas();
void paint_pixel(int x, int y, int color);
int get_canvas_width();
int get_canvas_height();
void render_canvas();
void clear_canvas();
Color value_to_color(int value);

#endif