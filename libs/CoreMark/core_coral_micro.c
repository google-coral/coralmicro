#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "core_portme.h"

static char *printf_buf;
static int current_buffer_length;
static bool print_to_console = true;

int ee_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    /* Use stdio printf for console output */
    if (print_to_console) {
        vprintf(fmt,args);
        printf("\r");
    }
    /* Append message to buffer */
    current_buffer_length += vsnprintf(printf_buf + current_buffer_length, MAX_COREMARK_BUFFER-current_buffer_length, fmt, args);
    current_buffer_length += vsnprintf(printf_buf + current_buffer_length, MAX_COREMARK_BUFFER-current_buffer_length, "\r", args);
    va_end(args);
    return 0;
}

void CoreMark_PrintToConsole(bool print) {
    print_to_console = print;
}

void RunCoreMark(char* buffer) {
    printf_buf = buffer;
    current_buffer_length = 0;
    coremark_main();
}


