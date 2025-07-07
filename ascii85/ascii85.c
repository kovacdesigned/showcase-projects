#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

void encode_output(long res_bytes);
void decode_output(long res);

int encode(void)
{
    long input;
    long result_bytes = 0;
    short int char_counter = 0;

    // read from input
    while ((input = getchar()) != EOF) {

        // place bytes next to each other
        input = input << (8 * (3 - char_counter));
        result_bytes += input;
        char_counter += 1;

        // encode 4 characters at a time
        if (char_counter == 4) {
            encode_output(result_bytes);
            result_bytes = 0;
            char_counter = 0;
        }
    }

    // encode leftover bytes
    if (char_counter != 0) {
        encode_output(result_bytes);
    }
    printf("\n");
    return 0;
}

void encode_output(long res_bytes)
{
    // output encoded characters
    int remainder = 0;
    for (int i = 0; i < 5; i++) {
        remainder = res_bytes % 85;
        res_bytes /= 85;
        printf("%c", remainder + 33);
    }
}

int decode(void)
{
    long input;
    long result = 0;
    int powers = 1;
    short int char_counter = 0;

    // read from input
    while ((input = getchar()) != EOF) {

        // ignore whitespace characters
        if (isspace(input)) {
            continue;
        }

        // check for invalid characters
        if (input < 33 || input > 125) {
            return 1;
        }

        // decode input
        input -= 33;
        input *= powers;
        result += input;
        powers *= 85;
        char_counter += 1;

        // output 5 characters at a time
        if (char_counter == 5) {

            // check if decoded input is bigger than 4 bytes
            if (result > 0xFFFFFFFF) {
                return 1;
            }

            decode_output(result);
            result = 0;
            powers = 1;
            char_counter = 0;
        }
    }

    // check if input length was not divisible by 5
    if (char_counter != 0) {
        return 1;
    }
    return 0;
}

void decode_output(long res)
{
    // output decoded characters
    long res_copy;
    int output;
    for (int i = 0; i < 4; i++) {
        res_copy = res;
        res_copy = res_copy >> (8 * (3 - i));
        res_copy = res_copy & 255;
        output = res_copy;
        printf("%c", output);
    }
}


int main(int argc, char *argv[])
{
    int retcode = 1;

    if (argc == 1 || (argc == 2 && strcmp(argv[1], "-e") == 0)) {
        retcode = encode();
    } else if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        retcode = decode();
    } else {
        fprintf(stderr, "usage: %s [-e|-d]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (retcode != 0) {
        fprintf(stderr, "an error occurred\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
