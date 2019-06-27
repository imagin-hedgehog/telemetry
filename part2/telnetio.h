#ifndef TELNETIO_H
#define TELNETIO_H

#include <time.h>  // Для time_t
#include <stdio.h> // Для FILE*

#define MAXPARAM 10
// содаём структуру именно в этом файле, чтобы она была видна в остальных
struct measurement{
    int num;                // number – № измерения (011)
    time_t ttime;           // ttime – время измерения (в тиках)
    float quan;             // quantity – величина измерения (0.12345)
    char param[MAXPARAM];   // parameter – имя параметра (ТМ12, ДДН1)
};

void filing_measure(struct measurement *measure);
void printOne(struct measurement *measure);
void printAll(FILE *fPtr);
void Opening_file(FILE **fPtr, const char *mode);
void OpenFile(FILE** fPtr, const char *path, const char *mode);
int CountMeasureFile(FILE *fPtr);
int SizeFileByte(FILE *fPtr);
void SmartCloseFile(FILE *fPtr);

#endif