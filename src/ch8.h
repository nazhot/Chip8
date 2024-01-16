#ifndef CH8_H
#define CH8_H
#include <stdint.h> 
#include "screen.h"
#include <stdbool.h>

#define BYTES_MEMORY 4096 //standard is 4096
#define STACK_SIZE 15 //standard is 16
#define DEBUG  //define to print debug messages
#define STEP 0 //whether to wait for user input to step through instructions

#ifdef DEBUG
#define log(...) printf(__VA_ARGS__) //macro for logging
#else
#define log(...) //if not debugging, don't printf
#endif

struct Chip8 {
    bool keyBlocked; //if the chip should prevent instructions running because it 
                     //is waiting on a key
    bool instBlocked; //if the chip should prevent instructions running while it
                      //waits for enough time to pass to stay at 
                      //instructionsPerSecond
    struct Screen *screen; //pointer to SDL components that will display the
                           //pixels
    uint8_t memory[BYTES_MEMORY]; //core memory of the chip
    bool display[DISPLAY_WIDTH][DISPLAY_HEIGHT]; //pixel data, each element is
                                                 //whether a pixel is on or off.
                                                 //originally wanted to have only
                                                 //one bit per pixel, but was
                                                 //too much work when trying to
                                                 //just get it working
    uint8_t registers[16]; //the 16 general 8-bit registers of the chip
    uint16_t indexRegister; //register that stores an address
    uint16_t programCounter; //address the program is executing
    uint16_t stack[STACK_SIZE]; //used to hold addresses to return to after a
                                //function returns. Addresses are the next
                                //address in order after the opcode for
                                //pushing to the stack is called
    uint16_t stackAddress; //index of the stack + 1
    uint8_t delayTimer; //decremented 60 times per second, used by programs
                        //for delay, not used by chip
    uint8_t soundTimer; //decremented 60 times per second, emits a beep when
                        //greater than 0
    uint16_t startingFontAddress; //first address of where fonts are stored
    uint16_t startingProgramAddress; //first address of where programs start
    uint16_t currentInstruction; //opcode, 2 8-bit memory values in a row
    uint8_t firstNibble; //first 4 bits of current instruction
    uint8_t optionX; //second 4 bits of current instruction
    uint8_t optionY; //third 4 bits of current instruction
    uint8_t optionN; //fourth 4 bits of current instruction
    uint8_t optionNN; //second 8 bits of current instruction (Y + N)
    uint16_t optionNNN; //final 12 bits of current instruction
    float lastDrawTime; //time in seconds of last draw to screen
    uint32_t framesPerSecond;
    float secondsPerFrame;
    float lastInstructionTime; //time in seconds of last instruction execution
    uint32_t instructionsPerSecond;
    float secondsPerInstruction;
};

struct Chip8* ch8_initialize();
void ch8_initializeFonts( struct Chip8 *chip, const uint16_t startingAddress );
void ch8_loadFileIntoMemory( struct Chip8 *chip, const char filePath[] );
void ch8_clearMemory( struct Chip8 *chip );
void ch8_clearProgramMemory( struct Chip8 *chip );
void ch8_clearScreen( struct Chip8 *chip );
void ch8_displaySprite( struct Chip8 *chip );
void ch8_drawScreen( struct Chip8 *chip );
void ch8_updateScreen( struct Chip8 *chip );
void ch8_fetchNextInstruction( struct Chip8 *chip );
void ch8_decodeAndExecuteCurrentInstruction( struct Chip8 *chip );





#endif
