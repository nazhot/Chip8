#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <SDL2/SDL.h>

#define DISPLAY_WIDTH 64  //pixels, standard is 64
#define DISPLAY_HEIGHT 32 //pixels, standard is 32
#define BYTES_MEMORY 4096 //standard is 4096

#define DEBUG  //define to print debug messages
#define STEP 0 //whether to wait for user input to step through instructions

#ifdef DEBUG
#define log(...) printf(__VA_ARGS__) //macro for logging
#else
#define log(...) //if not debugging, don't printf
#endif

struct Chip8 {
    uint8_t memory[BYTES_MEMORY];
    int8_t display[DISPLAY_WIDTH][DISPLAY_HEIGHT];
    uint8_t registers[16];
    uint16_t indexRegister;
    uint16_t programCounter;
    uint16_t stack[16];
    uint8_t delayTimer;
    uint8_t soundTimer;
    uint16_t startingFontAddress;
    uint16_t startingProgramAddress;
    uint16_t currentInstruction;
    uint8_t firstNibble;
    uint8_t optionX;
    uint8_t optionY;
    uint8_t optionN;
    uint8_t optionNN;
    uint16_t optionNNN;
};

struct Chip8* ch8_initialize() {
    struct Chip8 *chip = malloc( sizeof( struct Chip8 ) );
    memset( chip, 0, sizeof( struct Chip8 ) );
    chip->startingProgramAddress = 0x200;
    chip->programCounter = 0x200;
    return chip;
}

void ch8_initializeFonts( struct Chip8 *chip, uint16_t startingAddress ) {
    static uint8_t fonts[16][5] = {
        { 0xF0, 0x90, 0x90, 0x90, 0xF0 }, //0
        { 0x20, 0x60, 0x20, 0x20, 0x70 }, //1
        { 0xF0, 0x10, 0xF0, 0x80, 0xF0 }, //2
        { 0xF0, 0x10, 0xF0, 0x10, 0xF0 }, //3
        { 0x90, 0x90, 0xF0, 0x10, 0x10 }, //4
        { 0xF0, 0x80, 0xF0, 0x10, 0xF0 }, //5
        { 0xF0, 0x80, 0xF0, 0x90, 0xF0 }, //6
        { 0xF0, 0x10, 0x20, 0x40, 0x40 }, //7
        { 0xF0, 0x90, 0xF0, 0x90, 0xF0 }, //8
        { 0xF0, 0x90, 0xF0, 0x10, 0xF0 }, //9
        { 0xF0, 0x90, 0xF0, 0x90, 0x90 }, //A
        { 0xE0, 0x90, 0xE0, 0x90, 0xE0 }, //B
        { 0xF0, 0x80, 0x80, 0x80, 0xF0 }, //C
        { 0xE0, 0x90, 0x90, 0x90, 0xE0 }, //D
        { 0xF0, 0x80, 0xF0, 0x80, 0xF0 }, //E
        { 0xF0, 0x80, 0xF0, 0x80, 0x80 }  //F
    };
    chip->startingFontAddress = startingAddress; 
    for ( int i = 0; i < 16; ++i ) {
        for ( int j = 0; j < 5; ++j ) {
            chip->memory[startingAddress] = fonts[i][j];
        }
    }
    printf( "Fonts initialized\n" );
}

void ch8_loadFileIntoMemory( struct Chip8 *chip, const char filePath[] ) {
    FILE *inputFile = fopen( filePath, "rb" );
    if ( !inputFile ) {
        fprintf( stderr, "Cannot find file at path %s\n", filePath );
        exit( 1 );
    }
    uint16_t programAddress = chip->startingProgramAddress;
    //read one byte at a time into memory
    while ( fread( &chip->memory[programAddress++], 1, 1, inputFile ) );
    printf( "Program read in: %d bytes, program starts at %x\n",
               programAddress - chip->startingProgramAddress,
               chip->startingProgramAddress );
}

void ch8_clearMemory( struct Chip8 *chip ) {
    memset( chip->memory, 0, BYTES_MEMORY );
}

void ch8_clearProgramMemory( struct Chip8 *chip ) {
    for ( int i = chip->startingProgramAddress; i < BYTES_MEMORY; ++i ) {
        chip->memory[i] = 0;
    }
}

void ch8_clearScreen( struct Chip8 *chip ) {
    for ( int i = 0; i < DISPLAY_WIDTH; ++i ) {
        for ( int j = 0; j < DISPLAY_HEIGHT; ++j ) {
            chip->display[i][j] = 0x00;
        }
    }
}

void ch8_displaySprite( struct Chip8 *chip ) {
    uint8_t xPos = chip->registers[chip->optionX] % DISPLAY_WIDTH;
    uint8_t yPos = chip->registers[chip->optionY] % DISPLAY_HEIGHT;
    chip->registers[0xF] = 0;
    for ( int i = 0; i < chip->optionN && yPos + i < DISPLAY_HEIGHT; ++i ) {
        assert( chip->indexRegister + i < BYTES_MEMORY );
        uint8_t spriteByte = chip->memory[chip->indexRegister + i];
        for ( int j = 0; j < 8 && j + xPos < DISPLAY_WIDTH; ++j ) {
            uint8_t spritePixel = ( spriteByte >> ( 7 - j ) ) & 1;
            if ( spritePixel ) {
                if ( chip->display[xPos + j][yPos + i] ) {
                    chip->registers[0xF] = 1;
                }
                chip->display[xPos + j][yPos + i] = !chip->display[xPos + j][yPos + i];
            }
        }
    }
}

void ch8_fetchNextInstruction( struct Chip8 *chip ) {
    chip->currentInstruction = chip->memory[chip->programCounter] << 8 |
                               chip->memory[chip->programCounter + 1];

    chip->firstNibble = ( chip->currentInstruction & 0xF000 ) >> 12;
    chip->optionX = ( chip->currentInstruction & 0x0F00 ) >> 8;
    chip->optionY = ( chip->currentInstruction & 0x00F0 ) >> 4;
    chip->optionN = ( chip->currentInstruction & 0x000F );
    chip->optionNN = ( chip->currentInstruction & 0x00FF );
    chip->optionNNN = ( chip->currentInstruction & 0x0FFF );
    chip->programCounter += 2;
}

void ch8_decodeAndExecuteCurrentInstruction( struct Chip8 *chip ) {
    switch ( chip->firstNibble ) {
        case 0x0:
            if ( chip->currentInstruction == 0x00E0 ) {
                //clear screen
                log( "Clearing screen\n" );
                ch8_clearScreen( chip );
            }
            break;
        case 0x1:
            //jump to address
            log( "Jumping from %x to %x\n", chip->programCounter - 2, 
                                            chip->optionNNN );
            chip->programCounter = chip->optionNNN;
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
            log( "Setting register %x to %x\n", chip->optionX, chip->optionNN );
            chip->registers[chip->optionX] = chip->optionNN;
            break;
        case 0x7:
            //add to register
            log( "Adding %x to register %x\n", chip->optionNN, chip->optionX );
            chip->registers[chip->optionX] += chip->optionNN;
            break;
        case 0x8:
            break;
        case 0x9:
            break;
        case 0xA:
            //set index register
            log( "Setting index register to %x\n", chip->optionNN );
            chip->indexRegister = chip->optionNNN;
            break;
        case 0xB:
            break;
        case 0xC:
            break;
        case 0xD:
            log( "Displaying sprite with X: %x, Y: %x, N: %x\n", 
                    chip->registers[chip->optionX],
                    chip->registers[chip->optionY], chip->optionN );
            ch8_displaySprite( chip );
            break;
        case 0xE:
            break;
        case 0xF:
            break;
    }
}

int main( int argc, char *argv[] ) {

    struct Chip8 *chip = ch8_initialize();
    ch8_loadFileIntoMemory( chip, "roms/IBM_Logo.ch8" );

    //Test program, just drawing 0 at the top left of the screen
    //memory[0x200] = 0x00;
    //memory[0x201] = 0xE0;
    //memory[0x202] = 0xA0;
    //memory[0x203] = 0x55;
    //memory[0x204] = 0xD0;
    //memory[0x205] = 0x05;
    //memory[0x206] = 0x12;
    //memory[0x207] = 0x06;

    //set up all of the SDL things
    if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
        fprintf( stderr, "Could not initialize SDL2\n" );
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow( "CHIP-8", SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED, 680, 
                                           480, 0 );
    if ( !window ) {
        fprintf( stderr, "Could not create window\n" );
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer( window, -1, 
                                                 SDL_RENDERER_ACCELERATED );
    if ( !renderer ) {
        fprintf( stderr, "Could not create renderer\n" );
        return 1;
    }
    SDL_RenderClear( renderer );

    SDL_Surface *windowSurface = SDL_GetWindowSurface( window );
    if ( !windowSurface ) {
        fprintf( stderr, "Could not get surface from window\n" );
        return 1;
    }

    //set up all of the pixel requirements
    int pixelWidth = windowSurface->w / DISPLAY_WIDTH;
    int pixelHeight = windowSurface->h / DISPLAY_HEIGHT;
    int pixelSize = pixelWidth < pixelHeight ? pixelWidth : pixelHeight;
    int displayWidth = pixelSize * DISPLAY_WIDTH;
    int displayHeight = pixelSize * DISPLAY_HEIGHT;
    int displayXOffset = ( windowSurface->w - displayWidth ) / 2;
    int displayYOffset = ( windowSurface->h - displayHeight ) / 2;

    SDL_Event e;

    while ( 1 ) {
        /*
         * If STEP is 1, the program will hang up here with every step of the
         * chip, until the user presses any button. This also allows for the
         * user to still exit the program while it is running and stepping
         * through.
         */
        int step = STEP;
        while ( step ) {
            if ( SDL_PollEvent( &e ) > 0 ) {
                switch ( e.type ) {
                    case SDL_QUIT:
                        exit( 1 );
                    case SDL_KEYUP:
                        step = 0;
                        break;
                }
            }
        }

        //this is in case STEP is 0, still allowing user to quit
        while ( SDL_PollEvent( &e ) > 0 ) {
            switch ( e.type ) {
                case SDL_QUIT:
                    exit( 1 );
            }
            SDL_UpdateWindowSurface( window );
        }

        //draw the pixels based on display
        SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
        for ( int i = 0; i < DISPLAY_WIDTH; ++i ) {
            for ( int j = 0; j < DISPLAY_HEIGHT; ++j ) {
                if ( chip->display[i][j] ) { 
                    SDL_Rect r = {displayXOffset + pixelSize * i,
                                  displayYOffset + pixelSize * j,
                                  pixelSize, pixelSize};
                    //SDL_RenderFillRect( renderer, &r ); //filled rect
                    SDL_RenderDrawRect( renderer, &r );   //rect outline
                }
            }
        }
        SDL_RenderPresent( renderer );

        //fetch
        ch8_fetchNextInstruction( chip ); 
        //decode
        ch8_decodeAndExecuteCurrentInstruction( chip );
    }
    return 0;
}
