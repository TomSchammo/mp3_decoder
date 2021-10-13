
#ifndef MP3_H
#define MP3_H

#include <stdint.h>
#include <stdio.h>

typedef uint8_t byte;


/**
 * Struct that contains basic information about the current
 * mp3 file.
 *
 * stream:      a FILE* that contains an open stream for the file
 * start:       location of the first byte of the mp3 data
 * info_offset: the offset to the 'Info' or 'Xing' tag (from the beginning of the file)
 */
typedef struct {

    FILE* stream;
    uint32_t start;
    uint32_t info_offset;

} mp3_container;


int read_header(uint64_t position, mp3_container mp3);

uint32_t calculate_frame_length(uint32_t bit_rate, uint32_t sample_rate, byte padding);

#endif // MP3_H
