#include "terminal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    //future implementation for windows compatibility

    void enableRawMode() {}

    void disableRawMode() {}

    void clearScreen() {}

    void goHome() {}

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
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        raw.c_oflag &= ~(OPOST);
        raw.c_iflag &= ~(IXON);

        //read function can return 0 if it read nothing in 0.1 seconds
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1; 

        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }

    void clearScreen()
    {
        // \x1b = ESC
        // [2J = clear screen
        const char * command = "\x1b[2J";
        write(STDOUT_FILENO, command, strlen(command));
    }

    void goHome()
    {
        // \x1b = ESC
        // [H  = move cursor home
        const char * command = "\x1b[H";
        write(STDIN_FILENO, command, strlen(command));
    }

    void setBeckgroundColor(const char * colorCode) {
        write(STDIN_FILENO, colorCode, strlen(colorCode));
        clearScreen();
        goHome();
    }

    void disableAlternativeScreen()
    {
        printf("\033[?1049l");
        fflush(stdout);
    }

    void enableAlternativeScreen()
    {
        atexit(disableAlternativeScreen);
        printf("\033[?1049h");
        fflush(stdout);
    }

    void setCursorBlock()
    {
        printf("\033[2 q");
    }

    void setFlashCursorBlock()
    {
        printf("\033[0 q");
    }

    void setCursorBar()
    {
        printf("\033[6 q");
    }

    void setFlashCursorBar()
    {
        printf("\033[5 q");
    }

    void resetCursorShape()
    {
        printf("\033[0 q");
        printf("\033[?25h");
    }
#endif