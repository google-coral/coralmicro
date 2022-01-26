#include "Arduino.h"

#include <cstdlib>

void randomSeed(unsigned long seed)
{
    if (seed != 0) {
        srand48(seed);
    }
}

long random(long max)
{
    if (max == 0) {
        return 0;
    }
    return lrand48() % max;
}

long random(long min, long max)
{
    long diff = max - min;

    if (diff <= 0) {
        diff = 0;
    }

    return random(diff) + min;
}
