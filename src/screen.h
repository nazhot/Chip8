#ifndef CH8_H
#define CH8_H
#include <SDL2/SDL.h>
#define DISPLAY_WIDTH 64  //pixels, standard is 64
#define DISPLAY_HEIGHT 32 //pixels, standard is 32

/*
 * Holds all of the relevant information about a screen. Screens are used
 * to display the pixel data on.
 *
 * @member window    SDL window to draw to
 * @member renderer  SDL renderer that holds the drawn rectangles
 * @member xOffset   offset from left/right side
 * @member yOffset   offset from top/bottom side
 * @member pixelSize width/height of each pixel
 */
struct Screen {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int xOffset;
    int yOffset;
    int pixelSize;
};

/*
 * Initialize a screen for use by chip8
 *
 * This function will also initialize SDL. After that, it will create a window
 * (680px X 480px), create the needed renderer, and determine the size of the
 * square to represent each pixel.
 *
 * @return newly created Screen
 */
struct Screen* screen_initialize(); 

#endif
