/*
	Программа DAVrun, для загрузки самораспаковывающегося архива через WebDAV с использованием библиотеки "libcurl" в скрытом (фоновом) режиме и запуск его с наивысшими правами.

	Данная программа является свободным программным обеспечением, распространяющимся по лицензии MIT.
	Копия лицензии: https://opensource.org/licenses/MIT

	Copyright (c) 2024 Otto
	Автор: Otto
	Версия: 20.06.24
	GitHub страница:  https://github.com/Otto17/DAVrun
	GitFlic страница: https://gitflic.ru/project/otto/davrun

	г. Омск 2024
*/


//Настройки получения имени ПК (обрезать указанное кол-во символов с начала строки или нет).
//Это нужно если к примеру сначала идёт инвентарный номер, а затем индивидуальное имя компьютера
//К примеру, в начале инвентарный номер (000124-NamePC), а потом имя ПК. Обрезаем первые 7 символов и получаем (NamePC).
#define DEL_SYMBOL_NAME_PC TRUE // Разрешить удаление символов (с начала строки) в имени ПК? TRUE - удалять, FALSE - не удалять
#define NUMBER_SYMBOL 7         // Кол-во удаляемых символов (с начала строки) в имени ПК (игнорируется, если "DEL_SYMBOL_NAME_PC" равен FALSE)

//Настройки WebDAV
#define HOST "https://77.77.77.77:7777/webdav/"           	// Защищённый хост WebDAV (слеш в конце обязателен!)
#define USERNAME "user"										// Логин (ИСПОЛЬЗУЙТЕ СЛОЖНЫЙ, ДЛИННЫЙ ЛОГИН)
#define PASSWD "password"   								// Пароль (ИСПОЛЬЗУЙТЕ СЛОЖНЫЙ, ДЛИННЫЙ ПАРОЛЬ)
#define PATH_DIR "C:\\ProgramData\\DAVrun\\Temp$DEV$RUN"    // Временная папка для загрузки самораспаковывающегося архива и дальнейшей работы с ним
#define DEL_DIR TRUE			                            // Удаление папки "Temp$DEV$RUN" с файлами перед завершением работы программы. TRUE - удалить, FALSE - не удалять
#define POINTS_DIRECTORY "Points"                           // Каталог на сервере, в котором нужно искать название файла с именем компьютера
#define CURL_CRT "C:\\Program Files\\DAVrun\\cert.crt"      // Полный путь до файла с сертификатом открытого ключа (*.crt или *.pem)

//Логирование
#define PATH_LOG "C:\\ProgramData\\DAVrun"  // Путь, куда сохранять лог файл
#define MAX_LOG_SIZE 50000                  // Максимальный размер (в байтах) лог файла для ротации. (Установлено 50 Кбайт)
#define MAX_LOG_FILES 1                     // Максимальное количество лог файлов для хранения. Архивные файлы с суффиксом "_0"


//Подключенные библиотеки
#include <stdio.h>		// Библиотека для определения функций и работы с потоками ввода/вывода
#include <stdlib.h>		// Библиотека предоставляет функции для управления памятью
#include <Windows.h>	// Библиотека, которая предоставляет доступ к API Windows
#include <shellapi.h>   // Библиотека для взаимодействия с оболочкой Windows (Shell), используется в данном случае для "ShellExecuteEx()"
#include <curl/curl.h>  // Библиотека для работы с libcurl
#include <sys/stat.h>   // Библиотека для работы с информацией о файлах и структурами данных, связанными с FS
#include <direct.h>     // Библиотека для работы с файлами и папками FS
#include <time.h>       // Библиотека для работы со временем и датой
#include <string.h>     // Библиотека для работы со строками
#include <stdbool.h>    // Библиотека для возможности использования bool
#include <wincrypt.h>   // Библиотека для работы с криптографией на Windows


#pragma comment(lib, "Crypt32.lib") // Указываем компилятору, что бы добавил библиотеку "Crypt32.lib" в список библиотек, которые нужно связать с исполняемым файлом


//Заглушка
void deleteFilesInFolder(const char *path, bool deleteFolder);


//Функция проверки размера лог файла
long getFileSize(const char *filepath) {
    FILE *fp = fopen(filepath, "r");    // Открываем файл на чтение
    if (fp == NULL) {                   // Проверка, что файл успешно открыт
        return -1;                      // Ошибка открытия файла
    }

    fseek(fp, 0, SEEK_END); // Перемещаем указатель в конец файла
    long size = ftell(fp);  // Получаем текущую позицию указателя файла, что равно размеру файла в байтах
    fclose(fp);             // Закрываем файл
    return size;            // Возвращаем размер файла в байтах
}


//Функция ротации лог файлов
void rotateLogFile(const char *directory, const char *filename) {
    char oldFilePath[128]; // Массив символов для old логов
    char newFilePath[128]; // Массив символов для новых логов

    //Удаление самого старого лога, если их больше "MAX_LOG_FILES"
    snprintf(oldFilePath, sizeof(oldFilePath), "%s/%s_%d", directory, filename, MAX_LOG_FILES - 1); // Формируем путь к самому старому логу
    remove(oldFilePath);    // Удаляем самый старый лог

    //Переименование оставшихся логов
    for (int i = MAX_LOG_FILES - 2; i >= 0; i--) {
        snprintf(oldFilePath, sizeof(oldFilePath), "%s/%s_%d", directory, filename, i);     // Формируем путь к старому файлу лога
        snprintf(newFilePath, sizeof(newFilePath), "%s/%s_%d", directory, filename, i + 1); // Формируем путь к новому файлу лога
        rename(oldFilePath, newFilePath);                                                   // Переименовываем старый лог в новый
    }

    //Архивирование текущего лога в новый файл с "_0" в конце
    snprintf(oldFilePath, sizeof(oldFilePath), "%s/%s", directory, filename);   // Формируем путь к текущему файлу лога
    snprintf(newFilePath, sizeof(newFilePath), "%s/%s_0", directory, filename); // Формируем путь к новому файлу лога с суффиксом "_0"
    rename(oldFilePath, newFilePath);                                           // Переименовываем текущий файл лога в новый файл с суффиксом "_0" (теперь он архивный)
}


//Функция логирования
void writeToLogFile(const char *directory, const char *filename, const char *text) {
    char filepath[128];                                                 // Буфер для полного пути к файлу
    snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename); // Создание полного пути к файлу

    //Проверка размера файла для ротации
    long fileSize = getFileSize(filepath);  // Получаем размер файла
    if (fileSize >= MAX_LOG_SIZE) {         // Если размер файла >= установленному (в байтах)
        rotateLogFile(directory, filename); // Тогда выполняем ротацию
    }

    FILE *fp = fopen(filepath, "a");        // Открытие файла в режиме добавления (добавление лога в конец файла)
    if (fp == NULL) {                       // Проверка, что файл успешно открыт
        printf("File opening error.\n");    // Ошибка открытия файла
        return;                             // Завершаем работу функции
    }

    time_t currentTime;     // Переменная, которая будет содержать текущее время в формате UNIX
    struct tm *localTime;   // Указатель на структуру "tm" под названием "localTime", для хранения и работы с локальным временем
    char timeBuf[80];       // Буфер для времени в текстовом формате

    time(&currentTime);                     // Получаем текущее время
    localTime = localtime(&currentTime);    // Преобразовываем текущее время в локальное время

    strftime(timeBuf, sizeof(timeBuf), "%d.%m.%y в %H:%M:%S", localTime); // Сохраняем в буфер формат времени "дд.мм.гг в чч:мм:сс"

    //Конвертируем время и текст из UTF-8 в Windows-1251 (что бы удобно было просматривать лог в обычном блокноте)
    int bufLength = MultiByteToWideChar(CP_UTF8, 0, timeBuf, -1, NULL, 0);
    wchar_t *wideTimeBuf = (wchar_t *)malloc(bufLength * sizeof(wchar_t));
    //Освобождаем выделенную память в случае ошибки
    if (!wideTimeBuf) {
        fclose(fp);
        return;
    }
    MultiByteToWideChar(CP_UTF8, 0, timeBuf, -1, wideTimeBuf, bufLength);

    bufLength = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    wchar_t *wideText = (wchar_t *)malloc(bufLength * sizeof(wchar_t));
    //Освобождаем выделенную память в случае ошибки
    if (!wideText) {
        free(wideTimeBuf);
        fclose(fp);
        return;
    }
    MultiByteToWideChar(CP_UTF8, 0, text, -1, wideText, bufLength);

    int win1251TimeLength = WideCharToMultiByte(1251, 0, wideTimeBuf, -1, NULL, 0, NULL, NULL);
    char *win1251TimeBuf = (char *)malloc(win1251TimeLength * sizeof(char));
    //Освобождаем выделенную память в случае ошибки
    if (!win1251TimeBuf) {
        free(wideTimeBuf);
        free(wideText);
        fclose(fp);
        return;
    }
    WideCharToMultiByte(1251, 0, wideTimeBuf, -1, win1251TimeBuf, win1251TimeLength, NULL, NULL);

    int win1251Length = WideCharToMultiByte(1251, 0, wideText, -1, NULL, 0, NULL, NULL);
    char *win1251Text = (char *)malloc(win1251Length * sizeof(char));
    //Освобождаем выделенную память в случае ошибки
    if (!win1251Text) {
        free(wideTimeBuf);
        free(win1251TimeBuf);
        free(wideText);
        fclose(fp);
        return;
    }
    WideCharToMultiByte(1251, 0, wideText, -1, win1251Text, win1251Length, NULL, NULL);

    fprintf(fp, "%s: %s", win1251TimeBuf, win1251Text);   // Записываем времени из буфера в файл

    //Чистим память после конвертации
    free(wideTimeBuf);
    free(win1251TimeBuf);
    free(wideText);
    free(win1251Text);

    fclose(fp); // Закрываем файл
}


//Функция получения имени компьютера
char* getNamePC () {
    static char computerName[MAX_COMPUTERNAME_LENGTH + 1];  // Массив размером "MAX_COMPUTERNAME_LENGTH + 1", для хранения имени компьютера
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;               // Размер имени компьютера, инициализируется значением "MAX_COMPUTERNAME_LENGTH + 1"

    if (!GetComputerNameEx(ComputerNamePhysicalDnsHostname, computerName, &size)) {    // Получаем имя компьютера, "size" сообщает, сколько байт было записано в массив
        writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Ошибка при получении имени компьютера.\n"); // Пишем в лог
        return "ErrorGetNamePC";                    // Если не смогли получить имя компьютера, то выходим с названием ошибки
    }

    #if DEL_SYMBOL_NAME_PC == TRUE                  // Если определено как ИСТИНА
        if (size > NUMBER_SYMBOL) {                 // Проверка, чтобы не обрезать строку за её пределы
            return computerName + NUMBER_SYMBOL;    // Возвращаем имя компьютера после "NUMBER_SYMBOL" символа от начала строки
        } else {
            writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Ошибка, имя компьютера слишком короткое, чтобы его можно было сократить.\n");	// Пишем в лог
        }
        #else                                       // Иначе определено как ЛОЖ
            return computerName;                    // Возвращаем полное имя компьютера
        #endif
}


//Пустая функция, чтобы игнорировать вывод данных xml от libcurl (да бы не срать в командную строку лишней информацией)
size_t write_callback(void* ptr, size_t size, size_t nmemb, void* userdata) {   // Принимает указатель на данные, их размер, кол-во элементов и указатель на пользовательские данные
    (void)ptr; (void)userdata;  // Используем (void) для игнорирования переменных "ptr" и "userdata", которые в данной функции не используются
    return size * nmemb;        // Возвращаем произведение "size" на "nmemb" как результат работы функции
}


//Функция для поиска на сервере файла с таким же именем компьютера, как у ПК, на котором запущена данная программа
int checkWebDavFile(const char* host, const char* username, const char* passwd, const char* curlCrt, const char* pointsDirectory, const char* pointsName) {
    CURL* curl;         // Инициализируем указатель на объект типа "CURL"
    CURLcode res;       // Инициализируем переменную типа "CURLcode" для хранения результата выполнения библиотеки libcurl
    long http_code = 0; // Инициализируем переменную "http_code" для хранения кода ответа HTTP

    curl_global_init(CURL_GLOBAL_DEFAULT);  // Инициализируем глобальное состояние библиотеки libcurl
    curl = curl_easy_init();                // Инициализируем указатель "curl" для работы с запросами

    if(curl) {          // Проверяем, что указатель "curl" не равен NULL
        char url[512];  // Символический массив в 512 элементов (с запасом) для хранения строки с URL-адресом

        snprintf(url, sizeof(url), "%s%s/%s", host, pointsDirectory, pointsName);   // Формируем строку с URL-адресом до файла, который будем искать

        //Открывается файл "NUL" для записи информации в никуда (что бы не получать в окно терминала мусорный xml вывод)
        FILE* devnull = fopen("NUL", "w");  // Открываем файл  "NUL" для записи
        if (!devnull) {                     // Проверка на успешное открытие файла
            writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Ошибка открытия файла \"NUL\" на запись, при проверке файла с WebDAV.\n"); // Пишем в лог

            curl_easy_cleanup(curl);    // Освобождаем ресурсы указателя "curl"
            curl_global_cleanup();      // Очищаем глобальное состояние библиотеки libcurl
            return 1;   // Возвращаем единицу, после вывода ошибки и завершаем работу функции
        }

        //Устанавливаем опции
        curl_easy_setopt(curl, CURLOPT_URL, url);                       // Устанавливаем сформированный URL-адрес для проверки 
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);      // Аутентификация посредством дайджеста
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);             // Логин
        curl_easy_setopt(curl, CURLOPT_PASSWORD, passwd);               // Пароль
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PROPFIND");      // Определяем метод "PROPFIND" для HTTP запроса (т.е. будем искать файл по имени)
        curl_easy_setopt(curl, CURLOPT_CAINFO, curlCrt);               // Путь к файлу сертификата
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);  // Отключение вывода xml данных в cmd, посредством перенаправления стандартного вывода в пустую функцию "write_callback"

        res = curl_easy_perform(curl);  // Выполняем запрос с помощью libcurl, результат пишем в "res"

        fclose(devnull);    // Закрываем файл "NULL"

        if(res != CURLE_OK) {   // Если запрос не успешен
            writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Ошибка выполнения запроса при проверке наличия файла с именем компьютера на сервере WebDAV.\n"); // Пишем в лог

            curl_easy_cleanup(curl);    // Освобождаем ресурсы указателя "curl"
            curl_global_cleanup();      // Очищаем глобальное состояние библиотеки libcurl
            return 1;                   // Возвращаем единицу, после вывода ошибки и завершаем работу функции
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);    // Получаем код ответа HTTP

        curl_easy_cleanup(curl);    // Освобождаем ресурсы указателя "curl"
    }
    curl_global_cleanup();  // Очищаем глобальное состояние библиотеки libcur

    if(http_code == 207) {  // Проверяем код ответа HTTP. Если он равен 207 (Multi-Status), функция возвращает 0 (файл присутствует на сервере)
        return 0; // Возвращаем нуль, если нашли нужный файл с именем
    }

    return 1; // Возвращаем единицу, если не нашли нужный файл с именем
}


//Функция проверки хеш-суммы установочного файла
char* checkSHA256(const char *directory, const char *filename) {
    static char hashString[65];
    char filepath[MAX_PATH];
    snprintf(filepath, MAX_PATH, "%s\\%s", directory, filename);

    // Извлечение хеша из имени файла
    char *underscorePos = strrchr(filename, '_');
    if (!underscorePos) {
        deleteFilesInFolder(PATH_DIR, false);
        writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Ошибка извлечения хеша из имени файла, установочный файл удалён.\n");	// Пишем в лог
        return NULL;
    }

    char originalFilename[MAX_PATH];
    strncpy(originalFilename, filename, underscorePos - filename);
    originalFilename[underscorePos - filename] = '\0';

    char providedHash[65];
    strncpy(providedHash, underscorePos + 1, sizeof(providedHash) - 1);
    providedHash[sizeof(providedHash) - 1] = '\0';

    // Переименование файла
    char newFilePath[MAX_PATH];
    snprintf(newFilePath, MAX_PATH, "%s\\%s", directory, originalFilename);
    if (rename(filepath, newFilePath) != 0) {
        deleteFilesInFolder(PATH_DIR, false);
        writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Ошибка переименования установочного файла, установочный файл удалён.\n");	// Пишем в лог
        return NULL;
    }

    // Вычисление SHA-256 хеша переименованного файла
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    BYTE hash[32];
    DWORD hashLen = 32;
    DWORD bytesRead;
    BYTE buffer[1024];
    HANDLE file = CreateFileA(newFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE) {
        writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Не удалось открыть файл для вычисления хеша.\n");	// Пишем в лог
        return NULL;
    }

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        CloseHandle(file);
        writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Не удалось получить доступ к криптопровайдеру.\n");	// Пишем в лог
        return NULL;
    }

    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        CloseHandle(file);
        writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Не удалось создать хеш-объект для алгоритма SHA-256.\n");	// Пишем в лог
        return NULL;
    }

    while (ReadFile(file, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead != 0) {
        if (!CryptHashData(hHash, buffer, bytesRead, 0)) {
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            CloseHandle(file);
            writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Не удалось добавить данные из буфера в хеш-объект при чтении файла (по частям).\n");	// Пишем в лог
            return NULL;
        }
    }

    if (CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
        for (DWORD i = 0; i < hashLen; i++) {
            snprintf(hashString + (i * 2), sizeof(hashString) - (i * 2), "%02x", hash[i]);
        }
    } else {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        CloseHandle(file);
        writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Не удалось получить вычисленные значения хеш-суммы из хеш-объекта.\n");	// Пишем в лог
        return NULL;
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(file);

    // Сравнение хеш-сумм
    if (strncmp(providedHash, hashString, sizeof(providedHash) - 1) != 0) {
        deleteFilesInFolder(PATH_DIR, false);
        writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Хеш-сумма установочного файла не совпадает, установочный файл удалён.\n");	// Пишем в лог
        return NULL;
    }

    return hashString;
}


//Определяем структуру для работы с памятью
struct MemoryStruct {
    char *memory;   // Указатель на память
    size_t size;    // Размер области памяти
};


//Функция для записи данных в память
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) { // Принимает указатель на данные, их размер, количество и указатель на пользовательскую структуру
    size_t realsize = size * nmemb;                             // Вычисляем реальный размер данных через переумножения размера и количества
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;    // Преобразуется указатель "userp" в указатель на структуру "MemoryStruct"

    char *ptr = realloc(mem->memory, mem->size + realsize + 1); // Выделяем память для новой порции данных плюс уже имеющихся данных в структуре MemoryStruct
    if (ptr == NULL) {  // Проверяем, была ли память успешно выделена. Если указатель "ptr" равен "NULL"
        writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Ошибка выделения памяти, освободите ресурсы ОЗУ.\n"); // Пишем в лог
        return 0;   // Возвращаем 0 (означает ошибку, вероятно не хватает памяти для выделения)
    }

    mem->memory = ptr;                                      // Устанавливаем указатель "mem->memory" на адрес, который содержит переменная "ptr" 
    memcpy(&(mem->memory[mem->size]), contents, realsize);  // Копируем содержимое "realsize" в память
    mem->size += realsize;                                  // Увеличиваем размер "mem->size" на "realsize"
    mem->memory[mem->size] = 0;                             // Ставим 0 в ячейке памяти после последнего скопированного символа

    return realsize;    // Возвращаем реальный размер данных
}


//Функция для поиска установочного файла на Сервере по маске (так как название у него будет разное из-за хеш-суммы в имени файла)
char* findSetupFile(const char* host, const char* username, const char* passwd, const char* curlCrt) {  // Возвращает указатель на char
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);   // Выделяем память для структуры в 1 байт
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);  // Инициализируем глобальное состояние библиотеки libcurl
    curl = curl_easy_init();                // Инициализируем указатель "curl" для работы с запросами

    if(curl) {                                  // Проверяем, что указатель "curl" не равен NULL
        char url[512];                          // Символический массив в 512 элементов (с запасом) для хранения строки с URL-адресом
        snprintf(url, sizeof(url), "%sSetup/", host); // Формируем строку с URL-адресом


        curl_easy_setopt(curl, CURLOPT_URL, url);                           // Устанавливаем сформированный URL-адрес
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);          // Аутентификация посредством дайджеста
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);                 // Логин
        curl_easy_setopt(curl, CURLOPT_PASSWORD, passwd);                   // Пароль
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); // Устанавливаем функцию обратного вызова для записи данных
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);          // Данные для записи в файл
        curl_easy_setopt(curl, CURLOPT_CAINFO, curlCrt);                    // Путь к файлу сертификата

        res = curl_easy_perform(curl);  // Выполняем запрос с помощью libcurl, результат пишем в "res"

        if(res != CURLE_OK) {   // Если не запрос не успешен
            writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Ошибка при выполнении запроса на скачивание установочного файла с сервера WebDAV.\n"); // Пишем в лог

            free(chunk.memory);         // Освобождаем память
            curl_easy_cleanup(curl);    // Освобождаем ресурсы указателя "curl"
            curl_global_cleanup();      // Очищаем глобальное состояние библиотеки libcurl
            return NULL;                // Возвращаем нулевой указатель
        }

        //Находим файл, начинающийся на "setup.exe_"
        char* found = strstr(chunk.memory, "setup.exe_");
        
        if (found) {
            char* end = strstr(found, "\"");    // Находим символ "
            if (end) {
                size_t length = end - found;            // Вычисляем длину подстроки между найденными указателями "found" и "end"
                char* filename = malloc(length + 1);    // Выделяем память для строки "filename"
                strncpy(filename, found, length);       // Копируем подстроку в строку
                filename[length] = '\0';                // Добавляем завершающий нуль

                free(chunk.memory);         // Освобождаем память
                curl_easy_cleanup(curl);    // Освобождаем ресурсы указателя "curl"
                curl_global_cleanup();      // Очищаем глобальное состояние библиотеки libcurl
                return filename;            // Возвращаем найденное имя
            }
        }

        free(chunk.memory);         // Освобождаем память
        curl_easy_cleanup(curl);    // Освобождаем ресурсы указателя "curl"
    }

    curl_global_cleanup();  // Очищаем глобальное состояние библиотеки libcurl
    return NULL;            // Возвращаем нулевой указатель
}


//Функция для создания пути (при скачивании файла)
void create_directory_if_not_exists(const char* path) { // Принимает указатель на строку (путь к директории)
    char buffer[128];                                   // Объявляем массив в 128 элементов (с запасом) для хранения буфера
    snprintf(buffer, sizeof(buffer), "%s", path);       // Записываем путь в буфер
    for (char* p = buffer + 1; *p; p++) {               // В цикле "p" пробегает по всем символам строки buffer, начиная со второго символа
        if (*p == '\\' || *p == '/') {                  // Если текущий символ "p" равен символу '' или '/',
            *p = '\0';                                  // то символ "p" заменяется на нуль-терминатор, чтобы обрезать строку до текущей точки пути
            _mkdir(buffer);                             // Создаем промежуточные директории
            *p = '\\';                                  // Восстанавливается символ '' вместо нуль-терминатора
        }
    }
    _mkdir(buffer);  // Создаем конечную директорию, указанную в "path"
}


//Функция загрузки файла с сервера
int downloadFile(const char* host, const char* username, const char* passwd, const char* curlCrt, const char* localDir, const char* filename) {
    CURL *curl;     // Инициализируем указатель на объект типа "CURL"
    FILE *fp;       // Инициализируем указатель на файл
    CURLcode res;   // Инициализируем переменную типа "CURLcode" для хранения результата выполнения библиотеки libcurl

    char url[512];              // Символический массив в 512 элементов (с запасом) для хранения строки с URL-адресом
    char localFilePath[512];    // Символический массив в 512 элементов (с запасом) для хранения пути локальной папки

    create_directory_if_not_exists(localDir);   // Создаём папки по указанному пути

    snprintf(url, sizeof(url), "%sSetup/%s", host, filename);                             // Формируем строку с URL-адресом до удалённого файла, который будем загружать
    snprintf(localFilePath, sizeof(localFilePath), "%s\\%s", localDir, filename);   // Формируем строку до локальной папки, уда будем скачивать

    curl_global_init(CURL_GLOBAL_DEFAULT);  // Инициализируем глобальное состояние библиотеки libcurl
    curl = curl_easy_init();                // Инициализируем указатель "curl" для работы с запросами

    if(curl) {                              // Проверяем, что указатель "curl" не равен NULL
        fp = fopen(localFilePath, "wb");    // Открываем файла для записи в двоичном режиме
        if(fp == NULL) {                    // Проверка на успешное открытие файла
            curl_easy_cleanup(curl);        // Освобождаем ресурсы указателя "curl"
            curl_global_cleanup();          // Очищаем глобальное состояние библиотеки libcurl

            writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Ошибка открытия установочного файла - downloadFile().\n"); // Пишем в лог
            return 1;   // Ошибка открытия установочного файла
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);                   // Устанавливаем сформированный URL-адрес
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);  // Аутентификация посредством дайджеста
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);         // Логин
        curl_easy_setopt(curl, CURLOPT_PASSWORD, passwd);           // Пароль
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);        // Не используем внешнюю функцию для записи данных
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);              // Установка файла, в который будут записываться полученные данные
        curl_easy_setopt(curl, CURLOPT_CAINFO, curlCrt);            // Путь к файлу сертификата

        res = curl_easy_perform(curl);  // Выполняем запрос с помощью libcurl, результат пишем в "res"
        fclose(fp);                     // Закрываем файл

        if(res == CURLE_OK) {           // Если запрос успешен
            curl_easy_cleanup(curl);    // Освобождаем ресурсы указателя "curl"
            curl_global_cleanup();      // Очищаем глобальное состояние библиотеки libcurl
            return 0;                   // Возвращаем путь до локальной папки
        }

        curl_easy_cleanup(curl);    // Освобождаем ресурсы указателя "curl"
    }
    curl_global_cleanup();  // Очищаем глобальное состояние библиотеки libcurl
    writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Не удалось скачать файл - downloadFile().\n"); // Пишем в лог
    return 1;   // Не удалось скачать файл
}


//Функция удаления файла с именем компьютера с сервера
void delWebDavFile(char* delName) {
    CURL *curl;     // Инициализируем указатель на объект типа CURL
    CURLcode res;   // Инициализируем переменную типа "CURLcode" для хранения результата выполнения библиотеки libcurl

    char del_url[512]; // Символический массив в 512 элементов (с запасом) для хранения строки с URL-адресом
    snprintf(del_url, sizeof(del_url), "%s%s/%s", HOST, POINTS_DIRECTORY, delName); // Формируем строку с URL-адресом до файла, который будем удалять

    curl_global_init(CURL_GLOBAL_DEFAULT);  // Инициализируем глобальное состояние библиотеки libcurl
    curl = curl_easy_init();                // Инициализируем указатель "curl" для работы с запросами

    if(curl) {                              // Проверяем, что указатель "curl" не равен NULL
        struct curl_slist *headers = NULL;  // Создаём список заголовков "headers" (это необходимо для корректной настройки HTTP запроса и передачи соответствующих заголовков серверу)
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");    // Добавляем заголовок

        //Устанавливаем опции
        curl_easy_setopt(curl, CURLOPT_URL, del_url);               // Устанавливаем сформированный URL-адрес для запроса
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);  // Аутентификация посредством дайджеста
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);        // Устанавливаем заголовки HTTP для запроса
        curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);         // Логин
        curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWD);           // Пароль
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");    // Определяем метод "DELETE" для HTTP запроса (т.е. будем удалять с сервера)
        curl_easy_setopt(curl, CURLOPT_CAINFO, CURL_CRT);           // Путь к файлу сертификата

        res = curl_easy_perform(curl);  // Выполняем запрос с помощью libcurl, результат пишем в "res"

       if(res != CURLE_OK) writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Ошибка при выполнении запроса на удаление файла с именем компьютера на сервере WebDAV.\n"); // Пишем в лог
        //if(res != CURLE_OK) fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));   // Если не запрос не успешен, то выводим сообщение об ошибке
    
        curl_easy_cleanup(curl);        // Освобождаем ресурсы указателя "curl"
        curl_slist_free_all(headers);   // Освобождаем ресурсы заголовка "headers"
    }

    curl_global_cleanup();  // Очищаем глобальное состояние библиотеки libcurl
}


//Функция для удаления временной папки, либо только содержимого этой папки
void deleteFilesInFolder(const char *path, bool deleteFolder) { // Путь до папки; true - удалять все файлы вместе с папкой, false - удалять только файлы внутри папки
    char command_delete[256]; // Объявляем символический массив

    if (deleteFolder) {
        snprintf(command_delete, sizeof(command_delete), "rmdir /s /q %s", path); // Формируем строку удаления папки вместе с её содержимым
    } else {
        snprintf(command_delete, sizeof(command_delete), "del /f /q %s\\*", path); // Формируем строку удаления содержимого папки
    }

    int result = system(command_delete); // Получаем результат выполнения "system()"

    // Проверяем, успешно ли прошло удаление
     if (result != 0) writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Ошибка при удалении содержимого папки.\n"); // Пишем в лог
}


//Функция верификации скаченного установочного файла и проверки его хеш суммы
int VerificationCycle() {
    char* findFileName = findSetupFile(HOST, USERNAME, PASSWD, CURL_CRT); // Ищем установочный файл на сервере по маске "setup.exe_" и узнаём его полное имя с хешем

    if (findFileName == NULL) { // Если файл не был найден
        writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Не удалось найти установочный файл на сервере.\n");	// Пишем в лог
        return 1; // Не удалось найти файл setup.exe на сервере
    }

    // Если установочный файл был найден на сервере, тогда скачиваем его
    if (downloadFile(HOST, USERNAME, PASSWD, CURL_CRT, PATH_DIR, findFileName) != 0) {	// Если произошла ошибка при загрузке файла
        writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Ошибка при загрузке файла.\n");		// Пишем в лог
        free(findFileName); // Освобождаем память
        return 1; // Возвращаем код ошибки
    }

    char* result = checkSHA256(PATH_DIR, findFileName); // Вычисляем хеш-сумму (SHA-256) скаченного файла и сравниваем с предоставленным хешем в его имени

    if (result) { // Если хеши совпадают
        free(findFileName); // Освобождаем память
        return 0; // Возвращаем 0 (успех)
    } else {
        writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Хеш-сумма установочного файла не совпадает.\n");	// Пишем в лог
        free(findFileName); // Освобождаем память
        return 1; // Возвращаем ошибку
    }
}


int main() {
	//Выполняем код в скрытом (фоновом) режиме
	HWND hWnd = GetConsoleWindow();	// Получаем дескриптор окна консоли, связанного с текущим процессом
	ShowWindow(hWnd, SW_HIDE);		// Скрываем окно консоли, дескриптора "hWnd"

    char* namePC = getNamePC(); // Копируем имя компьютера в "namePC"
    int resultName = checkWebDavFile(HOST, USERNAME, PASSWD, CURL_CRT, POINTS_DIRECTORY, namePC); // Ищем на сервере файл с таким же именем как у этого компьютера

    if(resultName == 0) {   // Если на сервере есть файл с таким же именем, как у этого компьютера
        if (VerificationCycle() == 0) { // И если установочный файл успешно скачали и верифицировали
        
            //Запуск самораспаковывающегося архива с повышенными правами
            char fullPathSetup[MAX_PATH];                                               // Буфер для полного создания пути
            snprintf(fullPathSetup, sizeof(fullPathSetup), "%s\\setup.exe", PATH_DIR);  // Формируем полный путь к файлу самораспаковывающегося архива

            SHELLEXECUTEINFO sei = { sizeof(sei) };                     // Создаём структуру "SHELLEXECUTEINFO" с инициализацией размера
            sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;  // Получаем хэндл процесса и отключаем всплывающие окна в случае ошибок
            sei.lpVerb = "runas";                                       // Запуск будет с повышенными правами
            sei.lpFile = fullPathSetup;                                 // Полный путь к файлу самораспаковывающегося архива
            sei.nShow = SW_HIDE;                                        // Скрываем окно приложения

            if (!ShellExecuteEx(&sei)) {            // Если "ShellExecuteEx()" неуспешно выполнилось
                DWORD dwError = GetLastError();     // Получаем информацию об ошибке
                if (dwError == ERROR_CANCELLED) {   // Выводим информацию об ошибке
                    writeToLogFile(PATH_LOG, "log_DAVrun.txt", "Ошибка при запуске установочного файла - Пользователь отказался от повышения прав.\n"); // Пишем в лог
                } else {
                    //Преобразование сообщения об ошибке от "ShellExecuteEx()", для корректного отображения в кодировке Windows-1251. Получаем сообщение об ошибке
                    char errorMsg[512];
                    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errorMsg, sizeof(errorMsg), NULL); // Форматируем сообщение и сохраняем в "errorMsg"
                    
                    //Преобразуем "errorMsg" из "CP_ACP" в "wchar_t"
                    wchar_t errorMsgW[512];
                    MultiByteToWideChar(CP_ACP, 0, errorMsg, -1, errorMsgW, sizeof(errorMsgW) / sizeof(wchar_t));

                    //Преобразуем "errorMsgW" из "wchar_t" в "UTF-8"
                    char errorMsgUTF8[512];
                    WideCharToMultiByte(CP_UTF8, 0, errorMsgW, -1, errorMsgUTF8, sizeof(errorMsgUTF8), NULL, NULL);

                    //Формируем сообщение лога
                    char logMessage[512];
                    snprintf(logMessage, sizeof(logMessage), "Ошибка при запуске установочного файла - Код ошибки: %lu, Описание: %s", dwError, errorMsgUTF8);

                    writeToLogFile(PATH_LOG, "log_DAVrun.txt", logMessage); // Пишем ошибку в лог

                    deleteFilesInFolder(PATH_DIR, true);	// Удаляем папку с файлами
                }
                return 1;   // Возвращаем 1 в случае ошибки
            }

            // Ожидание завершения процесса
            if (sei.hProcess != NULL) {                         // Проверяем, успешен ли вызов "ShellExecuteEx()"
                WaitForSingleObject(sei.hProcess, INFINITE);    // Если процесс успешно запущен и получен его хэндл, тогда ожидаем завершения процесса
                CloseHandle(sei.hProcess);                      // Освобождаем ресурсы, связанные с хэндлом
            }

            delWebDavFile(namePC);  // Если установка программы завершилась успешно, тогда удаляем с сервера файл с таким же именем, как у этого компьютера

            //Удаление папки со всеми подкаталогами и установочным файлом
            if (DEL_DIR == TRUE) deleteFilesInFolder(PATH_DIR, true);	// Если флаг поднят, то выполняем функцию удаления папки с файлами
        }
    }

    return 0;	// Возвращаем нуль из функции "main()", для обозначения успешного завершения работы программы
}
