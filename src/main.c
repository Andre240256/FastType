#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

enum configurations{
    UNLOCKED_CHARS = 1
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
char ** loadWords();

void randomIntegers();
int getRandInt(int min, int max);
int isValidWord(const char * str);
void shuffleWords(char ** array);


int main()
{
    if(!startFiles())   return 1;
    if(!startConfigs()) return 1;
    srand(time(NULL));

    char ** words = loadWords();

    for(int i = 0; i < number_words; i++)
        printf("%s\n", words[i]);

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