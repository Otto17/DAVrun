#ifndef LOGFILE_H
#define LOGFILE_H

#include <stdio.h>		// Библиотека для определения функций и работы с потоками ввода/вывода
#include <time.h>       // Библиотека для работы со временем и датой
#include <Windows.h>    // Библиотека, которая предоставляет доступ к API Windows


extern const int max_Log_Size;   // Получаем из настроек в "main.c" сюда максимальный размер (в байтах) лог файла для ротации. (Установлено 300 Кбайт)
extern const int max_Log_Files;  // Получаем из настроек в "main.c" сюда максимальное количество лог файлов для хранения. Архивные файлы с суффиксом "_0", "_1"...


//Функция проверки размера лог файла
long getFileSize(const char *filepath);

//Функция ротации лог файлов
void rotateLogFile(const char *directory, const char *filename);

//Функция логирования
void writeToLogFile(const char *directory, const char *filename, const char *text);

#endif // LOGFILE_H