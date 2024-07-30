/*
    Программа представляет собой службу в Windows, которая запускает другую указанную в коде программу через установленный интервал времени.

    Программа, при запуске с ключом "-is" (insstal - start) установит и запустит службу с именем "SERVICE-DAVrun.exe".
    При запуске с ключом "-sd" (stop - delete) служба остановится и удалится.
    При попытке запустить программу из командной строки без ключей выйдет сообщение:
	Программа будет работать только как служба!

	Ключ "-is" для установки и запуска службы
	Ключ "-sd" для остановки и удаления службы


	Данная программа является свободным программным обеспечением, распространяющимся по лицензии MIT.
	Копия лицензии: https://opensource.org/licenses/MIT

	Copyright (c) 2024 Otto
	Автор: Otto
	Версия: 20.06.24
	GitHub страница:  https://github.com/Otto17/DAVrun
	GitFlic страница: https://gitflic.ru/project/otto/davrun

	г. Омск 2024
*/


//Настройки службы
#define START_TIMER 10                                                  // Время (в минутах), через которое будет циклически запускаться программа
#define PROGRAM_PATH "C:\\Program Files\\DAVrun\\DAVrun.exe"            // Путь к программе, которая будет запускаться каждые "START_TIMER" минут
#define SERVICE_PATH "C:\\Program Files\\DAVrun\\SERVICE-DAVrun.exe"    // Путь к этой программе, которая будет работать как служба
#define SERVICE_NAME "DAVrun"                                           // Название службы
#define DISPLAY_NAME "DAVrun Service"                                   // Отображаемое имя службы
#define DESCRIPTION  L"Служба каждый установленный интервал времени запускает программу \"DAVrun\", для авторазвёртывания приложений." // Описание службы

//Логирование
#define PATH_LOG "C:\\ProgramData\\DAVrun"  // Путь, куда сохранять лог файл
#define MAX_LOG_SIZE 50000                  // Максимальный размер (в байтах) лог файла для ротации. (Установлено 50 Кбайт)
#define MAX_LOG_FILES 1                     // Максимальное количество лог файлов для хранения. Архивные файлы с суффиксом "_0"


//Подключаем библиотеки
#include <windows.h>    // Библиотека, которая предоставляет доступ к API Windows
#include <stdio.h>      // Библиотека для определения функций и работы с потоками ввода/вывода
#include <locale.h>     // Библиотека для работы с локалью и форматированием текста
#include <time.h>       // Библиотека для работы со временем и датой


//Переменные и функции для работы с сервисами Windows
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;
SERVICE_STATUS status;
SC_HANDLE schSCManager;
SC_HANDLE schService;


//Заглушки
void ServiceMain(int argc, char** argv);
void ControlHandler(DWORD request);
int InstallService();
int UninstallService();
int InitService();


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


//Функция установки параметров службы
void ServiceMain(int argc, char** argv) {
    ServiceStatus.dwServiceType        = SERVICE_WIN32_OWN_PROCESS;
    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwWin32ExitCode      = 0;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint         = 0;
    ServiceStatus.dwWaitHint           = 0;

    hStatus = RegisterServiceCtrlHandler(SERVICE_NAME, (LPHANDLER_FUNCTION)ControlHandler); // Регистрируем обработчик управляющих кодов службы
    ServiceStatus.dwCurrentState = SERVICE_RUNNING; // Устанавливаем текущее состояние службы как "запущена"
    SetServiceStatus(hStatus, &ServiceStatus);      // установка нового состояния и кода завершения службы (запускаем службу)

    if (InitService() != 0) {   // Инициализируем службу. Если результат функции не равен 0, выводится сообщение об ошибке
        wprintf(L"Ошибка при инициализации службы.\n");
        writeToLogFile(PATH_LOG, "log_SERVICE-DAVrun.txt", "Ошибка при инициализации службы.\n");   // Пишем ошибку в лог
    }
}


//Функция для определения действия при установке параметров службы
void ControlHandler(DWORD dwControl) {
    switch (dwControl) {    // Если "dwControl" равен "SERVICE_CONTROL_SHUTDOWN" или "SERVICE_CONTROL_STOP",
    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
        ServiceStatus.dwWin32ExitCode = 0;              // Устанавливаем код завершения 0
        ServiceStatus.dwCurrentState = SERVICE_STOPPED; // Устанавливаем состояние службы как "остановлена"
        SetServiceStatus(hStatus, &ServiceStatus);      // установка нового состояния и кода завершения службы (останавливаем службу)
        return;
    default:    // Иначе ничего не делаем
        break;
    }

    SetServiceStatus(hStatus, &ServiceStatus);  // установка нового состояния и кода завершения службы
}


//Преобразование "WCHAR" в "LPSTR"
LPSTR WideCharToLPSTR(WCHAR* wstr) {
    static char buf[512];                                           // Буфер в 512 символов
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, buf, 512, NULL, NULL); // Преобразуем
    return buf;                                                     // Возвращаем указатель на буфер с преобразованной строкой
}


//Функция установки службы
int InstallService() {
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);    // Открываем менеджер служб Windows для создания новой службы
    if (schSCManager) {                                                     // Создаём новую службу с указанными параметрами
        schService = CreateService(schSCManager, SERVICE_NAME, DISPLAY_NAME,
            SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START,
            SERVICE_ERROR_CRITICAL, SERVICE_PATH, NULL, NULL, NULL, NULL, NULL);
        
        if (schService) {
            //Добавляем описание службе
            SERVICE_DESCRIPTION serviceDescription;                                             // Объявляем строковую переменную для хранения информации о сервисе
            WCHAR description[] = DESCRIPTION;                                                  // Добавляем описание службы
            serviceDescription.lpDescription = WideCharToLPSTR(description);                    // Конвертируем широкие символы (Unicode) в формат LPSTR
            ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &serviceDescription);  // Изменяем текущую конфигурацию службы, добавляя описание службы

            if(StartService(schService, 0, NULL)){  // Запускаем службу после её успешной установки
                CloseServiceHandle(schService);     // Закрываем дескриптор самой службы
                CloseServiceHandle(schSCManager);   // Закрываем дескриптор менеджера служб
                wprintf(L"Служба установлена и успешно запущена!\n");
                writeToLogFile(PATH_LOG, "log_SERVICE-DAVrun.txt", "Служба установлена и успешно запущена!\n"); // Пишем ошибку в лог
                return 0;                           // Возвращаем 0 как успешное завершение функции
            }
            else{                                   // Иначе возвращаем код ошибки 1
                wprintf(L"Не удалось запустить службу после установки.\n");
                writeToLogFile(PATH_LOG, "log_SERVICE-DAVrun.txt", "Не удалось запустить службу после установки.\n");   // Пишем ошибку в лог
                CloseServiceHandle(schService);     // Закрываем дескриптор самой службы
                CloseServiceHandle(schSCManager);   // Закрываем дескриптор менеджера служб
                return 1;
            }
        }
        else {
            CloseServiceHandle(schSCManager); // Закрываем дескриптор менеджера служб
        }
    }

    wprintf(L"Не удалось установить службу.\n");
    writeToLogFile(PATH_LOG, "log_SERVICE-DAVrun.txt", "Не удалось установить службу.\n");  // Пишем ошибку в лог
    return 1;   // Возвращаем код ошибки 1, если не смогли создать службу
}


//Функция удаления службы
int UninstallService() {
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);    // Открываем менеджер служб Windows для создания новой службы
    if (schSCManager) {                                                 // Открываем службу с указанным именем и привилегиями доступа
        schService = OpenService(schSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
        
        if (schService) {                                               // Если успешно открыли службу
            SERVICE_STATUS status;
            ControlService(schService, SERVICE_CONTROL_STOP, &status);  // Останавливаем службу

            if (DeleteService(schService)) {                            // Если успешно остановили службу, то удаляем её
                CloseServiceHandle(schService);                         // Закрываем дескриптор самой службы
                CloseServiceHandle(schSCManager);                       // Закрываем дескриптор менеджера служб
                wprintf(L"Служба успешно остановлена и удалена!\n");
                writeToLogFile(PATH_LOG, "log_SERVICE-DAVrun.txt", "Служба успешно остановлена и удалена!\n");  // Пишем ошибку в лог
                return 0;                                               // Возвращаем 0 как успешное завершение функции
            }
            CloseServiceHandle(schService);  // Освобождаем память дескриптора служб
        }
        CloseServiceHandle(schSCManager);  // Освобождаем память менеджера служб
    }

    wprintf(L"Не удалось удалить службу.\n");
    writeToLogFile(PATH_LOG, "log_SERVICE-DAVrun.txt", "Не удалось удалить службу.\n"); // Пишем ошибку в лог
    return 1;                                                           // Возвращаем код ошибки 1, если не смогли удалить службу
}


//Функция инициализации службы, для запуска программы по установленному таймеру
int InitService() {
    while (1) {                         // Создаём бесконечный цикл
        STARTUPINFO si;                 // 'si' и 'pi' используются для передачи информации о запускаемом процессе
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));    // Инициализируем 'si'
        si.cb = sizeof(si);             // Устанавливаем значение поля cb структуры si равным размеру структуры si
        ZeroMemory(&pi, sizeof(pi));    // Инициализируем 'pi'

        if (!CreateProcess(PROGRAM_PATH, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {   // Создаём новый процесс и запускаем программу "PROGRAM_PATH"
            writeToLogFile(PATH_LOG, "log_SERVICE-DAVrun.txt", "Ошибка запуска программы \"DAVrun\"\n");    // Пишем ошибку в лог
            return 1;   // Если процесс не создался, выходим с кодом ошибки 1
        }

        CloseHandle(pi.hProcess);   // Закрываем процессы, созданные "CreateProcess", для избежания утечек ресурсов
        CloseHandle(pi.hThread);
        
        Sleep(START_TIMER * 60 * 1000); // Ждём "START_TIMER" минут до следующего запуска программы
    }

    return 0;
}


//Главная функция
int main(int argc, char* argv[]) {              // Принимаем аргументы из cmd
    setlocale(LC_ALL, "ru_RU.UTF-8");           // Установка локали для корректного отображения текста на Русском языке

    if (argc == 2) {                            // Проверка кол-ва переданных аргументов командной строки
        if (strcmp(argv[1], "-is") == 0) {      // Сравнение строки из аргументов командной строки с "-is", если совпадает
            return InstallService();            // Вызываем функцию установки и запуска службы
        }
        else if (strcmp(argv[1], "-sd") == 0) { // Сравнение строки из аргументов командной строки с "-sd", если совпадает
            return UninstallService();          // Вызываем функцию остановки и удаления службы
        }
    }

    SERVICE_TABLE_ENTRY ServiceTable[] = {      // Инициализация массива структур "SERVICE_TABLE_ENTRY"
        { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    if (!StartServiceCtrlDispatcher(ServiceTable)) {    // Вызываем функцию для регистрации службы в контроллере служб Windows
        wprintf(L"Программа будет работать только как служба!\n\n");
        wprintf(L"Ключ \"-is\" для установки и запуска службы\n");
        wprintf(L"Ключ \"-sd\" для остановки и удаления службы\n");
        writeToLogFile(PATH_LOG, "log_SERVICE-DAVrun.txt", "Попытка запустить службу как программу, либо с неверным ключом.\n");    // Пишем ошибку в лог
        return 1;                                       // Если не смогли вызвать функцию, выводим сообщения и завершаем работу с кодом ошибки 1
    }

    return 0;   // Возвращаем 0 как успешное завершение работы главной функции
}
