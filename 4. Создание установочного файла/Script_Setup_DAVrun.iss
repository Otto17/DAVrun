;Сценарий, установки программы "DAVrun" создан через Inno Setup v6.3.1


;Данная программа является свободным программным обеспечением, распространяющимся по лицензии MIT.
;Копия лицензии: https://opensource.org/licenses/MIT

;Copyright (c) 2024 Otto
;Автор: Otto
;Версия: 20.06.24
;GitHub страница:  https://github.com/Otto17/DAVrun
;GitFlic страница: https://gitflic.ru/project/otto/davrun

;г. Омск 2024



; Имя приложения
#define MyAppName "DAVrun"
; Версия приложения
#define MyAppVersion "20.06.2024"
;Издатель приложения
#define MyAppPublisher "Otto"
; Место сохранения скомпилированного установочного фала
#define MyFolderOutput "D:\build"
; Папка с файлами, которые будут собраны в установочном файле
#define MySourceFile "D:\build\DAVrun\*"
; Файл лицензии
#define MyLicense "D:\build\DAVrun\LICENSE.txt"
; Иконка установочного файла после компиляции
#define MyIconSetupOutput "D:\build\icon_Setup.ico"
; Имя файла установщика после компиляции
#define MySetupOutput "Setup_DAVrun"
; Место куда будем устанавливать программу при запуске Setup файла
#define MySetupFilePath "C:\Program Files"


[Setup]
; ПРИМЕЧАНИЕ: Значение AppID однозначно идентифицирует это приложение. Не используйте одно и то же значение AppID в установщиках для других приложений.
; ( Чтобы сгенерировать новый GUID, нажмите Tools |  Generate GUID в среде IDE )
AppId={{7A5F84AE-20D3-4755-86DD-9C89B4B49B3D}
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
UninstallDisplayIcon={#MyIconSetupOutput}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin


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

; После установки DAVrun устанавливаем и запускаем службу с ключом "-is"
Filename: "{app}\SERVICE-DAVrun.exe"; Parameters: "-is"; Flags: runascurrentuser nowait postinstall


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