#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <SDL2/SDL.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define BYTES_MEMORY 4096

#define DEBUG

#ifdef DEBUG
#define log(...) printf(__VA_ARGS__)
#else
#define log(...)
#endif

uint8_t memory[BYTES_MEMORY] = {0};
uint8_t display[DISPLAY_WIDTH][DISPLAY_HEIGHT] = {0};
uint8_t registers[16] = {0};
uint16_t indexRegister = 0;
uint16_t programCounter = 0;
uint16_t stack[16] = {0};
uint8_t delayTimer = 0;
uint8_t soundTimer = 0;

void displaySprite( uint8_t optionX, uint8_t optionY, uint8_t optionN ) {
    uint8_t xPos = registers[optionX] % DISPLAY_WIDTH;
    uint8_t yPos = registers[optionY] % DISPLAY_HEIGHT;
    registers[0xF] = 0;
    for ( int i = 0; i < optionN && yPos + i < DISPLAY_HEIGHT; ++i ) {
        assert( indexRegister + i < BYTES_MEMORY );
        uint8_t spriteByte = memory[indexRegister + i];
        for ( int j = 0; j < 8 && j + xPos < DISPLAY_WIDTH; ++j ) {
            uint8_t spritePixel = ( spriteByte >> ( 7 - j ) ) & 1;
            if ( spritePixel ) {
                if ( display[xPos + j][yPos + i] ) {
                    registers[0xF] = 1;
                }
                display[xPos + j][yPos + i] = !display[xPos + j][yPos + i];
            }
        }
    }
}


int main( int argc, char *argv[] ) {

    uint8_t fonts[16][5] = {
        { 0xF0, 0x90, 0x90, 0x90, 0xF0 }, 
        { 0x20, 0x60, 0x20, 0x20, 0x70 },
        { 0xF0, 0x10, 0xF0, 0x80, 0xF0 },
        { 0xF0, 0x10, 0xF0, 0x10, 0xF0 },
        { 0x90, 0x90, 0xF0, 0x10, 0x10 },
        { 0xF0, 0x80, 0xF0, 0x10, 0xF0 },
        { 0xF0, 0x80, 0xF0, 0x90, 0xF0 },
        { 0xF0, 0x10, 0x20, 0x40, 0x40 },
        { 0xF0, 0x90, 0xF0, 0x90, 0xF0 },
        { 0xF0, 0x90, 0xF0, 0x10, 0xF0 },
        { 0xF0, 0x90, 0xF0, 0x90, 0x90 },
        { 0xE0, 0x90, 0xE0, 0x90, 0xE0 },
        { 0xF0, 0x80, 0x80, 0x80, 0xF0 },
        { 0xE0, 0x90, 0x90, 0x90, 0xE0 },
        { 0xF0, 0x80, 0xF0, 0x80, 0xF0 },
        { 0xF0, 0x80, 0xF0, 0x80, 0x80 }
    };



    { //initialize the fonts into memory
        int fontIndex = 0x050;
        for ( int i = 0; i < 16; ++i ) {
            for ( int j = 0; j < 5; ++j ) {
                memory[fontIndex++] = fonts[i][j];
            }
        }
        printf( "Fonts initialized\n" );
    }

    { //read in the program file
        int startingProgramIndex = 0x200;
        int programIndex = startingProgramIndex;
        FILE *inputFile = fopen( "roms/IBM_Logo.ch8", "rb" );
        while ( fread( &memory[programIndex++], 1, 1, inputFile ) );
        programCounter = startingProgramIndex;
        printf( "Program read in: %d bytes, program starts at %x\n", programIndex - startingProgramIndex, startingProgramIndex );
    }

    if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
        fprintf( stderr, "Could not initialize SDL2\n" );
        return 1;
    }


    SDL_Window *window = SDL_CreateWindow( "CHIP-8", SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED, 680, 
                                           480, 0 );
    SDL_Renderer *renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    SDL_RenderClear( renderer );

    if ( !window ) {
        fprintf( stderr, "Could not create window\n" );
        return 1;
    }

    SDL_Surface *windowSurface = SDL_GetWindowSurface( window );

    if ( !windowSurface ) {
        fprintf( stderr, "Could not get surface from window\n" );
        return 1;
    }

    int pixelWidth = windowSurface->w / DISPLAY_WIDTH;
    int pixelHeight = windowSurface->h / DISPLAY_HEIGHT;

    int pixelSize = pixelWidth < pixelHeight ? pixelWidth : pixelHeight;
    int displayWidth = pixelSize * DISPLAY_WIDTH;
    int displayHeight = pixelSize * DISPLAY_HEIGHT;
    int displayXOffset = ( windowSurface->w - displayWidth ) / 2;
    int displayYOffset = ( windowSurface->h - displayHeight ) / 2;
    SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
    for ( int i = 0; i < DISPLAY_WIDTH; ++i ) {
        for ( int j = 0; j < DISPLAY_HEIGHT; ++j ) {
            SDL_Rect r = {displayXOffset + pixelSize * i,
                          displayYOffset + pixelSize * j,
                          pixelSize, pixelSize};
            //SDL_RenderFillRect( renderer, &r ); 
            SDL_RenderDrawRect( renderer, &r );
        }
    }

    SDL_RenderPresent( renderer );
    SDL_Delay( 5000 );

    SDL_Event e;

    while ( 1 ) {
        while ( SDL_PollEvent( &e ) > 0 ) {
            switch ( e.type ) {
                case SDL_QUIT:
                    exit( 1 );
            }
            SDL_UpdateWindowSurface( window );
        }
        //fetch
        uint16_t instruction = memory[programCounter] << 8 | memory[programCounter + 1];
        programCounter += 2;
        //decode
        uint8_t firstNibble = ( instruction & 0xF000 ) >> 12;
        uint8_t optionX = ( instruction & 0x0F00 ) >> 8;
        uint8_t optionY = ( instruction & 0x00F0 ) >> 4;
        uint8_t optionN = ( instruction & 0x000F );
        uint8_t optionNN = ( instruction & 0x00FF );
        uint8_t optionNNN = ( instruction & 0x0FFF );
        switch ( firstNibble ) {
            case 0x0:
                if ( instruction == 0x00E0 ) {
                    //clear screen
                    log( "Clearing screen\n" );
                    for ( int i = 0; i < DISPLAY_WIDTH; ++i ) {
                        for ( int j = 0; j < DISPLAY_HEIGHT; ++j ) {
                            display[i][j] = 0x00;
                        }
                    }
                }
                break;
            case 0x1:
                //jump to address
                log( "Jumping to %x from %x\n", programCounter, optionNNN );
                programCounter = optionNNN;
                break;
            case 0x2:
                break;
            case 0x3:
                break;
            case 0x4:
                break;
            case 0x5:
                break;
            case 0x6:
                //set register
                log( "Setting register %x to %x\n", optionX, optionNN );
                registers[optionX] = optionNN;
                break;
            case 0x7:
                //add to register
                log( "Adding %x to register %x\n", optionNN, optionX );
                registers[optionX] += optionNN;
                break;
            case 0x8:
                break;
            case 0x9:
                break;
            case 0xA:
                //set index register
                log( "Setting index register to %x\n", optionNN );
                indexRegister = optionNNN;
                break;
            case 0xB:
                break;
            case 0xC:
                break;
            case 0xD:
                log( "Displaying sprite with X: %x, Y: %x, N: %x\n", optionX, optionY, optionN );
                displaySprite( optionX, optionY, optionN );
                break;
            case 0xE:
                break;
            case 0xF:
                break;
        }
        //execute
    }
    return 0;
}
