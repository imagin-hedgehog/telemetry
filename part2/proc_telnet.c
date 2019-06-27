#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <errno.h>
#include "telnetio.h"
#include "proc_telnet.h"

/*имеет ли файл настройки такое допусковое поле*/
/*
    возвращает
        * 0 - если НЕТ такого допускового паля
        * отличное от нуля число - ЕСТЬ такое Доп Поле
*/
// Просто проверяем есть строка param в файле настройки
int isHaveParam(char param[MAXPARAM], const char *name_conf)
{
    // Переменная, в которую будет помещен указатель на созданный
    // поток данных 
    // Указатель на файл настройки
    FILE* confPtr; 
    // Переменная, в которую поочередно будут помещаться 
    // считываемые строки из файла
    char line[50];
    // для fgets - обработка ошибок
    // Указатель, в который будет помещен адрес массива, в который считана 
    // строка, или NULL если достигнут конец файла или произошла ошибка
    char *estr;
    /* Логичская переменная
        * 0 - НЕ НАШЛИ имя параметра в файле
        * отличное от нуля число - НАШЛИ
    */
    int isFind = 0;
    // Открываем файл ностройки с обработкой ошибок
    OpenFile(&confPtr, name_conf, "r");

    //Чтение (построчно) данных из файла в бесконечном цикле
    while(1)
    {   

        // Чтение одной строки  из файла
        estr = fgets(line, sizeof(line), confPtr);

        //Проверка на конец файла или ошибку чтения
        if (estr == NULL)
        {
            // Проверяем, что именно произошло: кончился файл
            // или это ошибка чтения
            if ( feof (confPtr) != 0)
            {  
                //Если файл закончился выходим из бесконечного цикла
                break;
            }
            else
            {
                // Если при чтении произошла ошибка, выводим сообщение 
                // об ошибке и выходим из программы
                printf ("\nERROR: reading conf file : '%s'.\n", name_conf);
                _getch();
                exit(2);
            }
        }

        // Проверяем строку
        // здесь просто проверяем первые strlen(param) символов
        // в строке line
        // если они равны имени параметра 
        if(strncmp(param, line, strlen(param)) == 0)
        {
            isFind = 1;
            // Закрываем файл
            SmartCloseFile(confPtr);
            // выходим из функции с успешным результатом
            return isFind;
        }
    }

    // Достигли конца файла, но такого имени так и не нашли
    // Закрываем файл
    SmartCloseFile(confPtr);
    // Выходим из фукции с плохим результатом
    return isFind;
}

/*Найти отклонения*/
/*
В качестве аргументов принимет
    * field - структуру допускового поля
    * указатель на файл со структурами input_f(входной-обрабатываемый файл)
    * указатель на указатель результирующего файла res_f
    // чтобы изменить значение указателя
// В результате получится файл со структурами, которые не входят в заданный диапазон
*/
void findFluctuations(struct DP *field, FILE* input_f, FILE** res_f)
{
    // Счётчик наёденных структур
    int measureCounter = 0;
    // Вспомогательная буферная переменная
    struct measurement measure;
    // Размер одного измерения в байтах
    int sizeOneMeasureByte = sizeof(measure);
    // количество структур в фале в котором ведётся поиск
    int filecount = CountMeasureFile(input_f);

    int i; // переменная цикла
    int cRead = 0; // счётчик прочитанных структур
    int cWrite = 0; // счётчик записанных структур

    /* 
    Открываем временный файл "Плохих" измерений с уникальным именем
    в режиме "wb+" - чтобы можно было записать структуры,
    а потом их прочитать

    Функция tmpfile_s создает временный двоичный файл, 
    открытый для обновления (режим wb+  — см. fopen).
    Имя файла гарантированно будет отличаться от любого другого
    существующего файла. Временный файл, созданный автоматически, 
    удаляется при закрытии потока функцией fclose
    (или SmartCloseFile - наш вариант дополнили fclose),
    или когда программа завершается нормально.
    
    // errno_t tmpfile_s(FILE** pFilePtr);

    Принимает указатель на указатель файла
    (чтобы изменить значение указателя)
    и возвращает объект типа errno_t.

    обычно используется так:

        errno_t err;  // переменная - ошибка
        FILE *stream; // файловый указатель 

        // Создаём временный файл
        err = tmpfile_s(&stream);
        // И проверяем смогли ли мы его открыть
        if(err)
            perror("Не смогли создать временный файл\n");
        else
            printf("Временный файл успешно создан\n");
    */

    // но мы сделаем легче, без создания лишней переменной
    // сразу в условие передаём функцию
    // и если что-то пошло не так, просто закрываем программу
    // UPD: не забываем res_f - указатель на указатель файла
    //      поэтому НЕ НУЖНЫ звёздочки *res_f или амперсанты &res_f
    if (tmpfile_s(res_f))
    {
        printf("ERROR: Can not open temp file\n");
        _getch();
        exit(2);

    }

    // переходим в начало ОБРАБАТЫВАЕОГО файла(ИСХОДНЫХ структур)
    fseek(input_f, 0L, SEEK_SET);
    // и поочерёдно считываем структуры
    for(i=0; i < filecount; i++)
    {   
        cRead += fread(&measure, sizeOneMeasureByte, 1, input_f);
        // Если измерение имеет такое же имя параметра
        // И значение параметра не входит в пределы допусковых полей
        if (strcmp(measure.param, field->param) == 0 && (measure.quan < field->min_q || measure.quan > field->max_q))
        {   
            // тогда записываем её в результирующий файл
            measureCounter++; // увеличивая число найденных измерений
            // Здесь *res_f - т.к. передать ПРОСТО УКАЗАТЕЛЬ на файл
            cWrite += fwrite(&measure, sizeOneMeasureByte, 1, *res_f);            
        }
    }

    // если число прочитанных структур не равно количеству
    // этих структур в файле - то это ошибка
    if (cRead != filecount)
    {   
        printf("<func::findFluctuations>\n");
        printf("ERROR : read not all.\n");
        _getch();
        exit(3);
    }

    // Если число записанных структур не равно
    // числу найденных структур - то это ошибка
    if (cWrite != measureCounter)
    {   
        printf("<func::findFluctuations>\n");
        printf("ERROR : write not all.\n");
        _getch();
        exit(3);
    }

    // переходим в НАЧАЛО файла ИСХОДНЫХ структур
    fseek(input_f, 0L, SEEK_SET);

    // переходим в НАЧАЛО РЕЗУЛЬТИРУЮЩЕГО файла
    // Файла с найденными ОТЛИЧАЮЩИМИСЯ ОТ НОРМЫ структурами
    // чтобы потом их прочитать
    fseek(*res_f, 0L, SEEK_SET);
}

/*Найти МАКСИМАЛЬНУЮ структуру в файле*/
// Возвращает измерение - структуру с 
// * заданным именем параметра param
// * c МАКСИМАЛЬНЫМ значением поля quan(количесвто)
// Выводит ошибку если изерения с таким именем параметра НЕТ
struct measurement findMax(char param[MAXPARAM], FILE*fPtr)
{
    // Вспомогательная буферная переменная
    // что по одной считывать структуры-измерения из файла
    struct measurement measure;
    // результирущий МАКСИМУМ файла - возвращаемая функцие структура
    struct measurement res_max;
    // Размер одного измерения в байтах
    int sizeOneMeasureByte = sizeof(measure);
    // количество структур в обрабатываемом файле
    int filecount = CountMeasureFile(fPtr);
    // "Логическая переменная"
    // 1 и любое число !=0 - мы встретили структуру в обходе цикла
    // с нужным нам именем параметра первый раз
    // 0 - мы уже встречали структуру с таким именем 
    int isFirst = 1;

    int i; // переменная цикла
    int c = 0; // счётчик прочитанных структур

    // переходим в начало файла
    fseek(fPtr, 0L, SEEK_SET);
    // и поочерёдно считываем структуры
    for(i=0; i < filecount; i++)
    {   
        c += fread(&measure, sizeOneMeasureByte, 1, fPtr);
        // если у структуры нужное нам имя параметра
        if(strcmp(measure.param, param) == 0)
        {   
            // если это первая нам попавшаяся структура
            if(isFirst)
            {
                // делаем её максимумом
                res_max = measure;
                // теперь если встретим другие структуры
                // они уже будут НЕ ПЕРЫВМИ
                isFirst = 0;
            }
            else
            {
                // иначе сравниваем с предыдущим максимумом
                if(res_max.quan < measure.quan)
                {
                    res_max = measure;
                }
            }
        }
    }

    // если число прочитанных структур не равно количеству
    // этих структур в файле - то это ошибка
    if (c!= filecount)
    {   
        printf("<func::findMax>\n");
        printf("ERROR : read not all.\n");
        _getch();
        exit(3);
    }

    // переходим в начало файла структур
    fseek(fPtr, 0L, SEEK_SET);

    // Если мы так и НЕ НАШЛИ ни одной структуры
    // c нужным нам именем параметра - это ошибка
    if (isFirst)
    {
        printf("ERROR: So DP i don't find: '%s' in your file\n", param);
        _getch();
        exit(4);
    }

    // наконец в конце возвращаем найденный МАКСИМУМ
    return res_max;
}

/*Найти МИНИМАЛЬНУЮ структуру в файле*/
// Возвращает измерение - структуру с 
//  * заданным именем параметра param
//  * c МИНИМАЛЬНЫМ значением поля quan(количесвто)
// Выводит ошибку если изерения с таким именем параметра НЕТ
struct measurement findMin(char param[MAXPARAM], FILE*fPtr)
{
    // Вспомогательная буферная переменная
    // что по одной считывать структуры-измерения из файла
    struct measurement measure;
    // результирущий МИНИМУМ файла - возвращаемая функцие структура
    struct measurement res_min;
    // Размер одного измерения в байтах
    int sizeOneMeasureByte = sizeof(measure);
    // количество структур в обрабатываемом файле
    int filecount = CountMeasureFile(fPtr);
    // "Логическая переменная"
    // 1 и любое число !=0 - мы встретили структуру в обходе цикла
    // с нужным нам именем параметра первый раз
    // 0 - мы уже встречали структуру с таким именем 
    int isFirst = 1;
    
    int i; // переменная цикла
    int c = 0; // счётчик прочитанных структур

    // переходим в начало файла
    fseek(fPtr, 0L, SEEK_SET);
    // и поочерёдно считываем структуры
    for(i=0; i < filecount; i++)
    {   
        c += fread(&measure, sizeOneMeasureByte, 1, fPtr);
        // если у структуры нужное нам имя параметра
        if(strcmp(measure.param, param) == 0)
        {   
            // если это первая нам попавшаяся структура
            if(isFirst)
            {
                // делаем её минимумом
                res_min = measure;
                // теперь если встретим другие структуры
                // они уже будут НЕ ПЕРВЫМИ
                isFirst = 0;
            }
            else
            {
                // иначе сравниваем с предыдущим минимумом
                if(res_min.quan > measure.quan)
                {
                    res_min = measure;
                }
            }
        }
    }

    // если число прочитанных структур не равно количеству
    // этих структур в файле - то это ошибка
    if (c!= filecount)
    {   
        printf("<func::findMin>\n");
        printf("ERROR : read not all.\n");
        _getch();
        exit(3);
    }

    // переходим в начало файла структур
    fseek(fPtr, 0L, SEEK_SET);

    // Если мы так и НЕ НАШЛИ ни одной структуры
    // c нужным нам именем параметра - это ошибка
    if (isFirst)
    {
        printf("ERROR: So DP i don't find: '%s' in your file\n", param);
        _getch();
        exit(4);
    }

    // наконец в конце возвращаем найденный МИНИМУМ
    return res_min;
}


/*Конвертируем из строки данные*/
/*
строка line имеет вид :  <имя параметра>.<обозначение предела>:<значение>
    * <имя параметра>       - просто строка ("DDM1", "TM11" и прочие)
    * <обозначение предела> - какой предел нижний или верхний
        Просто строка "MAX" или "MIN"
    * <значение>            - вещественно число(1.25 или 21.3e3)

и вот из этой строки мы должны получить каждое поле по отдельности

param - <имя параметра>
lim   - <обозначение предела>
val   - <значение> предела

например строки line : "DDM1.MAX.100.45" или "TM11.MIN.-99"

в первом случае мы должны получить
*param == "DDM1"
*lim   == "MAX"
*val   == 100.45

ЗАМЕТИМ,что в ЗАГОЛОВКЕ функции используются указатели на указатели
char **param, char **lim - чтобы изменить значение самих указателей на строку

и простые указатели
double *val - чтобы изминить значение передваемой переменной
*/
void convert(char *line, char **param, char **lim, double *val)
{
    // Лучшее описание функции strtok
    // http://all-ht.ru/inf/prog/c/func/strtok.html

    /*

    //вызов функции расчленит как труп нашу стрку line
    strtok(line, sep)

    sep - строка разделителей
    одержит символы, которые разделяют поля в строке line
    <имя параметра>.<обозначение предела>:<значение>
    
    для простоты
    strtok как-бы разделяет строку line по символам из строки sep:
    она поочерёдно проходит строку и если наткнётся на символ,
    входящий в строку sep, то пройденные символы она преобразует в строку
    и возвращает указатель на эту строку.
    Используя её несколько раз, можно получить все подстроки, разделённые
    символами из строки sep
    // Не важно в какой последовательности идут символы в sep
    реализация пусть будет под завесой тайны
    */


    // При первом использовании нужно передать РАЗДЕЛЯЕМУЮ строку
    // получаем имя параметра
    // <ямя параметра> и <обозначение предела> разделены "."
    *param = strtok(line, ".");

    // если хотим найти другие подстроки из строки line
    // то при повторном использовании передаём вместо строки NULL
    // Это сообщит функции, что ищем из предыдущей строки

    // получение обозначение предела
    // <обозначение предела> и <значение предела> разделены ":"
    *lim = strtok(NULL, ":");

    // значение предела
    // функция atof преобразует строку в число типа double
    // в качестве строки передается указатель на строку
    // - возвращаемое значение функции strtok
    *val = atof(strtok(NULL, ":"));
}


/*Заполнить допусковое поле*/
/*
Перед использование поле param
структуры допусковых полей field ДОЛЖНО БЫТЬ ЗАПОЛНЕНО

    используя field->param - 
    мы находим измерение с нужным нам имененем параметра

Возвращает 0 - если успешно заполнили допусковое поле
В остальных случаях проблемы с файлом настройки - поэтому просто
сообщаем об ошибках и завершаем программу

Позже можно добавить возвращение других значений
Чтобы рассказать о возникших проблемах при работе
И не закрывать программу
*/
int fill_DP(struct DP *field, const char *name_conf)
{
    // Переменная, в которую будет помещен указатель на созданный
    // поток данных 
    // Указатель на файл настройки
    FILE* confPtr; 
    // Переменная, в которую поочередно будут помещаться 
    // считываемые строки из файла
    char line[50];
    // для fgets - обработка ошибок
    // Указатель, в который будет помещен адрес массива, в который считана 
    // строка, или NULL если достигнут конец файла или произошла ошибка
    char *estr;
    // Вспомогательная переменная
    // Чтобы найти переход на нувую строку(символ '/n')
    // и удалить его
    char *findSpace;

    // из строки line мы должны получить

    // указатель на строку <имя параметра>
    char *param;
    // указатель на строку <обозначение предела>
    char *lim;
    // значение предела
    double val;

    int haveMax = 0;
    int haveMin = 0;

    // Открываем файл ностройки с обработкой ошибок
    OpenFile(&confPtr, name_conf, "r");

    //Чтение (построчно) данных из файла в бесконечном цикле
    while(1)
    {   

        // Чтение одной строки  из файла
        estr = fgets(line, sizeof(line), confPtr);

        //Проверка на конец файла или ошибку чтения
        if (estr == NULL)
        {
            // Проверяем, что именно произошло: кончился файл
            // или это ошибка чтения
            if ( feof (confPtr) != 0)
            {  
                //Если файл закончился выходим из бесконечного цикла
                break;
            }
            else
            {
                // Если при чтении произошла ошибка, выводим сообщение 
                // об ошибке и выходим из программы
                printf ("\nERROR: reading conf file : '%s'.\n", name_conf);
                _getch();
                exit(2);
            }
        }

        // удаляем переход на следующую строку
        findSpace = strchr(line, '\n');
        // если адрес не равен NULL - нашлии '\n'
        if (findSpace)
            *findSpace = '\0';

        // если строка ПУСТАЯ - то пропускаем и НЕ ОБРАБАТЫВАЕМ ее
        if (strlen(line) == 0)
        {   
            // возвращаясь в начало БЕСКОНЕЧНОГО цикла
            continue;
        }

        // получаем из строки
        /*
            param - Имя параметра
            lim   - модификатор предела - строка "MAX" или "MIN"
            val   - значение предела
        */
        convert(line, &param, &lim, &val);

        // если имена парметров совпадают
        if(strcmp(field->param, param) == 0)
        {
            // заполняем МАКСИМАЛЬНОЕ допустимое значение
            if(strcmp(lim, "MAX") == 0)
            {
                field->max_q = val;
                // Нашли хотябы один МАКСИМУМ
                haveMax = 1;
            }
            // заполняем МИНИМАЛЬНОЕ допустимое значение
            else if((strcmp(lim, "MIN") == 0))
            {
                field->min_q = val;
                // Нашли хотябы один МИНИМУМ
                haveMin = 1;
            }
            // если lim != "MAX" или != "MIN"
            // это ошибка - неверное <обозначение предела>
            else
            {
                printf("ERROR: bad lim mod: '%s'\n", lim);
                printf("Lim mod can be \"MAX\" or \"MIN\"\n");
                printf("parametr: '%s'\n", field->param);
                printf("file    : '%s'\n", name_conf);
                _getch();
                exit(3);
            }
        }
    }

    // Закрываем файл настройки
    SmartCloseFile(confPtr);

    // если не нашли МИНИМУМ - это ошибка
    if (!haveMin)
    {
        printf("ERROR: I not had MIN\n");
        printf("parametr: '%s'\n", field->param);
        printf("file    : '%s'\n", name_conf);
        _getch();
        exit(4);
    }
    // если не нашли МАКСИМУМ - это ошибка
    else if (!haveMax)
    {
        printf("ERROR: I not had MAX\n");
        printf("parametr: '%s'\n", field->param);
        printf("file    : '%s'\n", name_conf);
        _getch();
        exit(4);
    }
    // Если нашли МИНИМУМ и МАКСИМУМ
    // Сообщаем об успешном завершении операции
    else
    {
        return 0;
    }

}