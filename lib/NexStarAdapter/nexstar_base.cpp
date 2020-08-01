/******************************************************************
    Author:     Juan Menendez Blanco    <juanmb@gmail.com>

    This code is part of the NexStarAdapter project:
        https://github.com/juanmb/NexStarAdapter

*******************************************************************/


#include <string.h>
#include "nexstar_base.h"


// The number 0x12345678 will be converted into {0x12, 0x34, 0x56}
void uint32To24bits(uint32_t in, char *out)
{
    uint32_t tmp = in;
    for (int i=0; i<3; i++) {
        tmp >>= 8;
        out[2-i] = tmp & 0xff;
    }
}


// The char array {0x12, 0x34, 0x56} will be converted into 0x12345600
uint32_t uint32From24bits(char *data)
{
    uint32_t out = 0;

    for (int i=0; i<3; i++) {
        out |= data[i] & 0xff;
        out <<= 8;
    }
    return out;
}
