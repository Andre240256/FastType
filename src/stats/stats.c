#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#include "include/stats.h"
#include "../debug/include/debug.h"

#define FILE_ASS 0x465431
#define FILE_VERSION 3

const char * STATS_FILE = "src/configs/userdata.bin";
const double minimunPrecision = 0.90;
const float minimunWpm = 35.0f;

extern char alphabet[26];
extern char unlockedLetters[27];
struct timespec lastKeyPressTime;
CharStats charStats[ALPHABET_SIZE];

void registerKeyStats(char c, struct timespec * now)
{
    charStats[(int)c].precision = (float)charStats[(int)c].rightCount / (float)(charStats[(int)c].rightCount + charStats[(int)c].wrongCount);

    double delta = (now->tv_sec - lastKeyPressTime.tv_sec) +
                   (now->tv_nsec - lastKeyPressTime.tv_nsec) / 1e9;

    lastKeyPressTime = *now;
    if(delta > 2.0){
        // logDebug("Discarting: Char '%c' for needing %.4f seg to be typed", c, delta);
        return;
    }

    int idx = (unsigned char)c;
    int current_slot = charStats[idx].sampleIndex;

    charStats[idx].samples[current_slot] = delta;
    charStats[idx].sampleIndex = (current_slot + 1) % MAX_SAMPLES;

    if(charStats[idx].count < MAX_SAMPLES) charStats[idx].count++;

    double sum = 0;
    int i;
    for(i = 0; i < charStats[idx].count; i++)
        sum += charStats[idx].samples[i];
    double averageTime = sum / charStats[idx].count;

    
    const float minute = 60.0f;
    const float lettersPerWord = 5.0f;

    if(averageTime > 0.0){
        int wpm = minute / (averageTime * lettersPerWord);
        if(wpm > 300.0)
            charStats[idx].currentWpm = 300.0; 
        else   
            charStats[idx].currentWpm = wpm;
        // logDebug("SUCESSO: Char '%c' (idx %d) | Delta: %.4fs | Avg: %.4fs | WPM: %.2f", 
        //           c, idx, delta, averageTime, charStats[idx].currentWpm);
    }
    // else{
    //     logDebug("ERRO: Avg time zero para '%c'", c);
    // }
}

int saveStats(CharStats * statsArray)
{
    FILE * file = fopen(STATS_FILE, "wb");
    if(!file){
        perror("Erro ao salvar os dados estatísticos");
        return 0;
    }

    FileHeader header = { .ass = FILE_ASS, .version = FILE_VERSION};
    fwrite(&header, sizeof(FileHeader), 1, file);

    size_t written = fwrite(statsArray, sizeof(CharStats), ALPHABET_SIZE, file);
    size_t strSize = fwrite(unlockedLetters, sizeof(char), 27, file);
    fclose(file);

    return (written == ALPHABET_SIZE) && (strSize == 27);
}

int loadStats(CharStats * statsArray)
{
    FILE * file = fopen(STATS_FILE, "rb");

    if(!file)
    {
        logDebug("Arquivo nao existe");
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    FileHeader header;
    if(fread(&header, sizeof(FileHeader), 1, file) != 1) {
        logDebug("Erro no header");
        fclose(file);
        return 0;
    }
    
    if(header.ass != FILE_ASS || header.version != FILE_VERSION)
    {
        fclose(file);
        return 0;
    }

    long expectedSize = sizeof(CharStats) * ALPHABET_SIZE + sizeof(FileHeader) + sizeof(char) * 27;


    if (fileSize != expectedSize) {
        fclose(file);
        return 0;
    }



    size_t read = fread(statsArray, sizeof(CharStats), ALPHABET_SIZE, file);
    size_t strSize = fread(unlockedLetters, sizeof(char), 27 , file);
    fclose(file);

    if (read != ALPHABET_SIZE) {
        return 0;
    }

    return (read == ALPHABET_SIZE) && (strSize == 27);
}

void loadGameData()
{
    if(!loadStats(charStats)){
        memset(charStats, 0, sizeof(CharStats) * ALPHABET_SIZE);
        memset(unlockedLetters, 0, sizeof(char) * 27);
        strcpy(unlockedLetters, "enitrl");
        saveStats(charStats);
    }
}

void updateUnlockedLetters()
{
    int newLetter = 1;
    for(int i = 0; i < (int)strlen(unlockedLetters); i++)
    {
        if(charStats[(int)unlockedLetters[i]].currentWpm < minimunWpm &&
           charStats[(int)unlockedLetters[i]].precision < minimunPrecision){
            logDebug("Precisa melhorar a letra %c\n", unlockedLetters[i]);
            newLetter = 0;
        }
    }

    int size = strlen(unlockedLetters);
    if(newLetter && size < 26){
        unlockedLetters[size] = alphabet[size];
        unlockedLetters[++size] = '\0';
    }

    saveStats(charStats);
}