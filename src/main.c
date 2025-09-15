#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "utils.h"

uint16_t reg[R_COUNT];
uint16_t memory[MEMORY_MAX];

int main(int argc, const char* argv[]) {
    // Arguments
    if (argc < 2) {
	printf("lc3 [image-file] ...\n");
	exit(2);
    }

    for (int i = 1; i < argc; i++) {
	if (!read_image(argv[i])) {
	    printf("failed to load the image: %s\n", argv[i]);
	    exit(1);
	}
    }

    // Setup
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    // First we will load data from memory to CPU register's addresses
    // A flag must be attached to each instruction executed
    reg[R_COND] = FL_ZRO;
    // default address for starting the PC
    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;
    int running = 1;

    while (running) {
	// after loading the data onto the register, we must increment the register
	uint64_t instr = mem_read(reg[R_PC]++); 
	uint64_t op = instr >> 12;

	switch (op) {
	    case OP_ADD: {
		// destination register
		uint16_t r0 = (instr >> 9) & 0x7;
		// first opperand, first source register
		uint16_t r1 = (instr >> 6) & 0x7;
		// defining the immediate flag
		uint16_t imm_flag = (instr >> 5) & 0x1;

		if (imm_flag) {
		    // second source register, here we have 5-bits signed values
		    // now in order to make them compatible with other 16-bit values
		    // we will have to use sign_extend function.
		    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
		    reg[r0] = reg[r1] + imm5;
		}
		else {
		    uint16_t r2 = instr & 0x7;
		    reg[r0] = reg[r1] + reg[r2];
		}
		update_flags(r0);
		break;
	    }
	    case OP_AND: {
		uint16_t r0 = (instr >> 9) & 0x7;
		uint16_t r1 = (instr >> 6) & 0x7;
		uint16_t imm_flag = (instr >> 5) & 0x1;

		if (imm_flag) {
		    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
		    reg[r0] = reg[r1] & imm5;
		}
		else {
		    uint16_t r2 = instr & 0x7;
		    reg[r0] = reg[r1] & reg[r2];
		    update_flags(r0);
		}
		break;
	    }
	    case OP_BR: {
		uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
		uint16_t cond_flag = (instr >> 9) & 0x7;
		if (cond_flag & reg[R_COND]) {
		    reg[R_PC] += pc_offset;
		}
		break;
	    }
	    case OP_NOT: {
		uint16_t r0 = (instr >> 9) & 0x7;
		uint16_t r1 = (instr >> 6) & 0x7;
		reg[r0] = ~reg[r1];
		update_flags(r0);
		break;
	    }
	    case OP_JMP: {
		uint16_t r1 = (instr >> 6) & 0x7;
		reg[R_PC] = reg[r1];
		break;
	    }
	    case OP_JSR: {
		uint16_t long_flag = (instr >> 11) & 1;
		reg[R_R7] = reg[R_PC];

		if (long_flag) {
		    uint16_t long_pc_offset = sign_extend((instr & 0x7FF) , 11);
		    reg[R_PC] += long_pc_offset;
		    break;
		}
		else {
		    uint16_t r1 = (instr >> 6) & 0x7;
		    reg[R_PC] = reg[r1];
		}
	    }
	    case OP_LD: {
		uint16_t r0 = (instr >> 9) & 0x7;
		uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
		reg[r0] = mem_read(pc_offset + reg[R_PC]);
		update_flags(r0);
		break;
	    }
	    /*case OP_LDI: {
		uint16_t r0 = (instr >> 9) & 0x7;
		uint16_t pc_offset = sign_extend((instr & 0x1FF), 9);
		reg[r0] = mem_read(pc_offset + reg[R_PC]);
		update_flags(r0);
		break;
	    }*/
	    case OP_LDR: {
		uint16_t r0 = (instr >> 9) & 0x7;
		uint16_t r1 = (instr >> 6) & 0x7;
		uint16_t offset = sign_extend(instr & 0x3F, 6);
		reg[r0] = mem_read(offset + reg[r1]);
		update_flags(r0);
		break;
	    }
	    case OP_LEA: {
		uint16_t r0 = (instr >> 9) & 0x7;
		uint16_t pc_offset = sign_extend((instr & 0x1FF), 9);
		reg[r0] = reg[R_PC] + pc_offset;
		update_flags(r0);
		break;
	    }
	    case OP_ST: {
		uint16_t r0 = (instr >> 9) & 0x7;
		uint16_t pc_offset = sign_extend((instr & 0x1FF), 9);
		mem_write((reg[R_PC] + pc_offset), reg[r0]);
		break;
	    }
	    case OP_STI: {
		uint16_t r0 = (instr >> 9) & 0x7;
		uint16_t pc_offset = sign_extend((instr & 0x1FF), 9);
		mem_write((reg[R_PC] + pc_offset), reg[r0]);
		break;
	    }
	    case OP_STR: {
		uint16_t r0 = (instr >> 9) & 0x7;
		uint16_t r1 = (instr >> 6) & 0x7;
		uint16_t offset = sign_extend((instr & 0x3F), 6);
		mem_write((reg[r1] + offset), reg[r0]);
		break;
	    }
	    case OP_TRAP: {
		reg[R_R7] = reg[R_PC];
		switch (instr & 0xFF) {
		    case TRAP_GETC: {
			reg[R_R0] = (uint16_t) getchar();
			update_flags(R_R0);
			break;
		    }
		    case TRAP_PUTS: {
			putc((char) memory[reg[R_R0]], stdout);
			fflush(stdout);
			break;
		    }
		    case TRAP_PUTSP: {
			uint16_t* c = memory + reg[R_R0];
			while (*c) {
			    char c1 = (*c) & 0xFF;
			    putc(c1, stdout);
			    char c2 = (*c) >> 8;
			    if (c2) putc(c2, stdout);
			    c++;
			}
			fflush(stdout);
			break;
		    }
		    case TRAP_IN: {
			printf("Enter a character: \n");
			char c = getchar();
			putc(c, stdout);
			fflush(stdout);
			reg[R_R0] = (uint16_t) c;
			update_flags(R_R0);
			break;
		    }
		    case TRAP_OUT: {
			putc((char)reg[R_R0], stdout);
			fflush(stdout);
			break;
		    }
		    case TRAP_HALT: {
			puts("HALT");
			fflush(stdout);
			running = 0;
			break;
		    }
		}
		break;
	    }
	    case OP_RES: { abort(); }
	    case OP_RTI: { abort(); }
	    default: {
		abort();
		break;
	    }
	}
    }
    restore_input_buffering();
}
