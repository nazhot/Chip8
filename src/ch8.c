#include "ch8.h"
#include <time.h>
#include <assert.h>


struct Chip8* ch8_initialize() {
    struct Chip8 *chip = malloc( sizeof( struct Chip8 ) );
    memset( chip, 0, sizeof( struct Chip8 ) );
    chip->startingProgramAddress = 0x200;
    chip->programCounter = 0x200;
    chip->framesPerSecond = 60;
    chip->secondsPerFrame = 1.0 / chip->framesPerSecond;
    chip->instructionsPerSecond = 700;
    chip->secondsPerInstruction = 1.0 / chip->instructionsPerSecond;
    chip->screen = screen_initialize(680, 480);
    return chip;
}

void ch8_initializeFonts( struct Chip8 *chip, const uint16_t startingAddress ) {
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

void ch8_dumpMemory( struct Chip8 *chip ) {
    uint16_t lastProgramCounter = chip->programCounter;
    float lastSecondsPerInstruction = chip->secondsPerInstruction;
    chip->secondsPerInstruction = 0;
    chip->programCounter = chip->startingProgramAddress;
    printf( "------Memory Dump------\n" );
    while ( chip->programCounter < BYTES_MEMORY ) {
        ch8_fetchNextInstruction( chip );
        if ( !chip->currentInstruction ) {
            continue;
        }
        printf( "--%x--\n", chip->programCounter - 2 );
        printf( "First nibble: %x ", chip->firstNibble );
        switch ( chip->firstNibble ) {
            case 0x0:
                if ( chip->currentInstruction == 0x00E0 ) {
                    //clear screen
                    printf( "(Clear Screen)\n" );
                } else if ( chip->currentInstruction == 0x00EE ) {
                    printf( "(Return to last stack address)\n" );
                    //commented out because techinically nothing will be in the stack
                    //printf( "\tStack Index: %u\n", chip->stackAddress - 1 );
                    //printf( "\tStack Address: %x\n", chip->stack[chip->stackAddress - 1] );
                }
                break;
            case 0x1:
                //jump to address
                printf( "(Jump to address)\n" );
                printf( "\tNNN (address): %x\n", chip->optionNNN );
                break;
            case 0x2:
                printf( "(Call Subroutine)\n");
                printf( "\tSubroutine Address: %x\n", chip->optionNNN );
                break;
            case 0x3:
                printf( "(Skip Next Instruction [Register Value == NN])\n" );
                printf( "\tRegister: %x\n", chip->optionX );
                printf( "\tCompared Against: %x\n", chip->optionNN );
                break;
            case 0x4:
                printf( "(Skip Next Instruction [Register Value != NN])\n" );
                printf( "\tRegister: %x\n", chip->optionX );
                printf( "\tCompared Against: %x\n", chip->optionNN );
                break;
            case 0x5:
                printf( "(Skip Next Instruction [Register Value == Register Value])\n" );
                printf( "\tRegister 1: %x\n", chip->optionX );
                printf( "\tRegister 2: %x\n", chip->optionY );
                break;
            case 0x6:
                //set register
                printf( "(Set Register)\n" );
                printf( "\tRegister: %x\n", chip->optionX );
                printf( "\tValue: %x\n", chip->optionNN );
                break;
            case 0x7:
                //add to register
                printf( "(Add to Register)\n" );
                printf( "\tRegister: %x\n", chip->optionX );
                printf( "\tValue: %x\n", chip->optionNN );
                break;
            case 0x8:
                printf( "(Register Operation)\n" );
                printf( "N: %x ", chip->optionN );
                switch ( chip->optionN ) {
                    case 0x0:
                        printf( "(Set)\n" );
                        printf( "\tTarget Register: %x\n", chip->optionX );
                        printf( "\tFrom Register: %x\n", chip->optionY );
                        break;
                    case 0x1:
                        printf( "(OR)\n" );
                        printf( "\tRegister 1 (Set): %x\n", chip->optionX );
                        printf( "\tRegister 2: %x\n", chip->optionY );
                        break;
                    case 0x2:
                        printf( "(AND)\n" );
                        printf( "\tRegister 1 (Set): %x\n", chip->optionX );
                        printf( "\tRegister 2: %x\n", chip->optionY );
                        break;
                    case 0x3:
                        printf( "(XOR)\n" );
                        printf( "\tRegister 1 (Set): %x\n", chip->optionX );
                        printf( "\tRegister 2: %x\n", chip->optionY );
                        break;
                    case 0x4:
                        //check for overflow, but still allow it to go through
                        printf( "(ADD [Overflow Allowed])\n" );
                        printf( "\tRegister 1 (Set): %x\n", chip->optionX );
                        printf( "\tRegister 2: %x\n", chip->optionY );
                        break;
                    case 0x5:
                        //check for underflow, but still allow it to go through
                        printf( "(SUBTRACT [Underflow Allowed])\n" );
                        printf( "\tRegister 1 (Set): %x\n", chip->optionX );
                        printf( "\tRegister 2: %x\n", chip->optionY );
                        break;
                    case 0x6:
                        //chip->registers[chip->optionX] = chip->registers[chip->optionY];
                        printf( "(SHIFT RIGHT)\n" );
                        printf( "\tRegister: %x\n", chip->optionX );
                        break;
                    case 0x7:
                        //check for underflow, but still allow it to go through
                        printf( "(SUBTRACT [Underflow Allowed])\n" );
                        printf( "\tRegister 1: %x\n", chip->optionY );
                        printf( "\tRegister 2 (Set): %x\n", chip->optionX );
                        break;
                    case 0xE:
                        //chip->registers[chip->optionX] = chip->registers[chip->optionY];
                        printf( "(SHIFT LEFT)\n" );
                        printf( "\tRegister: %x\n", chip->optionX );
                        break;
                }
                break;
            case 0x9:
                printf( "(Skip Next Instruction [Register Value != Register Value])\n" );
                printf( "\tRegister 1: %x\n", chip->optionX );
                printf( "\tRegister 2: %x\n", chip->optionY );
                break;
            case 0xA:
                //set index register
                printf( "(Set Index Register)\n" );
                printf( "\tValue: %x\n", chip->optionNN );
                break;
            case 0xB:
                //jump + constant
                printf( "(Jump [With Constant])\n" );
                printf( "\tAddress: %x\n", chip->optionNNN );
                break;
            case 0xC:
                //random number generator
                printf( "(Random Number)\n" );
                printf( "\tRegister (Set): %x\n", chip->optionX );
                printf( "\tValue (AND-ed): %x\n", chip->optionNN );
                break;
            case 0xD:
                printf( "(Display Sprite)\n" );
                printf( "\tRegister with xPos: %x\n", chip->optionX );
                printf( "\tRegister with yPos: %x\n", chip->optionY );
                printf( "\tRows: %u\n", chip->optionN );
                break;
            case 0xE:
                switch ( chip->optionY ) {
                    case 0x9:
                        //skip if key in VX is pressed
                        break;
                    case 0xA:
                        //skip if key in VX is not pressed
                        break;
                }
                break;
            case 0xF:
                switch ( chip->optionNN ) {
                    case 0x07:
                        printf( "(Set Register to Delay Timer)\n" );
                        printf( "\tRegister: %x\n", chip->optionX );
                        break;
                    case 0x15:
                        printf( "(Set Delay Timer to Register)\n" );
                        printf( "\tRegister: %x\n", chip->optionX );
                        break;
                    case 0x18:
                        printf( "(Set Delay Timer to Register)\n" );
                        printf( "\tRegister: %x\n", chip->optionX );
                        break;
                    case 0x1E:
                        printf( "(Increment Register)\n" );
                        printf( "\tIncrement Value: %x\n", chip->optionX );
                        break;
                    case 0x0A:
                        printf( "(Block Until Key Pressed)\n" );
                        break;
                    case 0x29:
                        printf( "(Set Index Register To Font Address)\n" );
                        printf( "\tRegister: %x\n", chip->optionX );
                        break;
                    case 0x33:
                        printf( "(Set Addresses at Index Register to Digits at Register )\n" );
                        printf( "\tRegister: %x\n", chip->optionX );
                        break;
                    case 0x55:
                        printf( "(Set Addresses at Index Register to Registers)\n" );
                        printf( "\tUp to and Including Register: %x\n", chip->optionX );
                        break;
                    case 0x65:
                        printf( "(Set Registers to Memory at Index Register)\n" );
                        printf( "\tUp to and Including Register: %x\n", chip->optionX );
                        break;
                }
                break;
        }
    }
    chip->programCounter = lastProgramCounter;
    chip->secondsPerInstruction = lastSecondsPerInstruction;
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

void ch8_drawScreen( struct Chip8 *chip ) {
    float currentTime = clock() * 1.0 / CLOCKS_PER_SEC;
    if ( currentTime - chip->lastDrawTime >= chip->secondsPerFrame ) {
        chip->lastDrawTime = currentTime;
        SDL_RenderPresent( chip->screen->renderer );
        if ( chip->delayTimer > 0 ) { //TODO: move to their own function(s)
            chip->delayTimer--;
        }
        if ( chip->soundTimer > 0 ) {
            fprintf( stdout, "\a" );
            chip->soundTimer--;
        }
    }
}

void ch8_updateScreen( struct Chip8 *chip ) {
    SDL_SetRenderDrawColor( chip->screen->renderer, 255, 0, 0, 255 );
    for ( int i = 0; i < DISPLAY_WIDTH; ++i ) {
        for ( int j = 0; j < DISPLAY_HEIGHT; ++j ) {
            if ( chip->display[i][j] ) { 
                SDL_Rect r = {chip->screen->xOffset + chip->screen->pixelSize * i,
                              chip->screen->yOffset + chip->screen->pixelSize * j,
                              chip->screen->pixelSize, 
                              chip->screen->pixelSize};
                //SDL_RenderFillRect( renderer, &r ); //filled rect
                SDL_RenderDrawRect( chip->screen->renderer, &r );   //rect outline
            }
        }
    }
}

void ch8_fetchNextInstruction( struct Chip8 *chip ) {
    if ( chip->keyBlocked ) {
        return;
    }
    float currentTime = clock() * 1.0 / CLOCKS_PER_SEC;
    if ( currentTime - chip->lastInstructionTime >= chip->secondsPerInstruction ) {
        chip->lastInstructionTime = currentTime;
        chip->instBlocked = false;
    } else  {
        return;
    }
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
    if ( chip->keyBlocked ) {
        return;
    }

    if ( chip->instBlocked ) {
        return;
    }

    chip->instBlocked = true;

    switch ( chip->firstNibble ) {
        case 0x0:
            if ( chip->currentInstruction == 0x00E0 ) {
                //clear screen
                log( "Clearing screen\n" );
                ch8_clearScreen( chip );
            } else if ( chip->currentInstruction == 0x00EE ) {
                assert( chip->stackAddress > 0 );
                log( "Returning to last stack address: %x\n", chip->stack[chip->stackAddress - 1] );
                chip->programCounter = chip->stack[--chip->stackAddress];
            }
            break;
        case 0x1:
            //jump to address
            log( "Jumping from %x to %x\n", chip->programCounter - 2, 
                                            chip->optionNNN );
            chip->programCounter = chip->optionNNN;
            break;
        case 0x2:
            assert( chip->stackAddress < STACK_SIZE ); 
            log( "Pushing %x to stack, jumping to  %x\n", chip->programCounter, chip->optionNNN );
            chip->stack[chip->stackAddress++] = chip->programCounter;
            chip->programCounter = chip->optionNNN;
            break;
        case 0x3:
            if ( chip->registers[chip->optionX] == chip->optionNN ) {
                chip->programCounter += 2;
            }
            break;
        case 0x4:
            if ( chip->registers[chip->optionX] != chip->optionNN ) {
                chip->programCounter += 2;
            }
            break;
        case 0x5:
            if ( chip->registers[chip->optionX] == chip->registers[chip->optionY] ) {
                chip->programCounter += 2;
            }
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
            switch ( chip->optionN ) {
                case 0x0:
                    chip->registers[chip->optionX] = chip->registers[chip->optionY];
                    break;
                case 0x1:
                    chip->registers[chip->optionX] = chip->registers[chip->optionX] |
                                                     chip->registers[chip->optionY];
                    break;
                case 0x2:
                    chip->registers[chip->optionX] = chip->registers[chip->optionX] &
                                                     chip->registers[chip->optionY];
                    break;
                case 0x3:
                    chip->registers[chip->optionX] = chip->registers[chip->optionX] ^
                                                     chip->registers[chip->optionY];
                    break;
                case 0x4:
                    //check for overflow, but still allow it to go through
                    chip->registers[0xF] = 255 - chip->registers[chip->optionX] 
                                           < chip->registers[chip->optionY];
                    chip->registers[chip->optionX] = chip->registers[chip->optionX] +
                                                     chip->registers[chip->optionY];
                    break;
                case 0x5:
                    //check for underflow, but still allow it to go through
                    chip->registers[0xF] = chip->registers[chip->optionX] >
                                           chip->registers[chip->optionY];
                    chip->registers[chip->optionX] = chip->registers[chip->optionX] -
                                                     chip->registers[chip->optionY];
                    break;
                case 0x6:
                    //chip->registers[chip->optionX] = chip->registers[chip->optionY];
                    chip->registers[0xF] = chip->registers[chip->optionX] & 1;
                    chip->registers[chip->optionX] >>= 1;
                    break;
                case 0x7:
                    //check for underflow, but still allow it to go through
                    chip->registers[0xF] = chip->registers[chip->optionY] >
                                           chip->registers[chip->optionX];
                    chip->registers[chip->optionX] = chip->registers[chip->optionY] -
                                                     chip->registers[chip->optionX];
                    break;
                case 0xE:
                    //chip->registers[chip->optionX] = chip->registers[chip->optionY];
                    chip->registers[0xF] = chip->registers[chip->optionX] & 0x80;
                    chip->registers[chip->optionX] <<= 1;
                    break;
            }
            break;
        case 0x9:
            if ( chip->registers[chip->optionX] != chip->registers[chip->optionY] ) {
                chip->programCounter += 2;
            }
            break;
        case 0xA:
            //set index register
            log( "Setting index register to %x\n", chip->optionNN );
            chip->indexRegister = chip->optionNNN;
            break;
        case 0xB:
            //jump + constant
            chip->programCounter = chip->optionNNN + chip->registers[0x0];
            break;
        case 0xC:
            //random number generator
            chip->registers[chip->optionX] = rand() & chip->optionNN;
            break;
        case 0xD:
            log( "Displaying sprite with X: %x, Y: %x, N: %x\n", 
                    chip->registers[chip->optionX],
                    chip->registers[chip->optionY], chip->optionN );
            ch8_displaySprite( chip );
            break;
        case 0xE:
            switch ( chip->optionY ) {
                case 0x9:
                    //skip if key in VX is pressed
                    break;
                case 0xA:
                    //skip if key in VX is not pressed
                    break;
            }
            break;
        case 0xF:
            switch ( chip->optionNN ) {
                case 0x07:
                    chip->registers[chip->optionX] = chip->delayTimer;
                    break;
                case 0x15:
                    chip->delayTimer = chip->registers[chip->optionX];
                    break;
                case 0x18:
                    chip->soundTimer = chip->registers[chip->optionX];
                    break;
                case 0x1E:
                    chip->indexRegister += chip->registers[chip->optionX];
                    chip->registers[0xF] = chip->indexRegister > 0x1000;
                    break;
                case 0x0A:
                    chip->keyBlocked = 1;
                    chip->programCounter -= 2;
                    break;
                case 0x29:
                    chip->indexRegister = chip->startingFontAddress + ( chip->registers[chip->optionX] & 0x0F ) * 5;
                    break;
                case 0x33:
                    chip->memory[chip->indexRegister] = chip->registers[chip->optionX] / 100;
                    chip->memory[chip->indexRegister + 1] = chip->registers[chip->optionX] / 10 % 10;
                    chip->memory[chip->indexRegister + 2] = chip->registers[chip->optionX] % 10;
                    break;
                case 0x55:
                    for ( int i = 0; i <= chip->optionX; ++i ) {
                        chip->memory[chip->indexRegister + i] = chip->registers[i]; 
                    }
                    break;
                case 0x65:
                    for ( int i = 0; i <= chip->optionX; ++i ) {
                        chip->registers[i] = chip->memory[chip->indexRegister + i];
                    }
                    break;
            }
            break;
    }
}
