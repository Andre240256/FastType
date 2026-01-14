#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include <termios.h>
#include <unistd.h>

#include "terminal/include/terminal.h"
#include "stats/include/stats.h"
#include "debug/include/debug.h"

#define CTRL_KEY(k) ((k) & 0x1f)
#define SYMBOL_SPACE "·"
#define SYMBOL_ENTER "↵"
#define SYMBOL_VERTICAL_DOUBLE_BAR "║"

#define KEY_ENTER 13
#define ESC_KEY 27

typedef enum APPstate{
    STATE_START_MENU,
    STATE_WINNER,
    STATE_LOSER,
    STATE_MENU,
    STATE_GAME,
    STATE_EXIT,
    STATE_ERROR,
} APPstate;

typedef enum configurations{
    UNLOCKED_CHARS = 1,
    BACKGROUND_COLOR,
    DIFICULTY
} configurations;

typedef enum comandsProcessInput{
    TIMEOUT,
    EXIT,
    GO_TO_MENU,
    GOT_RIGHT,
    GOT_WRONG,
    DEFEAT,
    CONTINUE_GAME,
    PLAY_AGAIN,
} comandsProcessInput;

const char * dictionary = "/usr/share/dict/words";
const char * status = "src/configs/status.txt";
const int max_line_size = 80;
const int max_configLine_size = 100;

//global variables for theme
char currentTheme_BG[20];
char currentStyle_Todo[30];
char currentStyle_Correct[30];
char currentStyle_Wrong[30];

int number_words = 20;
int totalWords;
int nColums = 10;
char unlockedLetters[27];
char bgColor[15];
int nLives;

FILE * dict = NULL;
FILE * stats = NULL;

//QSORT COMPARING
int cmpInt(const void * a, const void * b){ return ( *(int*)a - *(int*)b); }

//INITIAL CONFIGURATIONS
int startConfigs();
int startFiles();

//FORMATING AUXILIAR FUNCTIONS
void removeNL(char * str);
void lower(char * str);

//LOADING LEVEL
//-------------
char * initFinalStr();
char * initScreenString(const char * finalStr);
void printScreenStr(const char * screenStr);
char ** loadWords();

//auxiliar functions loading level
void randomIntegers();
int getRandInt(int min, int max);
int isValidWord(const char * str);
void shuffleWords(char ** array);

//INPUT FUNCTIONS
//---------------
int processInputKey(char rKey);

//STATS FUNCTIONS
float getWordsPerMinute(time_t init, time_t end, int strlen);
int getTypableChars(const char * str, int strSize);

//STATEMACHINE FUNCTIONS
APPstate runMenu();
APPstate runGame(float * wpm, float * precision);
APPstate runStartMenu();
APPstate runWinnerMenu(float wpm, float precision);
APPstate runLoserMenu();

int main()
{
    if(!startFiles())   return 1;
    if(!startConfigs()) return 1;
    loadGameData();
    srand(time(NULL));

    enableRawMode();
    enableAlternativeScreen();
    resetCursorShape();

    APPstate currentState = runStartMenu();
    float wpm, precision;
    while(currentState != STATE_EXIT && currentState != STATE_ERROR)
        switch (currentState){
            case STATE_START_MENU:
                currentState = runStartMenu();
                break;
            case STATE_LOSER:
                currentState = runLoserMenu();
                break;
            case STATE_WINNER:
                currentState = runWinnerMenu(wpm, precision);
                break;
            case STATE_MENU:
                currentState = runMenu();
                break;
            case STATE_GAME:
                currentState = runGame(&wpm, &precision);
                break;
            case STATE_EXIT:
            case STATE_ERROR:
                break;
        }

    //AT EXIT
    //----------------------

    printf(COLOR_RESET "\n");
    setBeckgroundColor(COLOR_RESET);
    resetCursorShape();
    clearScreen();
    fflush(stdout);

    saveStats(charStats);
    disableAlternativeScreen();
    disableRawMode();

    if(dict)    fclose(dict);
    if(stats)  fclose(stats);
    
    return 0;
}

//AUXILIAR FORMATING FUNCTIONS
void removeNL(char * str)
{
    size_t len = strlen(str);
    if(len > 0 && str[len - 1] == '\n')
        str[len - 1] = '\0';
    if(len > 0 && str[len - 1] == '\r') 
        str[len - 1] = '\0';
}

void lower(char * str)
{
    for(int i = 0; str[i] != '\0'; i++)
        str[i] = tolower(str[i]);
}

//INITIAL CONFIGURATIONS
int startFiles()
{
    dict = fopen(dictionary, "r");
    if(!dict){
        printf("Error opening dataset: %s\n", dictionary);
        return 0;
    }

    stats = fopen(status, "r");
    if(!stats){
        printf("Error opening games status: %s\n", status);
        fclose(dict);
        return 0;
    }

    return 1;
}

int startConfigs()
{
    char configLine[max_configLine_size];
    while(fgets(configLine, sizeof(configLine), stats))
    {
        int configID = configLine[0] - '0';

        switch (configID)
        {
            int i;
            case UNLOCKED_CHARS:
                for(i = 0; configLine[i+2] != ';' && configLine[i+2] != '\n'; i++)
                    unlockedLetters[i] = tolower(configLine[i+2]);
                unlockedLetters[i] = '\0';
                break;

            case BACKGROUND_COLOR:
                for(i = 0; configLine[i+2] != ';' && configLine[i+2] != '\n'; i++)
                    bgColor[i] = tolower(configLine[i+2]); 
                bgColor[i] = '\0';

                if(!strcmp(bgColor, "blue")){
                    strcpy(currentTheme_BG, BG_BLUE);
                }
                else if(!strcmp(bgColor, "red")){
                    strcpy(currentTheme_BG, BG_RED);
                }
                else if(!strcmp(bgColor, "black")){
                    strcpy(currentTheme_BG, BG_BLACK);
                }
                else if(!strcmp(bgColor, "cyan")){
                    strcpy(currentTheme_BG, BG_CYAN);
                }
                else {
                    strcpy(currentTheme_BG, BG_DARK_GRAY);
                }

                
                sprintf(currentStyle_Correct, "%s%s", currentTheme_BG, COLOR_WHITE);
                sprintf(currentStyle_Todo, "%s%s", currentTheme_BG, COLOR_GRAY);
                sprintf(currentStyle_Wrong, "%s%s", currentTheme_BG, COLOR_RED);
                break;
        
            case DIFICULTY:{
                char str[15];
                for(i = 0; configLine[i+2] != ';' && configLine[i+2] != '\n'; i++)
                    str[i] = tolower(configLine[i+2]);
                str[i] = '\0';

                if(!strcmp(str, "easy")){
                    nLives = __INT_MAX__;
                }
                else if (!strcmp(str, "hard")){
                    nLives = 3;
                }
                else if(!strcmp(str, "Impossible")){
                    nLives = 1;
                }
                else{
                    nLives = 7;
                }
                break;
            }
        }
    }

    return 1;
}


//LOADING LEVEL
char * initFinalStr()
{
    char ** words = loadWords();
    if(words == NULL) return NULL;

    int len = 0;
    for(int i = 0; i < number_words; i++)
        if(words[i] != NULL)
            len += strlen(words[i]);

    char * finalStr = (char *)malloc((len + number_words * 5 + 100)* sizeof(char));
    if(finalStr == NULL) return NULL;
    finalStr[0] = '\0';

    for(int i = 0; i < number_words; i++){
        if(words[i] != NULL){
            if(i%nColums == nColums - 1){
                strcat(finalStr, words[i]);
                strcat(finalStr, "\r\n");
            }
            else{
                strcat(finalStr, words[i]);
                strcat(finalStr, " ");
            }
        }
    }
    
    //free words
    for(int i = 0; i < number_words; i++)
        if(words[i] != NULL)
            free(words[i]);
    free(words);

    return finalStr;
}

char * initScreenString(const char * finalStr)
{
    int count = 0;
    int finalLen = strlen(finalStr);
    for(int i = 0; i < finalLen; i++, count++){
        if(finalStr[i] == '\r')
            count++;
    }

    char * screenString = (char *)malloc(sizeof(char) * (count + 1));

    int currentIdx = 0;
    for(int i = 0; i < finalLen; i++)
        if(finalStr[i] == '\r')
            screenString[currentIdx++] = '+';
        else if(finalStr[i] == '\n'){
            screenString[currentIdx++] = '\r';
            screenString[currentIdx++] = '\n';
        }
        else{
            screenString[currentIdx++] = finalStr[i];
        }
    screenString[currentIdx] = '\0';

    return screenString;
}

void printScreenStr(const char * screenStr)
{
    int len = strlen(screenStr);
    for(int i = 0; i < len; i++)
        if(screenStr[i] == ' ')
            printf(SYMBOL_SPACE);
        else if(screenStr[i] == '+')
            printf(SYMBOL_ENTER);
        else
            printf("%c", screenStr[i]);
}

int isValidWord(const char * str)
{
    size_t len = strlen(str);
    if(len < 3 || len > 10) return 0;

    if(strspn(str, unlockedLetters) == len)
        return 1;

    return 0;
}

int getRandInt(int min, int max)
{
    return min + rand() % (max - min + 1);
}

void randomIntegers(int * idx)
{
    for(int i = 0; i < number_words; i++)
    {
        idx[i] = getRandInt(0, totalWords - 1);
    }

    qsort(idx, number_words, sizeof(int), cmpInt);
}

void shuffleWords(char ** array)
{
    for(int i = number_words - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);

        char * temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

char ** loadWords()
{
    totalWords = 0;
    char line[max_line_size];

    rewind(dict);
    while(fgets(line, sizeof(line), dict))
    {
        removeNL(line);
        lower(line);

        if(isValidWord(line))
        {
            totalWords++;
        }
    }

    if(totalWords < number_words) number_words = totalWords;
    
    int idx[number_words];
    randomIntegers(idx);

    rewind(dict);

    char ** words = (char **)malloc(sizeof(char *) * number_words);

    int currentValidIndex = 0, itr = 0, storedCount = 0;
    while(fgets(line, sizeof(line), dict) && storedCount < number_words)
    {
        removeNL(line);
        lower(line);
        if(isValidWord(line)){
            while(itr < number_words && currentValidIndex == idx[itr]){
                words[storedCount] = (char *)malloc(sizeof(char) * (strlen(line) + 1));
                strcpy(words[storedCount], line);

                storedCount++, itr++;
            }
            currentValidIndex++;
        }
    }

    shuffleWords(words);

    return words;
}

int processInputKey(char rKey){
    char c;

    int nread = read(STDIN_FILENO, &c, sizeof(c));

    if(nread <= 0)  return TIMEOUT;

    if(c == CTRL_KEY('q') || c == CTRL_KEY('c')){
        return EXIT;
    }
    if(c == ESC_KEY)
        return GO_TO_MENU;

    if(rKey == '\r'){
        if(c == KEY_ENTER || c == '\n')
            return GOT_RIGHT;
        else
            return GOT_WRONG;
    }

    if(c == rKey) 
        return GOT_RIGHT;
    else 
        return GOT_WRONG;

    return 1;
}

float getWordsPerMinute(time_t init, time_t end, int charCount)
{
    double elapsedSeconds = difftime(end, init);
    if(elapsedSeconds <= 0.0) return 0.0f;

    //tempo em minutos
    double elapsedMinutes = elapsedSeconds / 60.0;
    float totalWords = (float)charCount / 5.0f;
    
    float result = totalWords/elapsedMinutes;
    return result;
}

int getTypableChars(const char * str, int strSize)
{
    int count = 0;
    for(int i = 0; str[i] != '\0' && i < strSize; i++){
        if(str[i] != '\r')
            count++;
    }
    return count;
}


//STATEMACHINE FUNCTIONS
APPstate runMenu() {
    clearScreen();
    setBeckgroundColor(currentTheme_BG);
    goHome();
    printf("%s", currentStyle_Correct);
    goHome();

    printf("################  MENU ###############\r\n\r\n");
    printf("STATS: ");
    for(int i = 0; i < 26; i++){
        if(i%4 == 0)
            printf("\r\n");
        printf("'%c' : %.2f wpm " SYMBOL_VERTICAL_DOUBLE_BAR, 'a' + i, charStats['a' + i].currentWpm);
    }
    printf("\r\n####################################\r\n");
    printf("\r\nType 'c' to continue playing\r\n");
    printf("Type 'cntrl + q' to quit game\r\n");
    printf("\r\n######################################\r\n");
    fflush(stdout);

    int nread = 0;
    char c;
    int action = 0;
    while(action != EXIT && action != CONTINUE_GAME){
        nread = read(STDIN_FILENO, &c, sizeof(c));
        if(nread == TIMEOUT) continue;
        
        printf("%c\r\n", c);
        if(c == CTRL_KEY('q') || c == ESC_KEY)
            action = EXIT;
        else if(c == 'c')
            action = CONTINUE_GAME;
    }

    switch (action)
    {
    case EXIT:
        return STATE_EXIT;
    case CONTINUE_GAME:
        return STATE_GAME;
    default:
        break;
    }

    return -1;
}

APPstate runGame(float * wpm, float * precision)
{
    int lives = nLives;
    char * finalStr = initFinalStr();
    if(finalStr == NULL) return STATE_ERROR;
    
    setBeckgroundColor(currentTheme_BG);
    clearScreen();
    goHome();
    setCursorBlock();
    printf("%s", currentStyle_Todo);


    char * screenStr = initScreenString(finalStr);
    printScreenStr(screenStr);
    free(screenStr);
    
    int nread = 0, i = 0, firstLetterTyped = 1;
    int gotWrong = 0;
    int finalLen = strlen(finalStr);
    time_t init, end;
    goHome();
    while(i < finalLen && nread != EXIT && nread != DEFEAT && nread != GO_TO_MENU){
        nread = processInputKey(finalStr[i]);

        if(nread == TIMEOUT) continue;
        if(nread == EXIT || nread == GO_TO_MENU) break;

        if(firstLetterTyped){
            init = time(NULL);
            clock_gettime(CLOCK_MONOTONIC, &lastKeyPressTime);
            firstLetterTyped = 0;
            // logDebug("JOGO INICIOU: Resetando clock.");
        }

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        if(nread == GOT_RIGHT){
            printf("%s", currentStyle_Correct);
            registerKeyStats(finalStr[i], &now);
        }
        else{
            printf("%s", currentStyle_Wrong);
            lives--;
            gotWrong++;
            lastKeyPressTime = now;
        }

        if(finalStr[i] == '\r'){
            printf(SYMBOL_ENTER "\r\n");
            i += 2;
        }
        else if(finalStr[i] == ' '){
            printf(SYMBOL_SPACE);
            i++;
        }
        else{
            printf("%c", finalStr[i]);
            i++;
        }

        printf("%s", currentStyle_Todo);
        fflush(stdout);

        if(!lives) nread = DEFEAT;
    }

    resetCursorShape();

    end = time(NULL); 
    switch (nread){
        case EXIT:
            free(finalStr);
            return STATE_EXIT;
        case DEFEAT:
            free(finalStr);
            return STATE_LOSER;
        case GO_TO_MENU:
            free(finalStr);
            return STATE_MENU;
    }

    int charCount = getTypableChars(finalStr, finalLen);
    *precision = (float)(charCount - gotWrong)/(float)charCount*100;
    *wpm = getWordsPerMinute(init, end, charCount);
    free(finalStr);
    
    return STATE_WINNER;
}

APPstate runLoserMenu()
{
    setBeckgroundColor(currentTheme_BG);
    goHome();

    printf("################ YOU LOST ##############\r\n\r\n");
    printf("You lost! Maybe nest time...\r\n");
    printf("Type 'y' if you would like to play again, or 'ctrl+ q' if you would like to quit\r\n\t");
    fflush(stdout);

    int nread = 0;
    char c;
    while(nread != EXIT && nread != PLAY_AGAIN && nread != GO_TO_MENU){
        nread = read(STDIN_FILENO, &c, sizeof(c));

        if(nread == TIMEOUT) continue;

        c = tolower(c);
        if(c == 'y')
            nread = PLAY_AGAIN;
        else if(c == CTRL_KEY('q') || c == CTRL_KEY('c'))
            nread = EXIT;
        else if(c == ESC_KEY)
            nread = GO_TO_MENU;
        else 
            nread = 0;
    }

    printf("\r\n");
    clearScreen();
    goHome();
    fflush(stdout);

    switch (nread)
    {
    case EXIT:
        return STATE_EXIT;
        break;
    case PLAY_AGAIN:
        return STATE_GAME;
    case GO_TO_MENU:
        return STATE_MENU;
    default:
        break;
    }

    return -1;
}

APPstate runWinnerMenu(float wpm, float precision)
{
    printf("%s", currentStyle_Correct);
    printf("\r\n\r\n");
    printf("################# WINNER #####################\r\n");
    printf("Congratulation! You typed %.2f words per minute, that's really fast!\r\n", wpm);
    printf("All of that with %.2f%% precision!\n\r", precision);
    printf("Type 'c' to play again or 'ctrl + q' to quit\r\n");
    printf("##############################################\r\n");
    fflush(stdout);

    int action = 0;
    int nread = 0;
    char c;
    while(action != EXIT && action != CONTINUE_GAME && action != GO_TO_MENU){
        nread = read(STDIN_FILENO, &c, sizeof(c));

        if(nread == TIMEOUT) continue;

        c = tolower(c);
        printf("%c\r\n", c);
        if(c == CTRL_KEY('q') || c == CTRL_KEY('c'))
            action = EXIT;
        if(c == ESC_KEY)
            action = GO_TO_MENU;
        if(c == 'c')
            action = CONTINUE_GAME;
    }

    switch (action)
    {
    case EXIT:
        return STATE_EXIT;
    case GO_TO_MENU:
        return STATE_MENU;
    case CONTINUE_GAME:
        return STATE_GAME;
    default:
        break;
    }

    return -1;
}

APPstate runStartMenu()
{
    const char * initialMsgPath = "src/configs/initialMsg.txt";
    FILE * initialMsg = fopen(initialMsgPath, "r");

    setBeckgroundColor(currentTheme_BG);
    clearScreen();
    goHome();
    printf("%s", currentStyle_Correct);
    fflush(stdout);
    goHome();
   
    if(initialMsg == NULL)
        printf("Welcome to FastTyping! (Art not found)\r\n");
    else{
        char line[512];
        while(fgets(line, sizeof(line), initialMsg))
        {
            line[strcspn(line, "\n")] = '\0';
            printf("%s\r\n", line);
        }

        fclose(initialMsg);
    }

    
    
    printf("\r\n\r\nHere you can train to type really fast!\r\n");
    printf("Type ['s'] to start a match.\r\n");
    printf("Type ['cntrl' + 'q'] to quit aplication.\r\n");
    fflush(stdout);
    
    char command;
    int nread = 0;
    int validCommand = 0;
    while(!validCommand){
        nread = read(STDIN_FILENO, &command, sizeof(command));

        if(nread == TIMEOUT) continue;

        if(iscntrl(command))
            printf("'%d'\r\n", command);
        else
            printf("%c\r\n", command);
            
        command = tolower(command);
        if(command == 's'){
            validCommand = 1;
            return STATE_GAME;
        }
        else if(command == CTRL_KEY('q')){
            validCommand = 1;
            return STATE_EXIT;
        }
        else if(command == ESC_KEY){
            validCommand = 1;
            return STATE_MENU;
        }

        if(!validCommand)
            printf("Not a valid command, please type it again:\r\n");
    }

    return -1;
}