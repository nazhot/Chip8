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
    bool keyBlocked;
    bool instBlocked;
    struct Screen *screen;
    uint8_t memory[BYTES_MEMORY];
    int8_t display[DISPLAY_WIDTH][DISPLAY_HEIGHT];
    uint8_t registers[16];
    uint16_t indexRegister;
    uint16_t programCounter;
    uint16_t stack[STACK_SIZE];
    uint16_t stackAddress;
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
    float lastDrawTime;
    uint32_t framesPerSecond; 
    float secondsPerFrame;
    float lastInstructionTime;
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
