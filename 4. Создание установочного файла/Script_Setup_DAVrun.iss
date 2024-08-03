;Сценарий, установки программы "DAVrun" создан через Inno Setup v6.3.3


;Данная программа является свободным программным обеспечением, распространяющимся по лицензии MIT.
;Копия лицензии: https://opensource.org/licenses/MIT

;Copyright (c) 2024 Otto
;Автор: Otto
;Версия: 02.08.24
;GitHub страница:  https://github.com/Otto17/DAVrun
;GitFlic страница: https://gitflic.ru/project/otto/davrun

;г. Омск 2024


// --------------------------- //

//Ниже используется код скрипта "CodeDependencies.iss", я его урезал, и он использует только x64 архитектура, так же вырезаны все остальные установщики библиотек, кроме "Visual C++ 2015-2022 Redistributable".
//Автор скрипта "CodeDependencies.iss" - "DomGries", ссылка на страницу автора "https://github.com/DomGries/InnoDependencyInstaller/tree/master".
//Код скрипта "CodeDependencies.iss" распространяется по лицензии "The Code Project Open License (CPOL) 1.02".

//ЕСЛИ ДАННАЯ ЛИЦЕНЗИЯ ВАМ НЕ ПОДХОДИТ, ТОГДА УДАЛИТЕ, ЛИБО ЗАКОММЕНТИРУЙТЕ ЭТУ СТРОКУ ИЗ СКРИПТА:
#include "CodeDependencies.iss"
//И установите вручную "Visual C++ 2015-2022 Redistributable" по ссылке с официального сайта Microsoft: "https://learn.microsoft.com/ru-ru/cpp/windows/latest-supported-vc-redist?view=msvc-170"

// --------------------------- //


; Имя приложения
#define MyAppName "DAVrun"
; Версия приложения
#define MyAppVersion "02.08.24"
;Издатель приложения
#define MyAppPublisher "Otto"
; Место сохранения скомпилированного установочного фала
#define MyFolderOutput "."
; Папка с файлами, которые будут собраны в установочном файле
#define MySourceFile ".\build\DAVrun\*"
; Файл лицензии
#define MyLicense ".\build\DAVrun\LICENSE.txt"
; Иконка установочного файла после компиляции
#define MyIconSetupOutput ".\build\icon_Setup.ico"
; Имя файла установщика после компиляции
#define MySetupOutput "Setup_DAVrun"
; Место куда будем устанавливать программу при запуске Setup файла
#define MySetupFilePath "C:\Program Files"


; ДАННЫЕ ДЛЯ ПОДКЛЮЧЕНИЯ К СЕРВЕРУ
; Включить автоматическое создание конфига для программы "DAVrun" (true - включить, false - выключить)
#define ENABLE true
; Создать конфиг файл от пользователя "LocalSystem"(СИСТЕМА) (true - создать от "СИСТЕМА", false - создать от текущего пользователя)
#define CreateConfLocalSystem false
; Хост
#define HOST      "https://77.77.77.77:7777/webdav/"
; Логин
#define USERNAME  "User"
; Пароль
#define PASSWORD  "Password"
; Имя сертификата с расширением
#define CERT      "cert.crt"
; Каталог ожидания установок на сервере
#define FILESETUP "Points"
; Кол-во символов (число от 0 до 14) для обрезки символов от начала имени ПК
#define SYMBOLCUT 7


[Setup]
; ПРИМЕЧАНИЕ: Значение AppID однозначно идентифицирует это приложение. Не используйте одно и то же значение AppID в установщиках для других приложений.
; ( Чтобы сгенерировать новый GUID, нажмите Tools |  Generate GUID в среде IDE )
AppId={{F64A2126-4A3C-4CEF-830B-7E882F6097D1}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={#MySetupFilePath}\{#MyAppName}
; Отключаем страницу выбора папки установки - DisableDirPage=yes
DisableDirPage=yes
; Отключаем создание группы в Меню ПУСК
DisableProgramGroupPage=yes
LicenseFile={#MyLicense}
OutputDir={#MyFolderOutput}
OutputBaseFilename={#MySetupOutput}
SetupIconFile={#MyIconSetupOutput}
UninstallDisplayIcon={app}\unins000.exe,0
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64os
; Отключаем предложение перезагрузить компьютер после установки программы
RestartIfNeededByRun=no


[Languages]
; Язык установщика
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"


[Files]
Source: "{#MySourceFile}"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; ПРИМЕЧАНИЕ: Не используйте "Flags: ignoreversion" для любых общих системных файлов


[Icons]
; Не создаём группу в Меню ПУСК 


[Run]
; Добавляем в белый список Защитника Windows путь "C:\ProgramData\DAVrun", что бы он не ругался на самораспаковывающиеся архивы
Filename: "{cmd}"; Parameters: "/C powershell -Command Add-MpPreference -ExclusionPath ""C:\ProgramData\DAVrun"""; Flags: runhidden
; Создаём папку в ProgramData
Filename: "{cmd}"; Parameters: "/C md C:\ProgramData\DAVrun"; Flags: runhidden
; Меняем владельца на "Все" для папки
Filename: "{cmd}"; Parameters: "/C icacls C:\ProgramData\DAVrun /setowner ""Все"" /T"; Flags: runhidden
; Добавляем полные права пользователям "Все" и "Пользователи"
Filename: "{cmd}"; Parameters: "/C icacls C:\ProgramData\DAVrun /grant:r ""все"":(OI)(CI)F /T"; Flags: runhidden
Filename: "{cmd}"; Parameters: "/C icacls C:\ProgramData\DAVrun /grant:r ""Пользователи"":(OI)(CI)F /T"; Flags: runhidden

; Даём полные права папке с установленными программами
Filename: "{cmd}"; Parameters: "/C icacls ""C:\Program Files\DAVrun"" /grant:r ""Пользователи"":(OI)(CI)F /T"; Flags: runhidden

; Если включено создание конфига, тогда после установки проверяем значение "CreateConfLocalSystem" и создаём зашифрованный конфиг файл от имени "СИСТЕМА" или текущего пользователя
#if ENABLE
  #if CreateConfLocalSystem
    ; Создание конфига от имени "СИСТЕМА"
    Filename: "{app}\Launch_LocalSystem.exe"; Parameters: """DAVrun.exe {#HOST} {#USERNAME} {#PASSWORD} {#CERT} {#FILESETUP} {#SYMBOLCUT}"""; Flags: runascurrentuser waituntilterminated
  #else
    ; Создание конфига от текущего пользователя
    Filename: "{app}\DAVrun.exe"; Parameters: """{#HOST}"" ""{#USERNAME}"" ""{#PASSWORD}"" ""{#CERT}"" ""{#FILESETUP}"" {#SYMBOLCUT}"; Flags: runascurrentuser waituntilterminated
  #endif
#endif


; Затем запускаем службу с ключом "-is"
Filename: "{app}\SERVICE-DAVrun.exe"; Parameters: "-is"; Flags: runascurrentuser waituntilterminated


[UninstallRun]
; Перед удалением DAVrun, останавливаем и удаляем службу "DAVrun" с ключом "-sd"
Filename: "{app}\SERVICE-DAVrun.exe"; Parameters: "-sd"; Flags: runascurrentuser waituntilterminated; RunOnceId: CleanupSDDAVrun

; Удаляем папку для темпов DAVrun из ProgramData и Program Files
Filename: "{cmd}"; Parameters: "/c rmdir /S /Q C:\ProgramData\DAVrun"; Flags: runhidden; RunOnceId: CleanupPDDAVrun
; Для надёжности удаляем папку (если в нём создавались файлы из программ для получения имени ПК)
Filename: "{cmd}"; Parameters: "/c rmdir /S /Q ""C:\Program Files\DAVrun"""; Flags: runhidden; RunOnceId: CleanupPFDAVrun
; Удаляем из белого списка Защитника Windows путь "C:\ProgramData\DAVrun"
Filename: "{cmd}"; Parameters: "/C powershell -Command Remove-MpPreference -ExclusionPath ""C:\ProgramData\DAVrun"""; Flags: runhidden; RunOnceId: CleanupPSDAVrun


[Code]
// Выбираем по умолчанию "Я принимаю условия соглашения" на странице с лицензией
procedure InitializeWizard;
begin
  WizardForm.LicenseAcceptedRadio.Checked := True;
end;


// Выделяем и выбираем по умолчанию "Нет, я произведу перезагрузку позже" на странице завершения
procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpFinished then
  begin
    WizardForm.PreparingNoRadio.Checked := True;
    WizardForm.NoRadio.Checked := True;
  end;
end;
