#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



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


    uint8_t memory[4096] = {0};
    uint8_t registers[16] = {0};
    uint16_t indexRegister = 0;
    uint16_t programCounter = 0;
    uint8_t display[8][4] = {0};
    uint16_t stack[16] = {0};
    uint8_t delayTimer = 0;
    uint8_t soundTimer = 0;

    {
        int startingMemoryIndex = 0x050;
        for ( int i = 0; i < 16; ++i ) {
            for ( int j = 0; j < 5; ++j ) {
                memory[startingMemoryIndex++] = fonts[i][j];
            }
        }
    }

    while ( 1 ) {
        //fetch
        uint16_t instruction = memory[programCounter] << 8 | memory[programCounter + 1];
        programCounter += 2;
        //decode
        uint16_t firstNibble = instruction & 0xF000;
        switch ( firstNibble ) {
            case 0x0:
                break;
            case 0x1:
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
                break;
            case 0x7:
                break;
            case 0x8:
                break;
            case 0x9:
                break;
            case 0xA:
                break;
            case 0xB:
                break;
            case 0xC:
                break;
            case 0xD:
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
