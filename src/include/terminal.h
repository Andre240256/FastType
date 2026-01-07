#pragma once

//text colors
#define COLOR_RESET     "\x1b[0m"
#define COLOR_RED       "\x1b[31m"
#define COLOR_GREEN     "\x1b[32m"
#define COLOR_WHITE     "\x1b[37m"
#define COLOR_GRAY      "\x1b[90m"

//beckground color
#define BG_BLACK        "\x1b[40m"
#define BG_BLUE         "\x1b[44m"

//combinations color
#define APP_BG BG_BLUE
#define STYLE_TODO      APP_BG COLOR_GRAY
#define STYLE_CORRECT   APP_BG COLOR_WHITE
#define STYLE_WRONG     APP_BG COLOR_RED


//FUNCTIONS DECLARATIONS
//----------------------
void enableRawMode();

void disableRawMode();

void clearScreen();

void goHome();

void setBeckgroundColor(const char * colorCode);