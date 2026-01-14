#include <stdio.h>
#include <stdarg.h>

void logDebug(const char * format, ...)
{
    FILE * file = fopen("debug.txt", "a");
    if(file == NULL) return;

    va_list args;
    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);

    fprintf(file, "\n");
    fclose(file);
}