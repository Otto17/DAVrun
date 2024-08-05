#include "logFile.h"

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

    //Удаление самого старого лога, если их больше "max_Log_Files"
    snprintf(oldFilePath, sizeof(oldFilePath), "%s/%s_%d", directory, filename, max_Log_Files - 1); // Формируем путь к самому старому логу
    remove(oldFilePath);    // Удаляем самый старый лог

    //Переименование оставшихся логов
    for (int i = max_Log_Files - 2; i >= 0; i--) {
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
    if (fileSize >= max_Log_Size) {         // Если размер файла >= установленному (в байтах)
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
