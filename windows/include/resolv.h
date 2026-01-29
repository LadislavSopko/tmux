/*
 * resolv.h - Windows compatibility shim for DNS resolver
 *
 * Provides base64 encoding functions and resolver structures.
 */

#ifndef COMPAT_RESOLV_H
#define COMPAT_RESOLV_H

#ifdef _WIN32

#include <winsock2.h>
#include <string.h>

/*
 * b64_ntop - encode binary data as base64
 *
 * Returns the length of the encoded string, or -1 on error.
 */
static __inline int b64_ntop(const unsigned char *src, size_t srclength,
                             char *target, size_t targsize)
{
    static const char b64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t datalength = 0;
    unsigned char input[3];
    unsigned char output[4];
    size_t i;

    while (srclength > 2) {
        input[0] = *src++;
        input[1] = *src++;
        input[2] = *src++;
        srclength -= 3;

        output[0] = b64[input[0] >> 2];
        output[1] = b64[((input[0] & 0x03) << 4) + (input[1] >> 4)];
        output[2] = b64[((input[1] & 0x0f) << 2) + (input[2] >> 6)];
        output[3] = b64[input[2] & 0x3f];

        if (datalength + 4 > targsize)
            return -1;
        target[datalength++] = output[0];
        target[datalength++] = output[1];
        target[datalength++] = output[2];
        target[datalength++] = output[3];
    }

    if (srclength != 0) {
        input[0] = input[1] = input[2] = '\0';
        for (i = 0; i < srclength; i++)
            input[i] = *src++;

        output[0] = b64[input[0] >> 2];
        output[1] = b64[((input[0] & 0x03) << 4) + (input[1] >> 4)];
        if (srclength == 1)
            output[2] = '=';
        else
            output[2] = b64[((input[1] & 0x0f) << 2) + (input[2] >> 6)];
        output[3] = '=';

        if (datalength + 4 > targsize)
            return -1;
        target[datalength++] = output[0];
        target[datalength++] = output[1];
        target[datalength++] = output[2];
        target[datalength++] = output[3];
    }

    if (datalength >= targsize)
        return -1;
    target[datalength] = '\0';
    return (int)datalength;
}

/*
 * b64_pton - decode base64 to binary
 *
 * Returns the length of the decoded data, or -1 on error.
 */
static __inline int b64_pton(const char *src, unsigned char *target, size_t targsize)
{
    static const int b64dec[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    };
    size_t tarindex = 0;
    int state = 0;
    int ch;
    unsigned char nextbyte;

    while ((ch = (unsigned char)*src++) != '\0') {
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
            continue;
        if (ch == '=')
            break;

        int pos = b64dec[ch];
        if (pos == -1)
            return -1;

        switch (state) {
        case 0:
            if (tarindex >= targsize)
                return -1;
            target[tarindex] = (unsigned char)(pos << 2);
            state = 1;
            break;
        case 1:
            if (tarindex >= targsize)
                return -1;
            target[tarindex++] |= (unsigned char)(pos >> 4);
            nextbyte = (unsigned char)((pos & 0x0f) << 4);
            if (tarindex < targsize)
                target[tarindex] = nextbyte;
            state = 2;
            break;
        case 2:
            if (tarindex >= targsize)
                return -1;
            target[tarindex++] |= (unsigned char)(pos >> 2);
            nextbyte = (unsigned char)((pos & 0x03) << 6);
            if (tarindex < targsize)
                target[tarindex] = nextbyte;
            state = 3;
            break;
        case 3:
            if (tarindex >= targsize)
                return -1;
            target[tarindex++] |= (unsigned char)pos;
            state = 0;
            break;
        }
    }

    return (int)tarindex;
}

#endif /* _WIN32 */
#endif /* COMPAT_RESOLV_H */
