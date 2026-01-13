#pragma once

//text colors
#define COLOR_RESET     "\x1b[0m"
#define COLOR_RED       "\x1b[31m"
#define COLOR_GREEN     "\x1b[32m"
#define COLOR_WHITE     "\x1b[37m"
#define COLOR_GRAY      "\x1b[90m"

//beckground color
#define BG_DARK_GRAY    "\x1b[48;5;235m"
#define BG_BLACK        "\x1b[48;5;0m"
#define BG_RED          "\x1b[48;5;52m"
#define BG_BLUE         "\x1b[48;5;21m"
#define BG_CYAN         "\x1b[48;5;36m"


//FUNCTIONS DECLARATIONS
//----------------------
void enableRawMode();

void disableRawMode();

void clearScreen();

void goHome();

void setBeckgroundColor(const char * colorCode);

void enableAlternativeScreen();

void disableAlternativeScreen();

void setCursorBlock();

void setFlashCursorBlock();

void setCursorBar();

void setFlashCursorBar();

void resetCursorShape();