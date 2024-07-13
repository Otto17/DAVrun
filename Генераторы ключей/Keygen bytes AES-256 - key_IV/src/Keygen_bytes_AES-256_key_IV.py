"""
Данная программа является свободным программным обеспечением, распространяющимся по лицензии MIT.
Копия лицензии: https://opensource.org/licenses/MIT

Copyright (c) 2024 Otto
Автор: Otto
Версия: 20.06.24
GitHub страница:  https://github.com/Otto17/DAVrun
GitFlic страница: https://gitflic.ru/project/otto/davrun

г. Омск 2024

"""


import os

# Генерация 32-байтового ключа
key = os.urandom(32)
# Генерация 16-байтового IV
iv = os.urandom(16)

# Преобразование байтов в строку для печати
key_str = ', '.join(str(b) for b in key)
iv_str = ', '.join(str(b) for b in iv)

# Подготовка шаблона перед сохранением в файл
intro_text = "Рандомные ключи AES-256 и IV в байтовом виде.\n\n"
output_content = f"32-байтовый ключ: {key_str}\n16-байтовый IV: {iv_str}"

# Запись в файл
with open("Output_key_IV.txt", "w") as file:
    file.write(intro_text + output_content)

print("Ключ и IV успешно записаны в Output_key_IV.txt")
