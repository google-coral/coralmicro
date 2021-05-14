#include <stdio.h>

void DebugLog(const char *s) __attribute__((weak));
void DebugLog(const char *s) {
    printf(s);
}