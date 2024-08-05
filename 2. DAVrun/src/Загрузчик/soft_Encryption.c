#include "soft_Encryption.h"


//Функции для генерации ключа шифрования
void generate_key_from_password(const char *password, const unsigned char *salt, const char *pepper, unsigned char *key) {
    char combined[128];
    snprintf(combined, sizeof(combined), "%s%s", password, pepper); // Комбинируем пароль и перчик

    //Проверяем результат вызова функции "PKCS5_PBKDF2_HMAC", которая используется для вычисления производного ключа из пароля
    if (!PKCS5_PBKDF2_HMAC(combined, strlen(combined), salt, SALT_SIZE, 10000, EVP_sha256(), 32, key)) {    // Если вернёт false, то это ошибка
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка генерации ключа из пароля.\n"); // Пишем в лог
        exit(1);    //Программа завершает свою работу с кодом ошибки "1"
    }
}


//Функции для шифрования данных
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;    // Создаём указатель на объект "EVP_CIPHER_CTX"
    int len;
    int ciphertext_len;

    //Выделение памяти и инициализация структуры "EVP_CIPHER_CTX" в переменную "ctx"
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка выделения памяти при шифровании конфига.\n"); // Пишем в лог
        return -1;                                           
    }

    //Инициализация шифра AES с размером ключа 256 бит в режиме CBC с помощью "EVP_EncryptInit_ex"
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка инициализации AES-256 в режиме CBC при шифровании конфига.\n"); // Пишем в лог
        return -1;
    }

    //Шифрование блока данных "plaintext" длиной "plaintext_len" с помощью "EVP_EncryptUpdate" и запись результата в "ciphertext"
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка при шифровании данных конфига.\n"); // Пишем в лог
        return -1;
    }
    ciphertext_len = len;   // Получение длины шифрованного текста

    //Завершение операции шифрования с помощью "EVP_EncryptFinal_ex", запись дополнительных данных в конец "ciphertext"
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка при завершении шифрования данных конфига.\n"); // Пишем в лог
        return -1;
    }
    ciphertext_len += len;  // Обновление переменной "ciphertext_len" на размер завершающей части шифрованного текста

    EVP_CIPHER_CTX_free(ctx);   // Освобождаем память

    return ciphertext_len;  // Возвращаем значение "ciphertext_len" - общей длины шифрованного текста
}


//Функции для дешифрования данных
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx;    // Создаём указатель на объект "EVP_CIPHER_CTX"
    int len;
    int plaintext_len;

    //Выделение памяти и инициализация структуры "EVP_CIPHER_CTX" в переменную "ctx"
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка выделения памяти при дешифровании конфига.\n"); // Пишем в лог
        return -1;
    }

    //Инициализируется расшифровка с использованием алгоритма AES с длиной ключа 256 бит в режиме CBC
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка инициализации AES-256 в режиме CBC при дешифровании конфига.\n"); // Пишем в лог
        return -1;
    }

    //Производится расшифровка шифротекста "ciphertext" длиной "ciphertext_len", результат записывается в буфер "plaintext"
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка при дешифровании конфига.\n"); // Пишем в лог
        return -1;
    }
    plaintext_len = len;    // Получение длины расшифрованного текста

    //Проверяется успешное завершение расшифровки
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка при завершении дешифрования конфига.\n"); // Пишем в лог
        return -1;
    }
    plaintext_len += len;   // Суммируется длина расшифрованного текста с предыдущей длиной

    EVP_CIPHER_CTX_free(ctx);   // Освобождаем память

    return plaintext_len;   // Возвращаем длину расшифрованного текста "plaintext_len"
}


//Функции для сохранения ключа шифрования
int save_key(const unsigned char *key, const unsigned char *salt) {
    //Открываем файл для записи в бинарном режиме
    FILE *file = fopen(KEY_FILE, "wb");
    if (!file) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка при открытии бинарного файла на запись для сохранения ключа шифрования.\n"); // Пишем в лог
        return 0;   // Если открытие файла не удалась, тогда возвращаем ошибку с кодом "0"
    }

    fwrite(salt, 1, SALT_SIZE, file);   // Записываем соль в бинарный файл
    fwrite(key, 1, 32, file);           // Записываем ключ из массива размером 32 байта в бинарный файл
    fclose(file);                       // Закрываем файл

    return 1;   // Возвращаем "1", как успех всех предыдущих шагов
}


//Функции для загрузки ключа шифрования
int load_key(unsigned char *key, unsigned char *salt) {
    //Открываем файл для чтения в бинарном режиме
    FILE *file = fopen(KEY_FILE, "rb");
    if (!file) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка при открытии бинарного файла на чтение для считывания ключа шифрования и соли.\n"); // Пишем в лог
        return 0;   // Возвращаем ошибку с кодом "0"
    }

    fread(salt, 1, SALT_SIZE, file);    // Считываем соль из бинарного файла
    fread(key, 1, 32, file);            // Считываем ключ размером 32 байта из бинарного файла
    fclose(file);                       // Закрываем файл

    return 1;   // Возвращаем "1", как успех всех предыдущих шагов
}


//Функции для сохранения конфигурации
void save_config(const char *host, const char *username, const char *password, const char *crt, const char *points_dir, int number_symbol) {
    unsigned char iv[16];
    
    //Генерируем случайный вектор инициализации (IV) длиной 16 байт
    if (!RAND_bytes(iv, 16)) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка, не удалось сгенерировать вектор инициализации (IV).\n"); // Пишем в лог
        return; // Выходим из функции, не возвращая ничего
    }

    //Создаём буфер "plaintext" и заполняем данными из аргументов при создании конфига через "snprintf()"
    unsigned char plaintext[1024];
    snprintf((char *)plaintext, sizeof(plaintext), "host=%s\nusername=%s\npassword=%s\ncrt=%s\npoints_dir=%s\nnumber_symbol=%d\n", host, username, password, crt, points_dir, number_symbol);

    //Создаём массивы "ciphertext", "key", "salt"
    unsigned char ciphertext[2048];
    unsigned char key[32];
    unsigned char salt[SALT_SIZE];

    if (!RAND_bytes(salt, SALT_SIZE)) { // Если не получилось сгенерировать случайную соль длиной "SALT_SIZE"
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка, не удалось сгенерировать случайную соль.\n"); // Пишем в лог
        return; // Выходим из функции, не возвращая ничего
    }

    generate_key_from_password(hardcoded_password, salt, pepper, key);                          // Генерируем ключ через "generate_key_from_password()" из жёстко закодированного пароля, соли и пеппера и сохраняем в массив "key"
    int ciphertext_len = encrypt(plaintext, strlen((char *)plaintext), key, iv, ciphertext);    // Шифруем "plaintext" с помощью полученного ключа и "IV". Результат сохраняется в массив "ciphertext", а длина зашифрованного текста в "ciphertext_len"

    //Открываем файл для записи в бинарном режиме
    FILE *file = fopen(CONFIG_FILE, "wb");
    if (!file) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка, не удалось открыть бинарный файл конфига для записи данных.\n"); // Пишем в лог
        return; // Выходим из функции, не возвращая ничего
    }

    fwrite(iv, 1, 16, file);                        // Записываем "IV" в конфиг файл
    fwrite(ciphertext, 1, ciphertext_len, file);    // Записываем зашифрованные данные в конфиг файл
    fclose(file);                                   // Закрываем файл

    //Если не удалось сохранить ключ шифрования с солью
    if (!save_key(key, salt)) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка, не удалось сохранить ключ шифрования с солью при создании конфиг файла.\n"); // Пишем в лог
    }
}


//Функции для загрузки конфигурации
int load_config(char *host, char *username, char *password, char *crt, char *points_dir, int *number_symbol) {
    unsigned char key[32];
    unsigned char salt[SALT_SIZE];

    //Загружаем ключ и соль
    if (!load_key(key, salt)) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка, не удалось загрузить ключ и соль при загрузке конфиг файла.\n"); // Пишем в лог
        return 0;   // Возвращаем ошибку с кодом "0"
    }

    generate_key_from_password(hardcoded_password, salt, pepper, key);  // Вызываем функцию для генерации ключа шифрования

    //Открываем файл для чтения в бинарном режиме
    FILE *file = fopen(CONFIG_FILE, "rb");
    if (!file) {
        writeToLogFile(path_Log, "log_DAVrun.txt", "Ошибка открытия бинарного файла на чтение при загрузке конфиг файла.\n"); // Пишем в лог
        return 0;   // Возвращаем ошибку с кодом "0"
    }

    //Создаём массив "iv" размером 16 байт и считываем 16 байт из файла в "iv"
    unsigned char iv[16];
    fread(iv, 1, 16, file);

    fseek(file, 0, SEEK_END);           // Устанавливаем указатель в конец файла, чтобы определить его размер
    long fileSize = ftell(file) - 16;   // Вычисляем разницу после 16-го байта (т.к. первые 16 байт - IV)
    fseek(file, 16, SEEK_SET);          // Переводим указатель в начало файла, после IV

    unsigned char *ciphertext = (unsigned char *)malloc(fileSize);  // Выделяем динамическую память для массива "ciphertext" размером "fileSize"
    fread(ciphertext, 1, fileSize, file);                           // Читаем "fileSize" байт из файла в "ciphertext"
    fclose(file);                                                   // Закрываем файл

    //Дешифруем конфиг через функцию "decrypt()" с использованием ключа и IV
    unsigned char plaintext[1024];
    int plaintext_len = decrypt(ciphertext, fileSize, key, iv, plaintext);

    if (plaintext_len == -1) {  // Если длина "plaintext_len" равна "-1"
        free(ciphertext);       // Освобождаем память
        return 0;               // Возвращаем ошибку с кодом "0"
    }
    plaintext[plaintext_len] = '\0';    // Добавляем символ конца строки

    //Извлекаем значения из конфига и сохраняем их в переменные
    sscanf((char *)plaintext, "host=%s\nusername=%s\npassword=%s\ncrt=%s\npoints_dir=%s\nnumber_symbol=%d\n", host, username, password, crt, points_dir, number_symbol);

    free(ciphertext);   // освобождаем память

    return 1;   // Возвращаем "1", как успех всех предыдущих шагов
}


//Функции для проверки валидности целого положительного числа
bool is_positive_integer(const char *str) {
    //Цикл выполняется, пока текущий символ в строке не будет равен '\0' (концу строки)
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) { // Проверяем, является ли текущий символ цифрой, если нет, возвращаем "false"
            return false;       // Если в строке встретился не цифровой символ, возвращаем "false"
        }
    }
    return true;    // Если все символы в строке являются цифрами, возвращаем "true"
}


//Функции для удаления файлов конфигурации и ключа
void delete_config_and_key() {
    remove(CONFIG_FILE);    // Удаляем конфиг файл
    remove(KEY_FILE);       // Удаляем ключ
    printf("Конфигурация и ключ успешно удалены.\n");
}


//Функции для проверки существования файла
bool file_exists(const char *filename) {
    DWORD fileAttr = GetFileAttributesA(filename);  // Получаем атрибут файла

    //Проверяем условие, если "fileAttr" не равен "INVALID_FILE_ATTRIBUTES" (константа, обозначающая, что атрибут файла недоступен) и при этом не является директорией "FILE_ATTRIBUTE_DIRECTORY"
    return (fileAttr != INVALID_FILE_ATTRIBUTES && !(fileAttr & FILE_ATTRIBUTE_DIRECTORY)); // Если оба условия верные, возвращаем "true"
}