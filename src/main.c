#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "ch8.h"

int main( int argc, char *argv[] ) {

    srand( ( unsigned ) time(NULL) );

    struct Chip8 *chip = ch8_initialize();
    ch8_initializeFonts( chip, 0x50 );
    ch8_loadFileIntoMemory( chip, "roms/test_opcode.ch8" );

    //Test program, just drawing 0 at the top left of the screen
    //memory[0x200] = 0x00;
    //memory[0x201] = 0xE0;
    //memory[0x202] = 0xA0;
    //memory[0x203] = 0x55;
    //memory[0x204] = 0xD0;
    //memory[0x205] = 0x05;
    //memory[0x206] = 0x12;
    //memory[0x207] = 0x06;

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
            //ch8_drawScreen( chip );
        }

        ch8_updateScreen( chip );
        ch8_drawScreen( chip );
        //fetch
        ch8_fetchNextInstruction( chip ); 
        //decode
        ch8_decodeAndExecuteCurrentInstruction( chip );
    }
    free( chip );
    return 0;
}
