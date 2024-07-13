#ifndef SOFT_ENCRYPTION_H
#define SOFT_ENCRYPTION_H

#include <openssl/evp.h>    // Библиотека OpenSSL для работы с шифрованием и хэшированием данных
#include <openssl/rand.h>   // Библиотека OpenSSL для генерации случайных чисел
#include <stdio.h>          // Библиотека для определения функций и работы с потоками ввода/вывода
#include <stdlib.h>         // Библиотека предоставляет функции для управления памятью
#include <string.h>         // Библиотека для работы со строками
#include <windows.h>        // Библиотека, которая предоставляет доступ к API Windows
#include <stdbool.h>        // Библиотека для возможности использования bool
#include <ctype.h>          // Библиотека для работы с символами (используется для функций обработки символов)
#include <locale.h>         // Библиотека для управления локальными настройками

#include "logFile.h"    // Подключаем файл лога

#define CONFIG_FILE "config.enc"    // Зашифрованный конфиг файл
#define KEY_FILE "key.bin"          // Тут храним зашифрованный ключ с рандомной солью
#define SALT_SIZE 32                // Длина соли (32 байта)

extern const char *hardcoded_password;  // Получаем сюда жёстко закодированный пароль из настроек в "main.c"
extern const char *pepper;              // Получаем сюда перчик для пароля из настроек в "main.c"
extern const char *path_Log;            // Получаем сюда путь для сохранения лога из настроек в "main.c"

//Функции для генерации ключа шифрования
void generate_key_from_password(const char *password, const unsigned char *salt, const char *pepper, unsigned char *key);

//Функции для шифрования данных
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext);

//Функции для дешифрования данных
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext);

//Функции для сохранения ключа шифрования
int save_key(const unsigned char *key, const unsigned char *salt);

//Функции для загрузки ключа шифрования
int load_key(unsigned char *key, unsigned char *salt);

//Функции для сохранения конфигурации
void save_config(const char *host, const char *username, const char *password, const char *crt, const char *points_dir, int number_symbol);

//Функции для загрузки конфигурации
int load_config(char *host, char *username, char *password, char *crt, char *points_dir, int *number_symbol);

//Функции для проверки валидности целого положительного числа
bool is_positive_integer(const char *str);

//Функции для удаления файлов конфигурации и ключа
void delete_config_and_key();

//Функции для проверки существования файла
bool file_exists(const char *filename);

#endif // SOFT_ENCRYPTION_H