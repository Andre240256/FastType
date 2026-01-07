#include "terminal.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
    //future implementation for windows compatibility

    void enableRawMode() {}

    void disableRawMode() {}

    void clearScreen() {}

#else
    #include <termios.h>
    #include <unistd.h>

    struct termios orig_termios;

    void disableRawMode(){
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    }

    void enableRawMode()
    {
        tcgetattr(STDIN_FILENO, &orig_termios);
        atexit(disableRawMode);

        struct termios raw = orig_termios;

        //optional ISIG: turn off ctrl-c
        raw.c_lflag &= ~(ECHO | ICANON | ISIG);
        raw.c_oflag &= ~(OPOST);

        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }

    void clearScreen()
    {
        // \x1b = ESC
        // [2J = clear screen
        // [H  = move cursor home
        const char * command = "\x1b[2J\x1b[H";
        write(STDOUT_FILENO, command, strlen(command));
    }

#endif