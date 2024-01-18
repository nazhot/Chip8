#ifndef CH8_H
#define CH8_H
#include <stdint.h> 
#include "screen.h"
#include <stdbool.h>

#define BYTES_MEMORY 4096 //standard is 4096
#define STACK_SIZE 15 //standard is 16
//#define DEBUG  //define to print debug messages
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
    bool keyPressed;
    uint8_t key;
};

/*
 * Set up the defaults for Chip8
 *
 * Non-zero defaults:
 * - startingProgramAddress/programCounter, 0x200
 * - framesPerSecond, 60
 * - instructionsPerSecond, 700
 * This function also initializes a Screen, with a default size of 64x32
 *
 * @return pointer to the intialized Chip8
 */
struct Chip8* ch8_initialize();

/*
 * Load default fonts into Chip8 memory
 *
 * Fonts are stored in an array with the function itself.
 * TODO: update so that font array is passed in so it can be changed
 *
 * @param chip            Chip8 to add fonts to
 * @param startingAddress address that the first font will start at
 */
void ch8_initializeFonts( struct Chip8 *chip, const uint16_t startingAddress );

/*
 * Read a binary file into the memory of a Chip8 to use as a program
 *
 * The program will start being written at startingProgramAddress
 * TODO: add in protection against overflow
 *
 * @param chip     Chip8 to add program to
 * @param filePath path to the file that holds the program
 */
void ch8_loadFileIntoMemory( struct Chip8 *chip, const char filePath[] );

/*
 * Set ALL of the memory within the Chip8 to 0x0
 *
 * @param chip Chip8 to clear the memory of
 */
void ch8_clearMemory( struct Chip8 *chip );

/*
 * Set only the memory where programs are stored of Chip8 to 0x0
 *
 * @param chip Chip8 to clear the memory of
 */
void ch8_clearProgramMemory( struct Chip8 *chip );

/*
 * Reset all of the elements in display to 0x0
 *
 * This will also call the needed SDL function to update the window.
 * TODO: determine if this should be static
 *
 * @param chip Chip8 to clear the screen of
 */
void ch8_clearScreen( struct Chip8 *chip );

void ch8_dumpMemory( struct Chip8 *chip );
/*
 * Display a sprite to the Screen
 *
 * Only other way to interact with the window other than just clearing it. The
 * indexRegister holds the first address of the sprite data, optionX refers to
 * the register with the X position and optionY refers to the register with the 
 * Y position (X,Y of the top left corner of the sprite). Each bit of the 8-bit
 * is used to manipulate a pixel of the display: a 0 means leave the pixel
 * alone, a 1 means to flip it (if it was being shown, hide it, if it was
 * hidden, show it). OptionN contains how many rows should be drawn, each row
 * is incremented from indexRegister (indexRegister itself is not incremented).
 * 
 * @param chip Chip8 to display a sprite on the Screen of
 */
void ch8_displaySprite( struct Chip8 *chip );

/*
 * Update the SDL Window on the user's computer screen
 *
 * Uses the display data to set all of the pixels. This will also check to make
 * sure that the frame rate is being adhered to. Currently the delay/sound
 * timers are tied to this since they all are locked to 60 per second.
 * TODO: remove the tie to delay/sound timers
 *
 * @param chip Chip8 to draw the Screen of
 */
void ch8_drawScreen( struct Chip8 *chip );

/*
 * Draw the pixels to the rendering buffer
 *
 * This will just draw the needed rectangles to the buffer in SDL, this will not
 * actually draw the Screen to the computer screen.
 * TODO: determine if this should be static
 *
 * @param chip Chip8 to update the Screen of
 */
void ch8_updateScreen( struct Chip8 *chip );

/*
 * Pull the next instruction from memory of the Chip8
 *
 * An instruction is made up of 2 consecutive 8-bit memory values, combined into
 * one 16-bit instruction.
 *
 * @param chip Chip8 to pull the instruction from
 */
void ch8_fetchNextInstruction( struct Chip8 *chip );

/*
 * Decode the instruction, and execute it
 *
 * With every instruction fetched, the chip will also set up all of the options
 * for that 16-bit value, even if they aren't used.
 *
 * @param chip Chip8 to decode/execute the instruction from
 */
void ch8_decodeAndExecuteCurrentInstruction( struct Chip8 *chip );
#endif
