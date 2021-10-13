
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include "string.h"

#include "sys/stat.h"
#include "sys/types.h"
#include "mp3.h"
#include "errno.h"


/**
 * Function to skip ID3 Tags to test the mp3 decoder
 *
 * @param filename is the filname of the mp3 file
 *
 * @return an offset to where the mp3 part starts or -1 if opening or reading the file results in a failure
 */
int64_t get_starting_position(const char* filename) {
    int fd = open(filename, O_RDONLY);

    if (fd == -1) {
        printf("%s\n",strerror(errno));
        return -1;
    }

    byte buffer[10];

    ssize_t status = read(fd, (void*)buffer, 10);

    int64_t ret;

    if (status == -1) {
        ret = -1;
    }

    else
    {
        if ((char)buffer[0] == 'I' && (char)buffer[1] == 'D' && (char)buffer[2] == '3') {
            // size are bytes 6-9
            uint32_t size = (buffer[6] << (3*7)) | (buffer[7] << (2*7)) | (buffer[8] << (7)) | buffer[9];

            // add 10 bytes for header
            size += 10;

            if (buffer[5] & 0x10) {
                // add 10 more bytes for the footer
                size += 10;
            }

            ret = size;
        }

        else
            ret = 0;
    }

    close(fd);
    return ret;
}

int main(int argc, char *argv[])
{
    if (argc > 1) {
        int64_t start = get_starting_position(argv[1]);
        if (start == -1) {
            printf("Opening or reading the file resulted in a failure\n");
            return -1;
        }

        else {
            uint32_t start_position = (uint32_t)start;
            FILE* f = fopen(argv[1], "rb");

            mp3_container mp3 = {f, start_position, 0};

            read_header(mp3.start, mp3);
            fclose(f);
            printf("done reading\n");
            return 0;
        }
    }

    else {
        printf("You need to enter a filename\n");
        return -1;
    }
}
