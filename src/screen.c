#include "screen.h"

struct Screen* screen_initialize() {
    struct Screen *screen = malloc( sizeof( struct Screen ) );
    if ( !screen ) {
        fprintf( stderr, "Could not create new struct Screen\n" );
        exit( 1 );
    }
    if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) { //TODO: may want to move out of this function
        fprintf( stderr, "Could not initialize SDL2\n" );
        exit( 1 );
    }

    screen->window = SDL_CreateWindow( "CHIP-8", SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED, 680, 
                                           480, 0 );
    if ( !screen->window ) {
        fprintf( stderr, "Could not create window\n" );
        exit( 1 );
    }

    screen->renderer = SDL_CreateRenderer( screen->window, -1, 
                                                 SDL_RENDERER_ACCELERATED );
    if ( !screen->renderer ) {
        fprintf( stderr, "Could not create renderer\n" );
        exit( 1 );
    }
    SDL_RenderClear( screen->renderer );
    
    int screenWidth;
    int screenHeight;
    SDL_GetWindowSize( screen->window, &screenWidth, &screenHeight );
    int pixelWidth = screenWidth / DISPLAY_WIDTH;
    int pixelHeight = screenHeight / DISPLAY_HEIGHT;
    int pixelSize = pixelWidth < pixelHeight ? pixelWidth : pixelHeight;
    int displayWidth = pixelSize * DISPLAY_WIDTH;
    int displayHeight = pixelSize * DISPLAY_HEIGHT;
    int displayXOffset = ( screenWidth - displayWidth ) / 2;
    int displayYOffset = ( screenHeight   - displayHeight ) / 2;

    screen->pixelSize = pixelSize;
    screen->xOffset = displayXOffset;
    screen->yOffset = displayYOffset;

    return screen;
}
