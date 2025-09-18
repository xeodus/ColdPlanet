#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef enum {
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC, 	 // program counter
    R_COND, 	// stores information about the most recently executed programs
    R_COUNT
} Register;

struct CPU {
    Register r;
};

// Storing register in an array
extern uint16_t reg[R_COUNT];

#define MEMORY_MAX (1 << 16)

// Storing memory in an array
// This is LC-3 architecture and has 65,536 memory locations
// it is the maximum that is addressable by a 16-bit unsigned integer
// each memory locations  stores a 16-bit value, that implies it can only store a total of 128KB.

extern uint16_t memory[MEMORY_MAX];

// This is our own Assembly code for VM
typedef enum {
    OP_BR = 0,  // branch
    OP_LD,      // load
    OP_ST,      // store
    OP_JSR,     // jump register
    OP_AND,     // bitwise additiod
    OP_ADD, 	// add
    OP_LDR,     // load register
    OP_STR,     // store register
    OP_RTI,     // unused
    OP_NOT,     // bitwise not
    OP_LDI,     // load indirect
    OP_STI,     // store indirect
    OP_JMP,     // jump
    OP_RES,     // reserved
    OP_LEA,     // load effective address
    OP_TRAP     // execute trap
} OpCodes;

// Conditional flags are used being used for only three
// possibilities negative, zero, and positive.
typedef enum {
    FL_POS = 1 << 0,
    FL_ZRO = 1 << 1,
    FL_NEG = 1 << 2
} ConditionFlags; 

typedef enum {
    MR_KBSR = 0xFE00,   // Keyboard status
    MR_KBDA = 0xFE02    // Keyboard data
} KeyboardMap;

// Trap varients used for different I/O ops
typedef enum {
    TRAP_GETC = 0x20,
    TRAP_OUT = 0x21,
    TRAP_PUTS = 0x22,
    TRAP_IN = 0x23,
    TRAP_PUTSP = 0x24,
    TRAP_HALT = 0x25
} Trap;

#endif
