"""
Данная программа является свободным программным обеспечением, распространяющимся по лицензии MIT.
Копия лицензии: https://opensource.org/licenses/MIT

Copyright (c) 2024 Otto
Автор: Otto
Версия: 11.07.24
GitHub страница:  https://github.com/Otto17/DAVrun
GitFlic страница: https://gitflic.ru/project/otto/davrun

г. Омск 2024

"""


import os
import secrets

# Функция для генерации случайной строки заданной длины
def generate_random_string(length):
    alphabet = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
    return ''.join(secrets.choice(alphabet) for _ in range(length))

# Длина "поперчить" и жёстко закодированного пароля
PEPPER_LENGTH = 16
PASSWORD_LENGTH = 32

# Генерация "поперчить" и жёстко закодированного пароля
pepper = generate_random_string(PEPPER_LENGTH)
hardcoded_password = generate_random_string(PASSWORD_LENGTH)

# Подготовка шаблона перед сохранением в файл
intro_text = "Рандомные значения жёстко задаваемого пароля и перца.\n\n"
output_content = f"Жёстко задаваемый пароль: {hardcoded_password}\nПоперчить: {pepper}"

# Создание файла и запись в него сгенерированных значений
with open("Output_Pepper_Passwd.txt", "w") as file:
    file.write(intro_text + output_content)

print("Сгенерированные значения сохранены в файл Output_Pepper_Passwd.txt")

