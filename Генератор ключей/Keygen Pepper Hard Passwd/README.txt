Данная программа является свободным программным обеспечением, распространяющимся по лицензии MIT.
Копия лицензии: https://opensource.org/licenses/MIT

Copyright (c) 2024 Otto
Автор: Otto
Версия: 11.07.24
GitHub страница:  https://github.com/Otto17/DAVrun
GitFlic страница: https://gitflic.ru/project/otto/davrun

г. Омск 2024



Для компиляции генератора ключей (Keygen Pepper & Hard Passwd) на Python и создания исполняемого файла:
1) Скачиваем Python3 "https://www.python.org/downloads/windows/" (перед установкой поставить обе галочки 'Use admin privileges when installing py.exe' и 'Add python.exe to PATH') и устанавливаем.
Использовал версию 3.12.4;
2) Запустить "cmd", перейти в папку с исходным кодом и установить 2 пакета для работы с криптографией и создание исполняемого файла из исходника: "pip install cryptography pyinstaller".
3) Создаём исполняемый файл командой "pyinstaller --onefile Keygen_Pepper_Hard_Passwd.py".

Создасться 2 папки "build" и "dist" и один файл с расширением ".spec" (потом их можно удалить).
Исполняемый файл будет в папке "dist", который можно запустить на любом ПК без установки Python.

При запуске исполняемого файла, рядом с ним появится "Output_Pepper_Passwd.txt", из которого можно скопировать сгенерированные рандомные ключи.
