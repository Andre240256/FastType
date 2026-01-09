#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include <termios.h>
#include <unistd.h>

#include "include/terminal.h"

#define CTRL_KEY(k) ((k) & 0x1f)
#define SYMBOL_SPACE "·"
#define SYMBOL_ENTER "↵"

#define KEY_ENTER 13
#define ESC_KEY 27

enum configurations{
    UNLOCKED_CHARS = 1,
    BACKGROUND_COLOR,
    DIFICULTY
};

enum comandsProcessInput{
    TIMEOUT,
    EXIT,
    GOT_RIGHT,
    GOT_WRONG,
    DEFEAT
};

const char * dictionary = "/usr/share/dict/words";
const char * status = "src/configs/status.txt";
const int max_line_size = 80;
const int max_configLine_size = 100;

//global variables for theme
char currentTheme_BG[10];
char currentStyle_Todo[20];
char currentStyle_Correct[20];
char currentStyle_Wrong[20];

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


int main()
{
    if(!startFiles())   return 1;
    if(!startConfigs()) return 1;
    srand(time(NULL));


    enableRawMode();
    setBeckgroundColor(currentTheme_BG);
    clearScreen();
    goHome();

    char ** words = loadWords();
    if(words == NULL) return 1;

    int len = 0;
    for(int i = 0; i < number_words; i++)
        if(words[i] != NULL)
            len += strlen(words[i]);

    char * finalStr = (char *)malloc((len + number_words * 5 + 100)* sizeof(char));
    if(finalStr == NULL) return 1;
    finalStr[0] = '\0';

    for(int i = 0; i < number_words; i++)
        if(words[i] != NULL)
            if(i%nColums == nColums - 1){
                strcat(finalStr, words[i]);
                strcat(finalStr, "\r\n");
            }
            else{
                strcat(finalStr, words[i]);
                strcat(finalStr, " ");
            }
    
    int finalLen = strlen(finalStr);

    printf("%s", currentStyle_Todo);
    for(int i = 0; i < finalLen; i++)
            if(finalStr[i] == ' ')
                printf(SYMBOL_SPACE);
            else if(finalStr[i] == '\r')
                printf(SYMBOL_ENTER);
            else if(finalStr[i] == '\n')
                printf("\r\n");
            else 
                printf("%c", finalStr[i]);
    fflush(stdin);
    
    //free words after use
    for(int i = 0; i < number_words; i++){
        if(words[i] != NULL) free(words[i]);
    }
    free(words);
    goHome();

    // //MAIN LOGIC
    // //--------------
    int nread = 0;
    int i = 0;
    int first_letter_typed = 1;
    time_t startTime = 0;
    while(i < finalLen && nread != EXIT && nread != DEFEAT){
        nread = processInputKey(finalStr[i]);

        if(nread == TIMEOUT) continue;
        if(nread == EXIT) break;

        if(first_letter_typed){
            startTime = time(NULL);
            first_letter_typed = 0;
        }

        if(nread == GOT_RIGHT){
            printf("%s", currentStyle_Correct);
        }
        else{
            printf("%s", currentStyle_Wrong);
            nLives--;
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

        if(!nLives) nread = DEFEAT;
            
    }   
    time_t finalTime = time(NULL);

    if(nread != EXIT){
        float wordsPerMinute = getWordsPerMinute(startTime, finalTime, finalLen);
        printf("%s", currentStyle_Correct);
        if(nLives == 0)
            printf("\n\n\rYou lost! Good luck next time.\r\n");
        else{
            printf("You got it! nice play!\r\n");
            printf("Great speed, you typed %g words per minute!\r\n", wordsPerMinute);
        }
        printf("Please press cntr+q or esc to quit\r\n");
        fflush(stdin);

        nread = 0;
        while(nread != EXIT)
            nread = processInputKey('/0');
    }

    //AT EXIT
    //----------------------

    printf(COLOR_RESET "\n");
    setBeckgroundColor(COLOR_RESET);
    clearScreen();
    fflush(stdin);

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
        
            case DIFICULTY:
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

    return 1;
}


//LOADING LEVEL
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

    if(c == CTRL_KEY('q') || c == CTRL_KEY('c') || c == ESC_KEY){
        return EXIT;
    }

    if(rKey == '\r')
        if(c == KEY_ENTER || c == '\n')
            return GOT_RIGHT;
        else
            return GOT_WRONG;

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
