#include <stdio.h>
#include <conio.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "telnet.h"


/*Открывает файл телеметрии с перехватом ошибок*/
void OpenTelnetFile(FILE** fPtr, const char *path, const char *mode)
// FILE** fPtr Нужен для fopen_s
{
    errno_t err = fopen_s(fPtr, path, mode);
        if (err)
        {   
            // Ошибка при открытии файла!
            puts("File could not be open\n");
            _getch();   //задержка после вывода сообщения на консоль    
            exit(1);            
        }
}

/*Форматный вывод одной структуры*/
void printOne(struct measurement *measure)
{   
    // ctrl+c ctrl+v я повторяю в голове
    char buftime[50];//буфер времени
    char *stime; // строка времени

    // выводим номер структуры
    // 0 - вставлять нули перед числом
    // 3 - ширина поля под число
    printf("[%03d]--------------------------------------\n", measure->num);
    // Параметр и его значение
    // -7 - выравнивае по левому краю
    // всего на имя параметра отводится 7 позиций
    printf("%-7s:   %.3f\n", measure->param, measure->quan);
    // форматируем время
    stime = ctime(&measure->ttime); //перевести время в строку
    strcpy(buftime,stime);
    buftime[20] ='\0';
    // выводим время
    // здесь до двоеточия тоже 7 позиций
    printf("time   :   %s", buftime);
    printf("\n");
}

/*Напечатать все структуры из файла*/
void printAll(FILE *fPtr)
{      
    int i; // переменная цикла
    int c = 0; // счётчик прочитанных структур
    // Вспомогательная буферная структура
    struct measurement measure;
    // количество байт под одну структуру
    int sizeOneMeasureByte = sizeof(struct measurement);
    // количество структур в фале
    int filecount = CountMeasure(fPtr);

    // переходим в начало файла
    fseek(fPtr, 0L, SEEK_SET);
    // и поочерёдно считываем структуры
    for(i=0; i < filecount; i++)
    {   
        c += fread(&measure, sizeOneMeasureByte, 1, fPtr);
        printOne(&measure);
    }

    // если число прочитанных структур не равно количеству
    // этих структур в файле - то это ошибка
    if (c != filecount)
    {
        printf("ERROR : read not all.\n");
        _getch();
        exit(3);
    }

    // Переходим в конец файла
    fseek(fPtr, 0L, SEEK_SET);

}

/*Размер файла в байтах*/
int SizeFileByte(FILE *fPtr)
{
    int size_file;

    fseek(fPtr,0L,SEEK_END);  //перейти в конец файла
    size_file = ftell(fPtr);  //ф-ия ftell() возвращает текущую позицию в файле
    fseek(fPtr, 0L, SEEK_SET); // Возвращаемся в начало файла
    return size_file;
}

/*Количество измерений в файле*/
int CountMeasure(FILE *fPtr)
{
    return SizeFileByte(fPtr)/sizeof(struct measurement);
}

/*Заполнение структуры пользователем */
void filing_measure(struct measurement *measure)
{   
    // хранит адрес символа "\n" в строке параметра
    char* find;

    printf("\tquantity(0.002)      : ");
    scanf_s("%f", &(measure->quan));
    // Фиксируем время
    measure->ttime = time(NULL);

    // для fgets()
    getchar();
    // Получаем имя параметра
    // MAXPARAM - максимальная длина строки для поля параметра
    // это макрос и telnet.h
    printf("\tparametr(DDM1, TM11) : ");
    fgets(measure->param, MAXPARAM, stdin);

    find = strchr(measure->param, '\n');    // поиск новой строки
    if (find)   // если адрес не равен NULL - существует
        // удаляем переход на следующую строку
        *find = '\0';
    
}

/* "умное" закрытие файла */
// Выводит информацию при ошибке
void SmartCloseFile(FILE *fPtr)
{
   if  (fclose(fPtr) == EOF)
    {
      printf("Error closing file\n");//Ошибка при закрытии файла
      _getch();
      exit(6);
    }
}