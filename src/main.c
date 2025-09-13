#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "utils.h"

int main(int argc, const char *argv[]) {
    // Arguments
    if (argc < 2) {
	printf("lc3 [image-file]\n");
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
    restore_input_buffering();

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
		// first opperand
		uint16_t r1 = (instr >> 6) & 0x7;
		// checking if we're in a immediate mode
		uint16_t imm_flag = (instr >> 5) & 0x1;

		if (imm_flag) {
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
		    reg[0] = reg[r1] & imm5;
		}
		else {
		    uint16_t r2 = instr & 0x7;
		    reg[r0] = reg[r1] & reg[r2];
		    update_flags(r0);
		    break;
		}
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
		reg[R_PC] = r1;
		break;
	    }
	    case OP_JPR: {
		uint16_t long_flag = (instr >> 11) & 1;
		reg[R_R7] = reg[R_PC];

		if (long_flag) {
		    uint16_t long_pc_offset = sign_extend((instr & 0x7FF) , 11);
		    reg[R_PC] += long_pc_offset;
		}
		else {
		    uint16_t r1 = (instr >> 6) & 0x7;
		    reg[R_PC] += reg[r1];
		    break;
		}
	    }
	    case OP_LD: {
		uint16_t r0 = (instr >> 9) & 0x7;
		uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
		reg[r0] = mem_read(pc_offset + reg[R_PC]);
		update_flags(r0);
		break;
	    }
	    case OP_LDI: {
		uint16_t r0 = (instr >> 9) & 0x7;
		uint16_t pc_offset = sign_extend((instr & 0x1FF), 9);
		reg[r0] = mem_read(pc_offset + reg[R_PC]);
		update_flags(r0);
		break;
	    }
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
		break;
	    }
	    case OP_RES: { abort(); }
	    case OP_RTI: { abort(); }
	}
    }
}
