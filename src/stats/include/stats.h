#pragma once

#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#define MAX_SAMPLES 50
#define ALPHABET_SIZE 256

typedef struct FileHeader{
    uint32_t ass;
    uint32_t version;
} FileHeader;

typedef struct {
    double samples[MAX_SAMPLES];
    int sampleIndex;
    int count;
    double currentWpm;
    int wrongCount;
    int rightCount;
    double precision;
} CharStats;

extern struct timespec lastKeyPressTime;
extern CharStats charStats[ALPHABET_SIZE];

void registerKeyStats(char c, struct timespec * now);

int saveStats(CharStats * statsArray);

int loadStats(CharStats * statsArray);

void loadGameData();

void updateUnlockedLetters();