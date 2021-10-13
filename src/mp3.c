#include "mp3.h"
#include <fcntl.h>
#include <unistd.h>
#include "stdio.h"


// lookup for bitrates of MPEG-1 Layer 3 stanard in kb/s
const uint16_t bitrate_lookup_table[14] = {32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320};


// lookup table for sampling rates of MPEG-1 Layer 3 stanard in HZ/100
const uint16_t sampling_lookup_table[3] = {441, 480, 320};

typedef enum {
    Stereo = 0,
    JointStereo = 1,
    DualChannel = 2,
    Mono = 3
} ChannelMode;


typedef enum {
    None = 0,
    MS5015 = 1,
    Reserved = 2,
    CCITJ17 = 3
} Emphasis;

uint32_t calculate_frame_length(uint32_t bit_rate, uint32_t sample_rate, byte padding) {

    // int((144 * BitRate / SampleRate ) + Padding);

    printf("--------------\n");
    printf("bitrate: %d\nsampling rate: %d\npadding: %x\n", bit_rate, sample_rate, padding);
    printf("--------------\n");

    // TODO
    return (144 * bit_rate)/sample_rate + padding;
}

int read_header(uint64_t position, FILE* f) {

    if (ftell(f) != position)
        fseek(f, position, SEEK_SET);


    if (getc(f) == EOF) {
        printf("Reached the end of the file\n");
        return 0;
    }

    printf("position: %lu\n", position);

    byte buffer[4];

    fseek(f, -1, SEEK_CUR);

    long cur = ftell(f);

    if (cur != position) {
        printf("Position mismatch: %d != %d\nAborting...\n", cur, position);
        return -1;
    }

    uint64_t result = fread_unlocked(buffer, 1, 4, f);
    // ssize_t status = pread(f, (void*)buffer, 4, position);

    if (result != 4) {
        printf("[MP3] Reading the file resulted in a failure\n");
        return -1;
    }


    // 12 bit syncword
    // upper 4 bits are zeroed out
    uint16_t syncword = (buffer[0] << 4) | (buffer[1] >> 4);

    printf("syncword: 0x%03x\n", syncword);

    if (syncword != 0xfff) {
        printf("Syncword 0x%03x is not 0xfff\n", syncword);
        return -1;
    }

    byte t = buffer[1] & (0x0f);

    if ((t & 0x8) && (t & 0x06) == 0x02) {
        printf("MPEG1 Layer 3\n");

        if (t & 0x01)
            printf("Error protection is off\n");

        // 0000 and 1111 are not valid values (will result in crashing the program currently, probably should change that)
        uint32_t bitrate = bitrate_lookup_table[((buffer[2] & 0xf0) >> 4) -1] * 1000;


        printf("bitrate: %d\n", bitrate);
        uint32_t sampling_frequency = sampling_lookup_table[(buffer[2] & 0x0f) >> 2] * 100;

        // 0 -> not padded
        // 1 -> padding has been added
        byte padding = (buffer[2] & 0x02) >> 1;

        // application specific, doesn't matter
        // byte priv_bit = (buffer[2] & 0x01);

        ChannelMode mode = (buffer[3] & 0xf0) >> 6;

        if (mode == JointStereo) {
            // TODO figure out which joint stereo mode is used
            byte mode_extension_bin = (buffer[3] & 0x30) >> 4;
        }

        // 0 -> not copyrighted (not important)
        // byte copyright_bit = (buffer[3] & 0x0f) >> 3;

        // 0 -> copy (not important)
        // byte original_bit = (buffer[3] & 0x04) >> 2;

        Emphasis emphasis = buffer[3] & 0x03;

        uint32_t frame_length = calculate_frame_length(bitrate, sampling_frequency, padding);

        printf("frame length: %d bytes\n", frame_length);

        read_header(position + frame_length, f);

    }

    else {
        printf("This version of MPEG or this Layer is not supported!\n");
    }


    return 0;

}
