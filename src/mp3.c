#include "mp3.h"
#include <fcntl.h>
#include <unistd.h>

#define XING 0x58696E67
#define INFO 0x496E666F

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


int read_crc(uint64_t position, mp3_container *mp3, uint16_t* crc) {

    int status = verify_position(mp3->stream, position);

    if (status == -1 || status == 1)
        return -1;

    byte result = fread(crc, sizeof(uint16_t), 1, mp3->stream);

    if (result != sizeof(uint16_t)) {
        printf("[MP3] Reading the file resulted in a failure\n");
        return -1;
    }

    return 0;
}

uint32_t calculate_frame_length(uint32_t bit_rate, uint32_t sample_rate, byte padding) {

    // TODO this 144 differs for other MPEG version and Layers
    return (144 * bit_rate)/sample_rate + padding;
}


int verify_position(FILE* stream, uint64_t position) {

    if (ftell(stream) != position)
        fseek(stream, position, SEEK_SET);


    if (getc(stream) == EOF) {
        printf("Reached the end of the file\n");
        return 1;
    }

    printf("position: %lu\n", position);

    fseek(stream, -1, SEEK_CUR);

    long cur = ftell(stream);

    if (cur != position) {
        printf("Position mismatch: %ld != %lu\nAborting...\n", cur, position);
        return -1;
    }

    return 0;

}


int read_header(uint64_t position, mp3_container* mp3) {


    int status = verify_position(mp3->stream, position);

    // failure
    if (status == -1)
        return -1;

    // EOF
    else if (status == 1)
     return 0;


    byte buffer_size = 4;
    byte buffer[buffer_size];


    // TODO I don't know if the file has to be locked.
    //      Theoretically there should not be another program reading,
    //      but I'll leave it like this for now.
    uint64_t result = fread(buffer, sizeof(byte), buffer_size, mp3->stream);


    if (result != 4) {
        // TODO should probably just try again and read what's missing
        printf("[MP3] Reading the file resulted in a failure\n");
        return -1;
    }


    // 12 bit syncword
    uint16_t syncword = (buffer[0] << 4) | (buffer[1] >> 4);


    if (syncword != 0xfff) {
        printf("Syncword 0x%03x is not 0xfff\n", syncword);
        return -1;
    }


    byte version_layer = buffer[1] & (0x0f);

    if ((version_layer & 0x8) && (version_layer & 0x06) == 0x02) {
        printf("MPEG1 Layer 3\n");

        // TODO does this matter?
        if (version_layer & 0x01)
            printf("Error protection is off\n");

        byte bitrate_key = (buffer[2] & 0xf0) >> 4;

        // 0000 and 1111 are not valid values
        if (bitrate_key == 0x0 || bitrate_key == 0xf) {
            printf("bitrate_key of 0x%x is not valid\n", bitrate_key);
            return -1;
        }

        uint32_t bitrate = bitrate_lookup_table[bitrate_key -1] * 1000;
        printf("bitrate: %d\n", bitrate);


        uint32_t sampling_frequency = sampling_lookup_table[(buffer[2] & 0x0f) >> 2] * 100;
        printf("sampling frequency: %dHz\n", sampling_frequency);

        byte padding = (buffer[2] & 0x02) >> 1;

        ChannelMode mode = (buffer[3] & 0xf0) >> 6;

        if (mode == Mono)
            printf("Mono\n");
        else if (mode == JointStereo)
            printf("Joint Stereo\n");
        else if (mode == Stereo)
            printf("Stereo\n");
        else if (mode == DualChannel)
            printf("Dual Channel\n");
        else
            printf("Error: Channel mode with value %d not known", mode);


        if (mp3->info_offset == 0) {
            if (mode == Mono) {
                mp3->info_offset = mp3->start + 21;
            }

            else {
                mp3->info_offset = mp3->start + 36;

                if (mode == JointStereo) {
                    // TODO figure out which joint stereo mode is used
                    byte mode_extension_bin = (buffer[3] & 0x30) >> 4;
                }
            }
        }

        else {
            if (mode == JointStereo) {
                // TODO figure out which joint stereo mode is used
                byte mode_extension_bin = (buffer[3] & 0x30) >> 4;
            }
        }


        Emphasis emphasis = buffer[3] & 0x03;

        uint32_t frame_length = calculate_frame_length(bitrate, sampling_frequency, padding);

        printf("frame length: %d bytes\n", frame_length);

        read_header(position + frame_length, mp3);

    }

    else {
        printf("This version of MPEG or this Layer is not supported!\n");
    }


    return 0;

}


int read_xing(mp3_container* mp3) {

    int status = verify_position(mp3->stream, mp3->info_offset);

    // failure or EOF
    if (status == -1 || status == 1)
        return -1;

    uint32_t tag;

    uint64_t result = fread(&tag, sizeof(uint32_t), 1, mp3->stream);

    if (tag == INFO || tag == XING) {
        // TODO
    }

    else {
        printf("Position mismatch. Expected magic bytes 'Info' (0x%x) or 'Xing' (0x%x), but got 0x%x instead\n", INFO, XING, tag);
        return -1;
    }

    return 0;
}





int read_data(uint64_t position, mp3_container* mp3) {

    int position_status = verify_position(mp3->stream, position);

    if (position_status == -1 || position_status == 1) {

        printf("Could not verify position (verify_position returned %d)", position_status);
        return -1;
    }


    uint32_t data_length = mp3->frame_length -4;

    byte buffer[data_length];


    uint64_t result = fread(buffer, 1, data_length, mp3->stream);


    if (result != data_length) {
        // TODO should probably just try again and read what's missing
        printf("[MP3] [read_data] Reading the file resulted in a failure\n");
        return -1;
    }

    // TODO what to do with data???


    return 0;
}

