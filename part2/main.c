#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include "telnetio.h"
#include "proc_telnet.h"

// Макрос имени файла настройки
// Для простоты работы с эти файлом
#define NAME_CONF_FILE "conf_file.txt"

// Прототипы функций
void printFluctuations(struct DP *field, FILE* telnet);
void getParametr(char param[MAXPARAM], const char *name_conf);
int menu_select();


int main()
{
    FILE* telnet_f;
    struct measurement measure;
    struct DP field;
    int choice; // хранит выбранный пункт меню

    // получаем имя файла от пользователя и открываем его
    Opening_file(&telnet_f, "rb");

    /*Интерактивное меню*/

    // бесконечный цикл - получения комманд от пользователя
    for(;;)
    {
        choice = menu_select();
        switch(choice)
        {
            
            case 1:
                // Печатаем весь файл
                printf("\n\n");
                printf("------------------Your_telnet_file--------------\n");
                // Если файл пуст то пишем что он пуст
                if (CountMeasureFile(telnet_f) == 0)
                {
                    printf("At that time file is EMPTY\n");
                }
                else
                {   
                    // процедура печатает все структуры из файла
                    printAll(telnet_f);
                }

                // Задержка перед выходом
                printf("\nPress [Enter] to return menu\n");
                _getch();
                break;
            case 2:
                // Получаем имя параметра от пользователя
                // до тех по пока он не введёт такое имя,
                // которое есть в файле настройке
                getParametr(field.param, NAME_CONF_FILE);
                // Заполняем допусковое поля значениями
                // из файла настройки
                fill_DP(&field, NAME_CONF_FILE);
                // Выводим все отклонения от допустимых
                printFluctuations(&field, telnet_f);
                break;
            case 3:
                // Получаем имя параметра от пользователя
                // до тех по пока он не введёт такое имя,
                // которое есть в файле настройке
                getParametr(field.param, NAME_CONF_FILE);

                // Находим МИНИМУМ
                measure = findMin(field.param, telnet_f);
                // Выводи его
                printf("\nThe MIN quantity\n");
                printOne(&measure);

                // Находим МАКСИМУМ
                measure = findMax(field.param, telnet_f);
                // Выводи его
                printf("\nThe MAX quantity\n");
                printOne(&measure);

                // Задержка перед выходом
                printf("\nPress [Enter] to return menu\n");
                _getch();

                break;
            case 4:
                // Перед выходом нужно закрыть файл телеметрии
                SmartCloseFile(telnet_f);
                exit(0);  // выход
        }
    }


    _getch(); // задержка перед выходом из программы

    return 0;
}

/*Интерактивное меню*/
// возвращает номер выбранного пункта
int menu_select()
{
    int c; // ответ от пользователя

    // Выводим само меню
    printf("\n\n");
    printf("\t1 - Print all measures from file\n");
    printf("\t2 - Find Fluctuations\n");
    printf("\t3 - Find limits\n");
    printf("\t4 - Exit\n");

    // получаем выбор пункта меню до тех пор
    // пользователь вводит некоректный ответ
    do
    {
        printf("\nEnter number: ");
        scanf_s("%d", &c, 1);
    } while(c <1 || c>4);

    return c;
}

/*Вывести отклонения*/
void printFluctuations(struct DP *field, FILE* telnet_f)
{
    // указатель на результирующий файл
    FILE* res_f;

    // создаём, открываем, и заполняем этот результирующий файл
    findFluctuations(field, telnet_f, &res_f);

    // Пояснительное сообщение
    printf("\nFluctuations parametr of '%s':\n", field->param);
    /*выводим измерения, отличающиеся от нормы*/
    // Если файл отклонений пуст то пишем, что нет "плохих измерений"
    if (CountMeasureFile(res_f) == 0)
    {
        printf("So measure I do not find!!!\n");
    }
    else
    {   // Пояснительное сообщение c выводом допустимых пределов
        printf("This quantities < MIN=%.3f or > MAX=%.3f\n", field->min_q, field->max_q);
        // Выводим все найденные измерения
        printAll(res_f);
    }

    // Закрываем результирующий файл
    SmartCloseFile(res_f);

    printf("\nPress [Enter] to return menu\n");
    _getch();
}

/*Получить имя параметра*/
void getParametr(char param[MAXPARAM], const char *name_conf)
{
    while(1)
    {
        printf("Enter name of parametr: ");
        scanf_s("%s", param, MAXPARAM);

        // если нашли такое имя выходим из цикла с успехом
        if (isHaveParam(param, name_conf))
        {
            return;
        }
        // иначе выводи сообщение о неверном вводе
        else
        {
            printf("Such parametr '%s' not have\n", param);
            printf("File: '%s'\n", name_conf);
            printf("Try again(press [Enter] to continue)\n");
            _getch();
        }
    }
}