#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define FPS 120
#define FRAME_TARGET_TIME (1000 / FPS) // How many ms each frame should take

extern SDL_Renderer* renderer;
extern SDL_Window* window;
extern uint32_t* color_buffer;
extern SDL_Texture* color_buffer_texture;	// SDL Texture som brukes til å vise innholdet i fargebuffere
extern int window_width;
extern int window_height;


bool initialize_window(void);
void draw_grid(void);
void draw_pixel(int x, int y, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void render_color_buffer(void);
void clear_color_buffer(uint32_t color);
void destroy_window(void);

#endif


