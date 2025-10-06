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
        printf("lc3 [2048.obj] ...\n");
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

    // Initialization of condition flags and PC
    reg[R_COND] = FL_ZRO;
    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;
	
	printf("PC started at: %#04x\n", reg[R_PC]);
	printf("Instructions at: %04x\n", memory[0x3000]);

    int running = 1;

    while (running) {
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        switch (op) {
            case OP_ADD: {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
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
                    reg[r0] = reg[r1] & imm5;
                }
                else {
                    uint16_t r2 = instr & 0x7;
                    reg[r0] = reg[r1] & reg[r2];
                }
                update_flags(r0);
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
                    uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
                    reg[R_PC] += long_pc_offset;
                }
                else {
                    uint16_t r1 = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[r1];
                }
                break;
            }
            case OP_LD: {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                reg[r0] = mem_read(reg[R_PC] + pc_offset);
                update_flags(r0);
                break;
            }
            case OP_LDI: {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                uint16_t addr = mem_read(reg[R_PC] + pc_offset);
                reg[r0] = mem_read(addr);
                update_flags(r0);
                break;
            }
            case OP_LDR: {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset = sign_extend(instr & 0x3F, 6);
                reg[r0] = mem_read(reg[r1] + offset);
                update_flags(r0);
                break;
            }
            case OP_LEA: {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                reg[r0] = reg[R_PC] + pc_offset;
                update_flags(r0);
                break;
            }
            case OP_ST: {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                mem_write(reg[R_PC] + pc_offset, reg[r0]);
                break;
            }
            case OP_STI: {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                uint16_t addr = mem_read(reg[R_PC] + pc_offset);
                mem_write(addr, reg[r0]);
                break;
            }
            case OP_STR: {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset = sign_extend(instr & 0x3F, 6);
                mem_write(reg[r1] + offset, reg[r0]);
                break;
            }
            case OP_TRAP: {
                reg[R_R7] = reg[R_PC];
                switch (instr & 0xFF) {
                    case TRAP_GETC: {
                        reg[R_R0] = (uint16_t)getchar();
                        update_flags(R_R0);
                        break;
                    }
                    case TRAP_OUT: {
                        putc((char)reg [R_R0], stdout);
                        fflush(stdout);
                        break;
                    }
                    case TRAP_PUTS: {
                        uint16_t* c = memory + reg[R_R0];
                        while (*c) {
                            char char1 = (*c) & 0xFF;
                            putc(char1, stdout);
                            c++;
                        }
                        fflush(stdout);
                        break;
                    }
                    case TRAP_IN: {
                        // Get character with prompt and echo
                        printf("Enter a character: \n");
                        fflush(stdout);
                        char c = getchar();
                        putc(c, stdout);
                        fflush(stdout);
                        reg[R_R0] = (uint16_t)c;
                        update_flags(R_R0);
                        break;
                    }
                    case TRAP_PUTSP: {
                        // Output string with packed characters
                        uint16_t* c = memory + reg[R_R0];
                        while (*c) {
                            char char1 = (*c) & 0xFF;
                            putc(char1, stdout);
                            char char2 = (*c) >> 8;
                            if (char2) putc(char2, stdout);
                            c++;
                        }
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
            case OP_RES:
            case OP_RTI:
            default: {
                abort();
                break;
            }
        }
    }
    restore_input_buffering();
    return 0;
}
