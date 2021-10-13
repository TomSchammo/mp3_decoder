
#ifndef MP3_H
#define MP3_H

#include <stdint.h>
#include <stdio.h>

typedef uint8_t byte;

int read_header(uint64_t position, FILE* f);

uint32_t calculate_frame_length(uint32_t bit_rate, uint32_t sample_rate, byte padding);

#endif // MP3_H
