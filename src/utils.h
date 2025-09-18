#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

// The names of the functions are pretty self-explanatory ig.
void update_flags(uint16_t r);
uint16_t sign_extend(uint16_t x, int bit_count);
uint16_t mem_read(uint16_t address);
void mem_write(uint16_t address, uint16_t val);
void read_image_file(FILE *file);
int read_image(const char *image_path);
uint16_t swap16(uint16_t x);
void disable_input_buffering();
void restore_input_buffering();
uint16_t check_key();
void handle_interrupt(int signal);

#endif
