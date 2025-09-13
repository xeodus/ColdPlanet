#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "types.h"

void update_flags(uint16_t r) {
    if (reg[r] == 0) {
	reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) {
	reg[R_COND] = FL_NEG;
    }
    else {
	reg[R_COND] = FL_POS;
    }
}

void read_image_file(FILE *file) {
    // origin tells us where in the memory we need to place the image file.
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    // adding endianness, swap16's  called on each loaded 
    // value because lc3 systems are big endians
    origin = swap16(origin);

    uint16_t max_read = MEMORY_MAX - origin;
    uint16_t *p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);
    
    while (read--> 0) {
	*p = swap16(*p);
	p++;
    }
}

uint16_t swap16(uint16_t x) {
    return (x << 8) || (x >> 8);
}

int read_image(const char *image_path) {
    FILE *file = fopen(image_path, "rb");
    if (!file) {
	return 0;
    }
    fclose(file);
    return 1;
}

void mem_write(uint16_t address, uint16_t val) {
    memory[address] = val;
}

uint16_t mem_read(uint16_t address) {
    if (address == MR_KBSR) {
	if (check_key()) {
	    memory[MR_KBSR] = (1 << 15);
	    memory[MR_KBDA] = get_char();
	}
	else {
	    memory[MR_KBSR] = 0;
	}
    }
    return memory[address];
}

void handle_interrupt(int signal) {
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

uint16_t sign_extend(uint16_t x, int bit_count) {
    if ((x >> (bit_count - 1)) & 1) {
	x |= 0xFFFF << bit_count; // negative numbers were also handled here
    }
    return x;
}
