#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include <termios.h>
#include <unistd.h>

#include "include/terminal.h"

#define CTRL_KEY(k) ((k) & 0x1f)

enum configurations{
    UNLOCKED_CHARS = 1
};

enum comandsProcessInput{
    TIMEOUT,
    EXIT,
    GOT_RIGHT,
    GOT_WRONG
};

const char * dictionary = "/usr/share/dict/words";
const char * status = "src/configs/status.txt";
const int max_line_size = 80;
const int max_configLine_size = 100;

int number_words = 20;
int totalWords;
char unlockedLetters[27];

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


int main()
{
    if(!startFiles())   return 1;
    if(!startConfigs()) return 1;
    srand(time(NULL));

    enableRawMode();
    setBeckgroundColor(BG_BLUE);
    clearScreen();
    goHome();

    char ** words = loadWords();
    if(words == NULL) return 1;

    //MAIN LOGIC
    //--------------
    if(number_words > 0 && words[0] != NULL){
        char * targetWord = words[0];
        int len = strlen(targetWord);
        int currentIdx = 0;

        printf(COLOR_GRAY "%s" COLOR_RESET "\r", targetWord);
        fflush(stdout);

        char c;
        while(currentIdx < len){
            int nread = processInputKey(targetWord[currentIdx]);

            if(nread == TIMEOUT) continue;
            if(nread == EXIT) break;

            if(nread == GOT_RIGHT){
                printf(STYLE_CORRECT "%c" , targetWord[currentIdx++]);
            }
            else{
                printf(STYLE_WRONG "%c" , targetWord[currentIdx++]);
            }

            printf(STYLE_TODO);
            fflush(stdout);
        }
    }
    
    sleep(0.5f);
    //AT EXIT
    //----------------------
    for(int i = 0; i < number_words; i++){
        if(words[i] != NULL) free(words[i]);
    }
    free(words);

    printf(COLOR_RESET);
    setBeckgroundColor(COLOR_RESET);
    clearScreen();
    fflush(stdin);

    if(dict)    fclose(dict);
    if(status)  fclose(stats);
    
    return 0;
}

//AUXILIAR FORMATING FUNCTIONS
void removeNL(char * str)
{
    size_t len = strlen(str);
    if(len > 0 && str[len - 1] == '\n')
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
        if(configID == UNLOCKED_CHARS){
            int i;
            for(i = 0; configLine[i] != '\n'; i++)
                unlockedLetters[i] = configLine[i+2];
            unlockedLetters[i] = '\0';
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
    char buffer[3];
    char c;

    int nread = read(STDIN_FILENO, &c, sizeof(c));

    if(nread <= 0)  return TIMEOUT;

    //ESC is ASCII 27
    if(c == CTRL_KEY('q') || c == CTRL_KEY('c') || c == 27){
        return EXIT;
    }

    if(c == rKey) 
        return GOT_RIGHT;
    else 
        return GOT_WRONG;

    return 1;
}