#include <stdio.h>
#include <conio.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "telnetio.h"


/*Открывает файл телеметрии с перехватом ошибок*/
void OpenFile(FILE** fPtr, const char *path, const char *mode)
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
    // форматируем время в строку
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
    // Вспомогательная буферная структура
    struct measurement measure;
    // количество байт под одну структуру
    int sizeOneMeasureByte = sizeof(struct measurement);
    // количество структур в фале
    int filecount = CountMeasureFile(fPtr);
    int i; // переменная цикла
    int c = 0; // счётчик прочитанных структур

    // переходим в начало файла
    fseek(fPtr, 0L, SEEK_SET);
    for(i=0; i < filecount; i++)
    {
        // и поочерёдно считываем структуры
        c += fread(&measure, sizeOneMeasureByte, 1, fPtr);
        // И поочерёдно выводим(по одной)
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

    // Переходим в КОНЕЦ файла
    // Вдруг после надо будет дописать ещё измерений - структур
    fseek(fPtr, 0L, SEEK_END);

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
int CountMeasureFile(FILE *fPtr)
{
    return SizeFileByte(fPtr)/sizeof(struct measurement);
}

/*Заполнение структуры пользователем */
void filing_measure(struct measurement *measure)
{   
    // хранит адрес символа "\n" в строке параметра
    char* find;

    printf("\tquantity(0.002)      : ");
    scanf_s("%f", &measure->quan);
    // Фиксируем время
    measure->ttime = time(NULL);

    // для fgets()
    getchar();
    // Получаем имя параметра
    // MAXPARAM - максимальная длина строки для поля параметра
    // это макрос и telnetio.h
    printf("\tparametr(DDM1, TM11) : ");
    fgets(measure->param, MAXPARAM, stdin);

    find = strchr(measure->param, '\n');    // поиск новой строки
    // удаляем переход на следующую строку
    if (find)   // если адрес не равен NULL - значит нашли '\n'
        *find = '\0';
    
}

/*Получение имени файла от пользователя и его открытие*/
void Opening_file(FILE **fPtr, const char *mode)
// Диалоговый режим
// Считываем имя файла от пользователя до тех пор
// Пока он не сможем открыть его
// строка - имя файла
// для fopen_s нужен указаетль на указатель файла FILE **fPtr
{  
    char f_name[30];
    errno_t err;
    for(;;)
    {   
        printf("Enter file name of telnet: ");
        scanf_s("%s", f_name, 30);

  
        //пытаемся открыть файл на чтение
        err = fopen_s(fPtr, f_name, mode);
        if (err)
        {
            printf("Cannot open file: \"%s\"\n", f_name);
            printf("Try again\n");
        }
        else
            break;
    }
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