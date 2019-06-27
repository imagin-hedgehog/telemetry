#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <conio.h>
#include "telnet.h"

int ContinueOrExit();



int main()
{   
    int usr_answer;
    // Указатель на файл телеметрии
    FILE* fPtr;
    // количество записей в файле на текщий момент
    int filecount;
    // количество ихмерений сделанных пользователем за текущую сессию
    int count = 0;
    // счётчик записей в файл
    int c = 0;
    // будем работать без массва структур, тк много памяти
    // но если адо то это легко реализовать
    // буферная-вспомогательная переменная - для записи и чтения данных из файла
    struct measurement tmp_telnet;
    // Размер одной структуры в байтах
    int sizeOneMeasureByte = sizeof(tmp_telnet);


    // открываем файл телеметриии
    OpenTelnetFile(&fPtr, "telnet_1.dat", "a+b");
    // узнаём сколько в нём измерений
    filecount = CountMeasure(fPtr);

    // если первое включение программы и telnet_1.dat - пуст
    if (filecount == 0)
    {   
        // Введите номер первого измерения
        printf("Enter the FIRST measure NUMBER(0321): ");
        scanf_s("%d", &tmp_telnet.num);
        getchar();  // очистить входную строку нужно при первом запуске
    }
    else
    {   
        // Если в файле есть уже измерения, то идём в файл
        // и берём оттуда номер последнего измерения
        // и увеличиваем его на еденицу

        // ставим указатель ПЕРЕД последней структурой,
        // чтобы её прочитать
        fseek(fPtr, -sizeOneMeasureByte, SEEK_END);
        fread(&tmp_telnet, sizeOneMeasureByte, 1, fPtr);
        fseek(fPtr,0L,SEEK_END);    // возвращаемся в конец файла    
        tmp_telnet.num++;
    }
    
    // Призыв начать что-то делать в своей жизни
    printf("Let's Go\n");
    
    while(1)
    {   
        // Если нажали Ctr+z - Exit
        usr_answer = ContinueOrExit();
        if (usr_answer == 0)
        {
            // Если в этой сессии ничего не записали
            if (count == 0)
            {
                printf("Good job. I like doing nothing, too.\n");
                _getch();
            }

            // Печатаем весь файл
            printf("\n\n");
            printf("------------------telnet_1.dat--------------\n");
            // Если файл пуст то пишем что он пуст
            if (filecount == 0)
            {
                printf("At that time file is EMPTY\n");
            }
            else
            {   
                // процедура печатает все структуры из файла
                printAll(fPtr);
            }

            // Закрываем файл
            SmartCloseFile(fPtr);

            // Выходим из цикла - завершаем работу - программы
            break;
        }

            /*меню заполнения полей*/
        // Красиво отделяем одни измерения от других
        printf("\n---------------------------------------\n");
        // Выводим номер измерения
        printf("\tNumber measure       : %d\n", tmp_telnet.num);

        // Получаем данные измерения от пользователя
        filing_measure(&tmp_telnet);

        // Сохраняем структуру в файл
        // С проверкой на запись
        // так ещё удобно считать сколько структур
        // сохранили в файл за сессию
        c += fwrite(&tmp_telnet, sizeOneMeasureByte, 1, fPtr);
        count++;

        // если счётчик записей не равен числу обработанных
        // структур за эту сессию - это ошибка
        if (c != count)
        {
            printf("ERROR: Do not write messure in file\n");
            _getch();
            exit(2);
        }

        // счётчик количество структур в файле
        // может пригодиться если нужно
        // ограничить число записей в файле
        filecount++;

        // Увеличиваем номер измерения - для следующего измерения
        tmp_telnet.num++;
    }


    // мы вне цикла - значит нажали ctrl+z
    //Сообщение при выходе
    printf("Press [Enter] to exit: ");
    // задержка перед выходом
    _getch();

    return 0;
}

/*Продолжить или выйти из программы*/
// Функция в диалоговом режиме спрашивает у пользователя
// Хочет ли он продолжать записывать измерения
// Или выйти из программы
/*
Возвращает
    1 - если продолжить
    0 - если выйти
*/
int ContinueOrExit(int cls)
{
    printf("_________________________________________\n");
    printf("Press [Enter] to continue\n");
    printf("or [CTRL+Z] and after [Enter] to exit: ");
    if (getchar()!=EOF)
    {
        return 1;
    }
    else
    {
        // Конец файла - EOF - Ctrl+Z
        return 0;
    }
           
}
