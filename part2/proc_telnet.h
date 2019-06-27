#ifndef PROC_TELNET_H
#define PROC_TELNET_H

#include <stdio.h>    // Для FILE*
#include "telnetio.h" // для макроса MAXPARAM

// структура Допускового Поля
struct DP
{
    // имя параметра
    char param[MAXPARAM];
    // нижний предел
    double min_q;
    // верхний предел
    double max_q;
};

int isHaveParam(char param[MAXPARAM], const char *name_conf);
void findFluctuations(struct DP *field, FILE* start_file, FILE**finish_file);
struct measurement findMax(char field[MAXPARAM], FILE*fPtr);
struct measurement findMin(char field[MAXPARAM], FILE*fPtr);
int fill_DP(struct DP *field, const char *name_conf);

#endif