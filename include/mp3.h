
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



/**
 * Reads the frame header and sets the information in the mp3 container
 *
 * @param position is the position of the start of the frame
 * @param mp3      contains various information necessary to deal with the data
 *
 * @return 0 if the function terminates successfully, -1 if it terminates on a failure
 */
int read_header(uint64_t position, mp3_container* mp3);


/**
 * Calculates the length of the current frame in bytes.
 * With the 4 bytes of the header included.
 *
 * @param bit_rate    is the bitrate specified in the header for the following data
 * @param sample_rate is the sampling rate specified in the header for the following data
 * @param padding     indicates whether there is padding added. (1 if there is padding, 0 otherwise)
 *
 * @return the length of the current frame in bytes
 */
uint32_t calculate_frame_length(uint32_t bit_rate, uint32_t sample_rate, byte padding);


/**
 * Verifies the current position and sets the stream
 *
 * @param stream   is the file stream
 * @param position is the intended position of the filestream
 *
 * @return 0 on success, 1 if the position is at the end of the file, -1 on failure
 */
int verify_position(FILE* stream, uint64_t position);


/**
 * Reads a Xing or Info tag
 *
 * @param mp3 is pointer to a container containing various information about the mp3 file
 *
 * @return 0 on success, -1 on failure
 */
int read_xing(mp3_container* mp3);

#endif // MP3_H
