#include "beep.h"
#define PIT_FREQ 0x1234DD

int make_beep()
{
    int notes[8] = {131,147,165,175,196,220,245,262};
    int i, counter;

    kOutPortByte(0x61, kInPortByte(0x61) | 3);
    kOutPortByte(0x43, 0xb6);

    for(i = 0; i < 8; i++)
    {
        counter = (PIT_FREQ / notes[i % 8]);
        kOutPortByte(0x42, counter & 0xff);
        kOutPortByte(0x42, counter >> 8);
        
    }
}