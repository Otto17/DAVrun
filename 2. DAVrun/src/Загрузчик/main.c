/*
	Программа DAVrun, для загрузки самораспаковывающегося архива через WebDAV с использованием библиотеки "libcurl" в скрытом (фоновом) режиме и запуск его с наивысшими правами.

    Использование: DAVrun.exe "Хост" "Логин" "Пароль" "Имя сертификата с расширением" "Каталог ожидания установок на сервере" "Кол-во символов (число от 0 до 14) для обрезки символов от начала имени ПК".
    Для удаления конфига использовать: DAVrun.exe "Delete Config в любом регистре".

    Пример: DAVrun.exe "https://77.77.77.77:7777/webdaw/" "User" "Password" "cert.crt" "Points" 7
    Пример: DAVrun.exe "DELETE CONFIG"

	Данная программа является свободным программным обеспечением, распространяющимся по лицензии MIT.
	Копия лицензии: https://opensource.org/licenses/MIT

	Copyright (c) 2024 Otto
	Автор: Otto
	Версия: 13.07.24
	GitHub страница:  https://github.com/Otto17/DAVrun
	GitFlic страница: https://gitflic.ru/project/otto/davrun

	г. Омск 2024
*/


//НАСТРОЙКИ
//Подключаем код для шифрования конфиг файла (нужное раскомментировать)
//#include "soft_Encryption.h"        // Шифрование без привязки к ПК (безопасность ниже, но конфиг можно переносить на другие компьютеры)
#include "strong_Encryption.h"    // Шифрование c привязкой к ПК (безопасность на много выше, но конфиг НЕ переносим на другие компьютеры)

//Данный блок будет доступен, если раскомментирована "soft_Encryption.h"
#ifdef SOFT_ENCRYPTION_H
const char *hardcoded_password = "mLAQKnTNuCShHREbpvyEELhxnK2eaejH";    // Жёстко закодированный пароль (ОБЯЗАТЕЛЬНО СГЕНЕРИРОВАТЬ СВОЙ ЧЕРЕЗ ПРОГРАММУ "Keygen_Pepper_Hard_Passwd.exe")
const char *pepper = "z79xciXAatZBwtRZ";                                // Поперчить пароль             (ОБЯЗАТЕЛЬНО СГЕНЕРИРОВАТЬ СВОЙ ЧЕРЕЗ ПРОГРАММУ "Keygen_Pepper_Hard_Passwd.exe")
#endif

//Папка куда будут скачиваться самораспаковывающиеся архивы
#define PATH_DIR "C:\\ProgramData\\DAVrun\\Temp$DEV$RUN"    // Временная папка для загрузки самораспаковывающегося архива и дальнейшей работы с ним
#define DEL_DIR TRUE			                            // Удаление папки "Temp$DEV$RUN" с файлами перед завершением работы программы. TRUE - удалить, FALSE - не удалять

//Логирование
#include "logFile.h"    // Подключаем файл лога

const char *path_Log = "C:\\ProgramData\\DAVrun";   // Путь, куда сохранять лог файл
const int max_Log_Size = 300000;                    // Максимальный размер (в байтах) лог файла для ротации. (Установлено 300 Кбайт)
const int max_Log_Files = 2;                        // Максимальное количество лог файлов для хранения. Архивные файлы с суффиксом "_0", "_1"...

//Подключенные библиотеки
#include <stdio.h>		        // Библиотека для определения функций и работы с потоками ввода/вывода
#include <stdlib.h>		        // Библиотека предоставляет функции для управления памятью
#include <Windows.h>	        // Библиотека, которая предоставляет доступ к API Windows
#include <shellapi.h>           // Библиотека для взаимодействия с оболочкой Windows (Shell), используется в данном случае для "ShellExecuteEx()"
#include <curl/curl.h>          // Библиотека для работы с libcurl
#include <sys/stat.h>           // Библиотека для работы с информацией о файлах и структурами данных, связанными с FS
#include <direct.h>             // Библиотека для работы с файлами и папками FS
#include <string.h>             // Библиотека для работы со строками
#include <wincrypt.h>           // Библиотека для работы с криптографией на Windows
#include <libxml/parser.h>      // Библиотека для работы с XML-парсингом. Позволяет разбирать XML-документы и работать с их структурой
#include <libxml/tree.h>        // Библиотека специализируется на работе с древовидными структурами XML. Предоставляет функционал для работы с деревьями DOM-структур XML-документов
#include <unicode/ustdio.h>     // Библиотека обеспечивающая поддержку Unicode форматов при вводе и выводе данных
#include <unicode/ustring.h>    // Библиотека для работы с строками в формате Unicode


#pragma comment(lib, "Crypt32.lib") // Указываем компилятору, что бы добавил библиотеку "Crypt32.lib" в список библиотек, которые нужно связать с исполняемым файлом (для вычисления хеш-суммы)


//Заглушка
void deleteFilesInFolder(const char *path, bool deleteFolder);

//Функция получения имени компьютера
char* getNamePC (unsigned int numberSymbol) {               // Принимает число, для обрезания кол-ва символов от начала строки
    static char computerName[MAX_COMPUTERNAME_LENGTH + 1];  // Массив размером "MAX_COMPUTERNAME_LENGTH + 1", для хранения имени компьютера
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;               // Размер имени компьютера, инициализируется значением "MAX_COMPUTERNAME_LENGTH + 1"

    if (!GetComputerNameEx(ComputerNamePhysicalDnsHostname, computerName, &size)) {             // Получаем имя компьютера, "size" сообщает, сколько байт было записано в массив
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка при получении имени компьютера.\n"); // Пишем в лог
        return "ErrorGetNamePC";                                                                // Если не смогли получить имя компьютера, то выходим с названием ошибки
    }

    if (size > numberSymbol) {              // Проверка, чтобы не обрезать строку за её пределы
        return computerName + numberSymbol; // Возвращаем имя компьютера после "numberSymbol" символа от начала строки
    } else {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка, имя компьютера слишком короткое, чтобы его можно было сократить.\n");   // Пишем в лог
        return "ErrorGetNamePC";    // Возвращаем ошибку, если имя слишком короткое
    }
}


//Структура для работы с памятью
struct MemoryStruct {
    char *memory;   // Указатель на память
    size_t size;    // Размер области памяти
};


//Функция для записи данных в память
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) { // Принимает указатель на данные, их размер, количество и указатель на пользовательскую структуру
    size_t realsize = size * nmemb;                             // Вычисляем реальный размер данных через перемножения размера и количества
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;    // Преобразуется указатель "userp" в указатель на структуру "findSetupFile_MemoryStruct"

    char *ptr = realloc(mem->memory, mem->size + realsize + 1); // Выделяем память для новой порции данных плюс уже имеющихся данных в структуре findSetupFile_MemoryStruct
    if (ptr == NULL) {                                          // Проверяем, была ли память успешно выделена. Если указатель "ptr" равен "NULL"
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка выделения памяти, освободите ресурсы ОЗУ.\n"); // Пишем в лог
        return 0;   // Возвращаем 0 (означает ошибку, вероятно не хватает памяти для выделения)
    }

    mem->memory = ptr;                                      // Устанавливаем указатель "mem->memory" на адрес, который содержит переменная "ptr" 
    memcpy(&(mem->memory[mem->size]), contents, realsize);  // Копируем содержимое "realsize" в память
    mem->size += realsize;                                  // Увеличиваем размер "mem->size" на "realsize"
    mem->memory[mem->size] = 0;                             // Ставим 0 в ячейке памяти после последнего скопированного символа

    return realsize;    // Возвращаем реальный размер данных
}


//Функция для декодирования URL-кодированной строки в UTF-8
void urlDecode(char *dst, const char *src) {    // Принимает указатель на массив символов "*dst" и константный указатель на массив символов "*src"
    char a, b;
    while (*src) {              // Крутимся в цикле, пока указатель не укажет на конец строки
        if ((*src == '%') &&    // Проверка условия, что текущий символ "*src" равен % и следующие два символа "src[1]" и "src[2]" существуют и являются шестнадцатеричными цифрами
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a' - 'A';   // Преобразуем символы "a" и "b" к верхнему регистру
            if (a >= 'A') a -= ('A' - 10);  // Преобразуем символы "a" и "b" из ASCII-кода в числовое значение
            else a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16 * a + b;   // Раскодирование символов "a" и "b" из формата URL-encoded в символ и записываем его в "*dst"
            src += 3;              // Перемещаем указатель "src" на 3 символа вперед
        } else {
            *dst++ = *src++;    // В случае, если условие на шестнадцатеричные цифры не выполняется, копируем текущий символ из "*src" в "*dst" с инкрементацией на +1 "*src" и "*dst"
        }
    }
    *dst++ = '\0';  // Добавляем завершающий ноль в коней строки
}


//Функция для преобразования строки в нижний регистр
void toLowerCase(char *dst, const char *src) {  // Указателем на строку нижнего регистра "*dst" и константным указателем на строку исходного текста "*src"
    UErrorCode status = U_ZERO_ERROR;           // Объявляем переменную "status" типа "UErrorCode" и присваиваем ей значение "U_ZERO_ERROR"
    UChar uSrc[512];
    UChar uDst[512];

    //Конвертация UTF-8 в UTF-16
    u_strFromUTF8(uSrc, 512, NULL, src, -1, &status);   // Передаём массив "uSrc" размером 512, указатель на исходную строку src, значение -1 для определения длины строки и переменная status для хранения статуса операции
    if (U_FAILURE(status)) {    // Проверка конвертации
        //Формируем сообщение лога
        char logMsg[512];
        snprintf(logMsg, sizeof(logMsg), "Ошибка конвертации UTF-8 в UTF-16 - описание ошибки: %s\n", u_errorName(status));
        writeToLogFile(path_Log, "log_DAVrun.txt", logMsg); // Пишем ошибку в лог
        return;
    }

    //Преобразование в нижний регистр
    u_strToLower(uDst, 512, uSrc, -1, NULL, &status);
    if (U_FAILURE(status)) {
        //Формируем сообщение лога
        char logMsg[512];
        snprintf(logMsg, sizeof(logMsg), "Ошибка преобразования в нижний регистр - описание ошибки: %s\n", u_errorName(status));
        writeToLogFile(path_Log, "log_DAVrun.txt", logMsg); // Пишем ошибку в лог
        return;
    }

    //Конвертация UTF-16 обратно в UTF-8
    u_strToUTF8(dst, 512, NULL, uDst, -1, &status);
    if (U_FAILURE(status)) {
       //Формируем сообщение лога
        char logMsg[512];
        snprintf(logMsg, sizeof(logMsg), "Ошибка конвертации UTF-16 в UTF-8 - описание ошибки: %s\n", u_errorName(status));
        writeToLogFile(path_Log, "log_DAVrun.txt", logMsg); // Пишем ошибку в лог
        return;
    }
}


//Функция для поиска файла по частичному совпадению
char* checkWebDavFile(const char *host, const char *username, const char *passwd, const char *curlCrt, const char *pointsDirectory, const char *pointsName) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;  // Структура для работы с памятью

    chunk.memory = malloc(1);   // Начальная память для ответа
    chunk.size = 0;             // Текущий размер ответа

    curl_global_init(CURL_GLOBAL_DEFAULT);  // Инициализируем глобальное состояние библиотеки libcurl
    curl = curl_easy_init();                // Инициализируем указатель "curl" для работы с запросами

    if(curl) {          // Проверяем, что указатель "curl" не равен NULL
        char url[512];  // Символический массив в 512 элементов (с запасом) для хранения строки с URL-адресом

        snprintf(url, sizeof(url), "%s%s", host, pointsDirectory);  // Формируем строку с URL-адресом

        curl_easy_setopt(curl, CURLOPT_URL, url);                           // Устанавливаем сформированный URL-адрес
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);          // Аутентификация посредством дайджеста
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);                 // Логин
        curl_easy_setopt(curl, CURLOPT_PASSWORD, passwd);                   // Пароль
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PROPFIND");          // Отправляем HTTP запрос методом "PROPFIND", для запроса свойств ресурса на сервере
        curl_easy_setopt(curl, CURLOPT_CAINFO, curlCrt);                    // Путь к файлу сертификата
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); // Устанавливаем функцию обратного вызова для записи данных
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);          // Записываем результат запроса в структуру "chunk"
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);                 // Следование перенаправлениям

        struct curl_slist *headers = NULL;                      // Создаём указатель на структуру "curl_slist" и инициализируем "NULL"
        headers = curl_slist_append(headers, "Depth: 1");       // Добавляем заголовок "Depth: 1" в список заголовков
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);    // Устанавливаем список заголовков "headers" для вызова "curl"

        res = curl_easy_perform(curl);  // Выполняем запрос с помощью libcurl, результат пишем в "res"

        if(res != CURLE_OK) {   // Если запрос не успешен
            //Формируем сообщение лога
            char logMsg[512];
            snprintf(logMsg, sizeof(logMsg), "Ошибка выполнения функции поиска файла на сервере - описание ошибки: %s\n", curl_easy_strerror(res));
            writeToLogFile(path_Log, "log_DAVrun.txt", logMsg); // Пишем ошибку в лог
        } else {
            xmlDocPtr doc = xmlReadMemory(chunk.memory, chunk.size, "noname.xml", NULL, 0); // Создаём указатель на структуру "xmlDoc" и присваиваем ему значение, полученное в результате чтения XML-данных из памяти
            if (doc == NULL) {  // Если указатель "doc" равен "NULL"
                writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка, не удалось разобрать XML.\n"); // Пишем в лог
            } else {
                xmlNode *root_element = xmlDocGetRootElement(doc);  // Создаём указатель на корневой элемент XML-документа
                xmlNode *cur_node = NULL;
                for (cur_node = root_element->children; cur_node; cur_node = cur_node->next) {                                  // Запускаем цикл перебора дочерних элементов корневого элемента
                    if (cur_node->type == XML_ELEMENT_NODE && xmlStrcmp(cur_node->name, (const xmlChar *)"response") == 0) {    // Проверяем, является ли текущий узел элементом XML и имеет ли он имя "response"
                        xmlNode *child_node = cur_node->children;                                                               // Получаем указатель на первый дочерний узел текущего узла
                        
                        while (child_node) {                                                                                            // Запускаем цикл перебора дочерних узлов текущего узла
                            if (child_node->type == XML_ELEMENT_NODE && xmlStrcmp(child_node->name, (const xmlChar *)"href") == 0) {    // Проверяем, является ли текущий дочерний узел элементом XML и имеет ли он имя "href"
                                xmlChar *content = xmlNodeGetContent(child_node);                                                       // Получаем содержимое текущего дочернего узла
                               
                                if (content) {
                                    char decoded_filename[512];
                                    urlDecode(decoded_filename, (const char *)content); // Если содержимое существует, декодируем его и находим имя файла
                                    char *filename = strrchr(decoded_filename, '/');    // Ищем последнее вхождение символа '/' в строке "decoded_filename" и возвращаем указатель на этот символ (или NULL, если символ не найден)
                                    
                                    //Сравниваем имя файла с именем точки "pointsName" после приведения обоих к нижнему регистру
                                    if (filename) {
                                        filename++; // Пропустить '/'
                                        char lower_filename[512];
                                        char lower_pointsName[512];
                                        toLowerCase(lower_filename, filename);
                                        toLowerCase(lower_pointsName, pointsName);

                                        //Если имена файлов совпадают или содержат точное совпадение со значением "pointsName" после знака "=", то возвращаем имя файла
                                        if (strcmp(lower_filename, lower_pointsName) == 0 ||
                                            (strchr(lower_filename, '=') && strcmp(strchr(lower_filename, '=') + 1, lower_pointsName) == 0)) {
                                            char *result = strdup(filename);
                                            xmlFree(content);               // Освобождаем память XML
                                            xmlFreeDoc(doc);                // Освобождаем память XML
                                            curl_easy_cleanup(curl);        // Освобождаем ресурсы указателя "curl"
                                            curl_global_cleanup();          // Очищаем глобальное состояние библиотеки libcurl
                                            curl_slist_free_all(headers);   // Освобождаем ресурсы заголовка "headers"
                                            free(chunk.memory);             // Освобождаем память
                                            return result;                  // Возвращаем имя файла
                                        }
                                    }
                                    xmlFree(content);   // Освобождаем память XML
                                }
                            }
                            child_node = child_node->next;  // Передаём указатель на следующий узел после узла, на который указывает указатель "child_node"
                        }
                    }
                }
                xmlFreeDoc(doc);    // Освобождаем память XML
            }
        }
        curl_easy_cleanup(curl);        // Освобождаем ресурсы указателя "curl"
        curl_slist_free_all(headers);   // Освобождаем ресурсы заголовка "headers"
    }

    curl_global_cleanup();  // Очищаем глобальное состояние библиотеки libcurl
    free(chunk.memory);     // Очищаем память

    return "NULL"; // Возвращаем 'Файл не найден'
}


//Функция проверки хеш-суммы установочного файла
char* checkSHA256(const char *directory, const char *filename) {
    static char hashString[65];
    char filepath[MAX_PATH];
    snprintf(filepath, MAX_PATH, "%s\\%s", directory, filename);    // Записываем путь к файлу в массив "filepath"

    //Извлечение хеша из имени файла
    char *underscorePos = strrchr(filename, '_');   // Извлекаем позицию подчеркивания в имени файла с помощью функции "strrchr()"

    //Если подчеркивание не найдено, тогда удаляем все файлы в каталоге
    if (!underscorePos) {
        deleteFilesInFolder(PATH_DIR, false);   // Путь до папки; true - удалять все файлы вместе с папкой, false - удалять только файлы внутри папки
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка извлечения хеша из имени файла, установочный файл удалён.\n");	// Пишем в лог
        return NULL;
    }

    //В массив "originalFilename" копируем подстроку имени файла до символа подчеркивания и добавляем завершающий символ
    char originalFilename[MAX_PATH];
    strncpy(originalFilename, filename, underscorePos - filename);
    originalFilename[underscorePos - filename] = '\0';

    //В массив "providedHash" копируем подстроку имени файла, начиная с символа после подчеркивания, с учетом размера "providedHash" и добавляем завершающий символ
    char providedHash[65];
    strncpy(providedHash, underscorePos + 1, sizeof(providedHash) - 1);
    providedHash[sizeof(providedHash) - 1] = '\0';

    //Переименование файла
    char newFilePath[MAX_PATH];
    snprintf(newFilePath, MAX_PATH, "%s\\%s", directory, originalFilename); // Формируем строку с новым именем файла

    if (rename(filepath, newFilePath) != 0) {   // Если не смогли переименоватьфайл, тогда удаляем все файлы в каталоге
        deleteFilesInFolder(PATH_DIR, false);   // Путь до папки; true - удалять все файлы вместе с папкой, false - удалять только файлы внутри папки
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка переименования установочного файла, установочный файл удалён.\n");	// Пишем в лог
        return NULL;
    }

    //Вычисление SHA-256 хеша переименованного файла
    HCRYPTPROV hProv = 0;   // Создаём криптопровайдер "hProv"
    HCRYPTHASH hHash = 0;   // Создаём хеш-объект "hHash" для алгоритма SHA-256
    BYTE hash[32];
    DWORD hashLen = 32;
    DWORD bytesRead;
    BYTE buffer[1024];
    HANDLE file = CreateFileA(newFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);    // Открываем файл "file" для чтения в бинарном режиме

    if (file == INVALID_HANDLE_VALUE) { // Если не смогли открыть файл
        writeToLogFile(path_Log, "log_DAVrun.txt", "Не удалось открыть файл для вычисления хеша.\n");	// Пишем в лог
        return NULL;
    }

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {  // Если не удалось получить доступ к криптопровайдеру
        CloseHandle(file);                                                              // Закрываем файловый дескриптор (освобождаем память)
        writeToLogFile(path_Log, "log_DAVrun.txt", "Не удалось получить доступ к криптопровайдеру.\n");	// Пишем в лог
        return NULL;
    }

    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {  // Если не удалось создать хеш-объект для алгоритма SHA-256
        CryptReleaseContext(hProv, 0);                          // Освобождаем память криптопровайдера
        CloseHandle(file);                                      // Закрываем файловый дескриптор (освобождаем память)
        writeToLogFile(path_Log, "log_DAVrun.txt", "Не удалось создать хеш-объект для алгоритма SHA-256.\n");	// Пишем в лог
        return NULL;
    }

    while (ReadFile(file, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead != 0) {    // Читаем файл по частям (1024 байта) и данные добавляем в хеш-объект "hHash"
        if (!CryptHashData(hHash, buffer, bytesRead, 0)) {                                  // Если при чтении файла данные не удалось добавить в хеш-объект
            CryptDestroyHash(hHash);                                                        // Освобождаем память хеш-объекта
            CryptReleaseContext(hProv, 0);                                                  // Освобождаем память криптопровайдера
            CloseHandle(file);                                                              // Закрываем файловый дескриптор (освобождаем память)
            writeToLogFile(path_Log, "log_DAVrun.txt", "Не удалось добавить данные из буфера в хеш-объект при чтении файла (по частям).\n");	// Пишем в лог
            return NULL;
        }
    }

    //Получаем вычисленное значение хеш-суммы из хеш-объекта и записываем его в "hashString" в виде строки в формате SHA-256
    if (CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
        for (DWORD i = 0; i < hashLen; i++) {
            snprintf(hashString + (i * 2), sizeof(hashString) - (i * 2), "%02x", hash[i]);
        }
    } else {
        CryptDestroyHash(hHash);        // Освобождаем память хеш-объекта
        CryptReleaseContext(hProv, 0);  // Освобождаем память криптопровайдера
        CloseHandle(file);
        writeToLogFile(path_Log, "log_DAVrun.txt", "Не удалось получить вычисленные значения хеш-суммы из хеш-объекта.\n");	// Пишем в лог
        return NULL;
    }

    CryptDestroyHash(hHash);        // Освобождаем память хеш-объекта
    CryptReleaseContext(hProv, 0);  // Освобождаем память криптопровайдера
    CloseHandle(file);

    //Сравниваем полученную хеш-сумму с предоставленной хеш-суммой
    if (strncmp(providedHash, hashString, sizeof(providedHash) - 1) != 0) { // Если хеш-суммы не совпадают
        deleteFilesInFolder(PATH_DIR, false);   // Путь до папки; true - удалять все файлы вместе с папкой, false - удалять только файлы внутри папки
        writeToLogFile(path_Log, "log_DAVrun.txt", "Хеш-сумма установочного файла не совпадает, установочный файл удалён.\n");	// Пишем в лог
        return NULL;
    }

    return hashString;  // Возвращаем строку с вычисленной хеш-суммой "hashString"
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

        if(res != CURLE_OK) {   // Если запрос не успешен
            //Формируем сообщение лога
            char logMsg[512];
            snprintf(logMsg, sizeof(logMsg), "Ошибка при выполнении запроса на скачивание установочного файла с сервера WebDAV - описание ошибки: %s\n", curl_easy_strerror(res));
            writeToLogFile(path_Log, "log_DAVrun.txt", logMsg); // Пишем ошибку в лог

            free(chunk.memory);         // Освобождаем память
            curl_easy_cleanup(curl);    // Освобождаем ресурсы указателя "curl"
            curl_global_cleanup();      // Очищаем глобальное состояние библиотеки libcurl
            return NULL;                // Возвращаем нулевой указатель
        }

        char* found = strstr(chunk.memory, "setup.exe_");   // Находим файл, начинающийся на "setup.exe_"
        
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
            //Формируем сообщение лога
            char logMsg[512];
            snprintf(logMsg, sizeof(logMsg), "Ошибка открытия установочного файла при попытке его скачивания - описание ошибки: %s\n", curl_easy_strerror(res));
            writeToLogFile(path_Log, "log_DAVrun.txt", logMsg); // Пишем ошибку в лог

            curl_easy_cleanup(curl);    // Освобождаем ресурсы указателя "curl"
            curl_global_cleanup();      // Очищаем глобальное состояние библиотеки libcurl
            return 1;                   // Ошибка открытия установочного файла
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
    //Формируем сообщение лога
    char logMsg[512];
    snprintf(logMsg, sizeof(logMsg), "Не удалось скачать файл - описание ошибки: %s\n", curl_easy_strerror(res));
    writeToLogFile(path_Log, "log_DAVrun.txt", logMsg); // Пишем ошибку в лог

    curl_global_cleanup();  // Очищаем глобальное состояние библиотеки libcurl
    return 1;               // Не удалось скачать файл
}


//Пустая функция для подавления вывода тела ответа при удалении файла через функцию "delWebDavFile()"
size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *stream) {
    (void)ptr;              // Игнорируем данный параметр
    (void)stream;           // Игнорируем данный параметр
    return size * nmemb;    // Возвращает количество байтов записанных данных (размер * количество элементов)
}

//Функция удаления файла с именем компьютера с сервера
void delWebDavFile(const char* host, const char* username, const char* password, const char* curlCrt, const char* pointsDirectory, char* delName) {
    CURL *curl;         // Указатель на объект типа CURL
    CURLcode res;       // Переменная типа "CURLcode" для хранения результата выполнения библиотеки libcurl
    long http_code = 0; // Переменная для хранения кода HTTP

    curl_global_init(CURL_GLOBAL_DEFAULT);  // Инициализируем глобальное состояние библиотеки libcurl
    curl = curl_easy_init();                // Инициализируем указатель "curl" для работы с запросами

    if(curl) {                                                  // Проверяем, что указатель "curl" не равен NULL
        char *encodedName = curl_easy_escape(curl, delName, 0); // Используем "curl_easy_escape()" для правильного URL-кодирования
        if(encodedName == NULL) {                               // Проверка на NULL
            //Формируем сообщение лога
            char logMsg[512];
            snprintf(logMsg, sizeof(logMsg), "Ошибка, не удалось закодировать URL-адрес перед попыткой удаления файла с именем компьютера с сервера - описание ошибки: %s\n", curl_easy_strerror(res));
            writeToLogFile(path_Log, "log_DAVrun.txt", logMsg); // Пишем ошибку в лог
            
            return; // Возвращаем управление вызывающей программе
        }

        char del_url[512];                                                                  // Символический массив в 512 элементов (с запасом) для хранения строки с URL-адресом
        snprintf(del_url, sizeof(del_url), "%s%s/%s", host, pointsDirectory, encodedName); // Формируем строку с URL-адресом до файла, который будем удалять

        struct curl_slist *headers = NULL;                                                          // Создаём список заголовков "headers" (это необходимо для корректной настройки HTTP запроса и передачи соответствующих заголовков серверу)
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");    // Добавляем заголовок

        //Устанавливаем опции
        curl_easy_setopt(curl, CURLOPT_URL, del_url);                   // Устанавливаем сформированный URL-адрес для запроса
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);      // Аутентификация посредством дайджеста
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);            // Устанавливаем заголовки HTTP для запроса
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);             // Логин
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);             // Пароль
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");        // Определяем метод "DELETE" для HTTP запроса (т.е. будем удалять с сервера)
        curl_easy_setopt(curl, CURLOPT_CAINFO, curlCrt);                   // Путь к файлу сертификата
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);   // Подавляем вывод тела ответа

        res = curl_easy_perform(curl);  // Выполняем запрос с помощью libcurl, результат пишем в "res"

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code); // Получаем код состояния HTTP

        if(res != CURLE_OK) {   // Если запрос не успешен
            //Формируем сообщение лога
            char logMsg[512];
            snprintf(logMsg, sizeof(logMsg), "Ошибка выполнения функции удаления файла с именем компьютера на сервере - описание ошибки: %s\n", curl_easy_strerror(res));
            writeToLogFile(path_Log, "log_DAVrun.txt", logMsg); // Пишем ошибку в лог

        } else if (http_code == 404) {
            writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка 404, файл с именем компьютера для удаления не найден на сервере.\n"); // Пишем в лог
        }

        curl_free(encodedName);         // Освобождаем память, выделенную для закодированного имени файла
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
     if (result != 0) writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка при удалении содержимого папки.\n"); // Пишем в лог
}


//Функция верификации скаченного установочного файла и проверки его хеш суммы
int VerificationCycle(const char* host, const char* username, const char* password, const char* curlCrt) {
    char* findFileName = findSetupFile(host, username, password, curlCrt); // Ищем установочный файл на сервере по маске "setup.exe_" и узнаём его полное имя с хешем

    if (findFileName == NULL) { // Если файл не был найден
        writeToLogFile(path_Log, "log_DAVrun.txt", "Не удалось найти установочный файл на сервере.\n");	// Пишем в лог
        return 1; // Не удалось найти файл setup.exe на сервере
    }

    // Если установочный файл был найден на сервере, тогда скачиваем его
    if (downloadFile(host, username, password, curlCrt, PATH_DIR, findFileName) != 0) {	// Если произошла ошибка при загрузке файла
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка при загрузке файла.\n");		// Пишем в лог
        free(findFileName); // Освобождаем память
        return 1; // Возвращаем код ошибки
    }

    char* result = checkSHA256(PATH_DIR, findFileName); // Вычисляем хеш-сумму (SHA-256) скаченного файла и сравниваем с предоставленным хешем в его имени

    if (result) { // Если хеши совпадают
        free(findFileName); // Освобождаем память
        return 0; // Возвращаем 0 (успех)
    } else {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Хеш-сумма установочного файла не совпадает.\n");	// Пишем в лог
        free(findFileName); // Освобождаем память
        return 1; // Возвращаем ошибку
    }
}


//Функция установки цвета в командной строке Windows
void setConsoleTextColor(WORD color) {                  // Функция принимает 16-битное число цвета
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // Получаем дескриптор консоли
    SetConsoleTextAttribute(hConsole, color);           // Устанавливаем цвет в консоли
}


int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "ru_RU.UTF-8"); // Установка локали для корректного отображения текста на Русском языке

    //Проверка аргумента для удаления конфига
    if (argc == 2 && strcasecmp(argv[1], "DELETE CONFIG") == 0) {       // Если получили аргумент "DELETE CONFIG" (в любом регистре)
        if (!file_exists(CONFIG_FILE) && !file_exists(KEY_FILE)) {      // Проверяем, что файлы есть
            setConsoleTextColor(FOREGROUND_RED | FOREGROUND_INTENSITY); // Ярко красный цвет
            printf("Файлы конфигурации и ключа не существуют!\n");
            setConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);   // Сбрасывавем цвет на стандартный
            return 0;
        }
        setConsoleTextColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);               // Ярко зелёный цвет
        delete_config_and_key();                                                    // Удаляем конфиг
        setConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);   // Сбрасывавем цвет на стандартный
        return 0;
    }

    //Если получили все 6 аргументов (нулевой аргумент это сама программа, поэтому прибавляем +1)
    if (argc == 7) {
        const char *main_host = argv[1];
        const char *main_username = argv[2];
        const char *main_password = argv[3];
        const char *main_crt = argv[4];
        const char *main_points_dir = argv[5];

        //Разрешаем ввод чисел от 0 до 14 в последнем аргументе при создании конфига
        if (!is_positive_integer(argv[6]) || atoi(argv[6]) < 0 || atoi(argv[6]) > 14) {
            setConsoleTextColor(FOREGROUND_RED | FOREGROUND_INTENSITY); // Ярко красный цвет
            printf("Недопустимое значение в последнем аргументе. Используйте положительное целое число от 0 до 14.\n");
            setConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);   // Сбрасывавем цвет на стандартный
            return 1;
        }

        int number_symbol = atoi(argv[6]);                                                              // Конвертируем последний аргумент в число
        save_config(main_host, main_username, main_password, main_crt, main_points_dir, number_symbol); // Сохраняем конфиг

        setConsoleTextColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Ярко зелёный цвет
        printf("Конфигурация сохранена.\n");
        setConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);   // Сбрасывавем цвет на стандартный
        return 0;
    } else {
        //Если получили меньше или больше 6 аргументов
        char main_host[128];
        char main_username[128];
        char main_password[128];
        char main_crt[128];
        char main_points_dir[128];
        unsigned int main_number_symbol;

        //Если не нашли конфиг файл и ключ, выводим справку
        if (!file_exists(CONFIG_FILE) && !file_exists(KEY_FILE)) {
            //СПРАВКА
            setConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);   // Ярко белый цвет
            printf("Конфигурация не найдена. Задайте конфигурацию с помощью соответствующих аргументов.\n\n");

            setConsoleTextColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);    // Ярко зелёный цвет
            printf("Использование: DAVrun.exe \"Хост\" \"Логин\" \"Пароль\" \"Имя сертификата с расширением\" \"Каталог ожидания установок на сервере\" \"Кол-во символов (число от 0 до 14) для обрезки символов от начала имени ПК\".\n");
            printf("Для удаления конфига использовать: DAVrun.exe \"Delete Config в любом регистре\".\n\n");

            setConsoleTextColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);     // Ярко синий цвет
            printf("Пример: DAVrun.exe \"https://77.77.77.77:7777/webdaw/\" \"User\" \"Password\" \"cert.crt\" \"Points\" 7\n");
            printf("Пример: DAVrun.exe \"DELETE CONFIG\"\n\n");
            
            setConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);   // Ярко жёлтый цвет
            printf("Автор Otto, г.Омск 2024\n");
            printf("GitHub страница:  https://github.com/Otto17/DelCert\n");
            printf("GitFlic страница: https://gitflic.ru/project/otto/delcert\n");

            setConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Сбрасывавем цвет на стандартный
            return 1;
        }

        //Если конфиг и ключ существуют и успешно расшифрованы, тогда запускаем программу
        if (load_config(main_host, main_username, main_password, main_crt, main_points_dir, &main_number_symbol)) {
            HWND hWnd = GetConsoleWindow(); // Получаем дескриптор окна консоли, связанного с текущим процессом
	        ShowWindow(hWnd, SW_HIDE);		// Скрываем окно консоли, дескриптора "hWnd"

            //Полный путь до файла с сертификатом открытого ключа (*.crt или *.pem)
            char curl_CRT[256];
            snprintf(curl_CRT, sizeof(curl_CRT), "C:\\Program Files\\DAVrun\\%s", main_crt);

            char* namePC = getNamePC(main_number_symbol);   // Копируем имя компьютера в "namePC"

            //Проверяем получение имени ПК на ошибки
            if (namePC == "ErrorGetNamePC") {
                printf("Ошибка! - %s", namePC);   // ТЕСТ
                return 1;   // Завершаем работу программы
            }

            char* resultName = checkWebDavFile(main_host, main_username, main_password, curl_CRT, main_points_dir, namePC); // Ищем на сервере файл с таким же именем как у этого компьютера

            if(strcmp(resultName, "NULL") != 0) {   // Если на сервере есть файл с таким же именем, как у этого компьютера
                if (VerificationCycle(main_host, main_username, main_password, curl_CRT) == 0) {     // И если установочный файл успешно скачали и верифицировали
                
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
                            writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка при запуске установочного файла - Пользователь отказался от повышения прав.\n"); // Пишем в лог
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
                            char logMsg[512];
                            snprintf(logMsg, sizeof(logMsg), "Ошибка при запуске установочного файла - Код ошибки: %lu, Описание: %s", dwError, errorMsgUTF8);

                            writeToLogFile(path_Log, "log_DAVrun.txt", logMsg); // Пишем ошибку в лог

                            deleteFilesInFolder(PATH_DIR, true);	// Удаляем папку с файлами
                        }
                        return 1;   // Возвращаем 1 в случае ошибки
                    }

                    //Ожидание завершения процесса
                    if (sei.hProcess != NULL) {                         // Проверяем, успешен ли вызов "ShellExecuteEx()"
                        WaitForSingleObject(sei.hProcess, INFINITE);    // Если процесс успешно запущен и получен его хэндл, тогда ожидаем завершения процесса
                        CloseHandle(sei.hProcess);                      // Освобождаем ресурсы, связанные с хэндлом
                    }

                    delWebDavFile(main_host, main_username, main_password, curl_CRT, main_points_dir, resultName);  // Если установка программы завершилась успешно, тогда удаляем с сервера файл с таким же именем, как у этого компьютера

                    //Удаление папки со всеми подкаталогами и установочным файлом
                    if (DEL_DIR == TRUE) deleteFilesInFolder(PATH_DIR, true);	// Если флаг поднят, то выполняем функцию удаления папки с файлами
                }
            }
            free(resultName);   // Освобождаем память
    
        } else {
            //Если не конфига или ключа не нашли или не смогли расшифровать конфиг
            setConsoleTextColor(FOREGROUND_RED | FOREGROUND_INTENSITY); // Ярко красный цвет
            printf("Конфигурационный файл или ключ отсутствуют, либо повреждены.\n");
            
            setConsoleTextColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);               // Ярко зелёный цвет
            delete_config_and_key();                                                    // Удаляем конфиг
            setConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);   // Сбрасывавем цвет на стандартный
            return 1;
        }
    }

    return 0;
}