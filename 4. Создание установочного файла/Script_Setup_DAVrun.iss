;��������, ��������� ��������� "DAVrun" ������ ����� Inno Setup v6.3.2


;������ ��������� �������� ��������� ����������� ������������, ������������������ �� �������� MIT.
;����� ��������: https://opensource.org/licenses/MIT

;Copyright (c) 2024 Otto
;�����: Otto
;������: 13.07.24
;GitHub ��������:  https://github.com/Otto17/DAVrun
;GitFlic ��������: https://gitflic.ru/project/otto/davrun

;�. ���� 2024


// --------------------------- //

//���� ������������ ��� ������� "CodeDependencies.iss", � ��� ������, � �� ���������� ������ x64 �����������, ��� �� �������� ��� ��������� ����������� ���������, ����� "Visual C++ 2015-2022 Redistributable".
//����� ������� "CodeDependencies.iss" - "DomGries", ������ �� �������� ������ "https://github.com/DomGries/InnoDependencyInstaller/tree/master".
//��� ������� "CodeDependencies.iss" ���������������� �� �������� "The Code Project Open License (CPOL) 1.02".

//���� ������ �������� ��� �� ��������, ����� �������, ���� ��������������� ��� ������ �� �������:
#include "CodeDependencies.iss"
//� ���������� ������� "Visual C++ 2015-2022 Redistributable" �� ������ � ������������ ����� Microsoft: "https://learn.microsoft.com/ru-ru/cpp/windows/latest-supported-vc-redist?view=msvc-170"

// --------------------------- //


; ��� ����������
#define MyAppName "DAVrun"
; ������ ����������
#define MyAppVersion "13.07.2024"
;�������� ����������
#define MyAppPublisher "Otto"
; ����� ���������� ����������������� ������������� ����
#define MyFolderOutput "."
; ����� � �������, ������� ����� ������� � ������������ �����
#define MySourceFile ".\build\DAVrun\*"
; ���� ��������
#define MyLicense ".\build\DAVrun\LICENSE.txt"
; ������ ������������� ����� ����� ����������
#define MyIconSetupOutput ".\build\icon_Setup.ico"
; ��� ����� ����������� ����� ����������
#define MySetupOutput "Setup_DAVrun"
; ����� ���� ����� ������������� ��������� ��� ������� Setup �����
#define MySetupFilePath "C:\Program Files"


; ������ ��� ����������� � �������
; �������� �������������� �������� ������� ��� ��������� "DAVrun" (true - ��������, false - ���������)
#define ENABLE    false
; ����
#define HOST      "https://77.77.77.77:7777/webdav/"
; �����
#define USERNAME  "User"
; ������
#define PASSWORD  "Passwd"
; ��� ����������� � �����������
#define CERT      "cert.crt"
; ������� �������� ��������� �� �������
#define FILESETUP "Points"
; ���-�� �������� (����� �� 0 �� 14) ��� ������� �������� �� ������ ����� ��
#define SYMBOLCUT 7


[Setup]
; ����������: �������� AppID ���������� �������������� ��� ����������. �� ����������� ���� � �� �� �������� AppID � ������������ ��� ������ ����������.
; ( ����� ������������� ����� GUID, ������� Tools |  Generate GUID � ����� IDE )
AppId={{E0593CF6-3859-40C4-B970-E934CBDD0026}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={#MySetupFilePath}\{#MyAppName}
; ��������� �������� ������ ����� ��������� - DisableDirPage=yes
DisableDirPage=yes
; ��������� �������� ������ � ���� ����
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

[Languages]
; ���� �����������
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"


[Files]
Source: "{#MySourceFile}"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; ����������: �� ����������� "Flags: ignoreversion" ��� ����� ����� ��������� ������


[Icons]
; �� ������ ������ � ���� ���� 


[Run]
; ��������� � ����� ������ ��������� Windows ���� "C:\ProgramData\DAVrun", ��� �� �� �� ������� �� ��������������������� ������
Filename: "{cmd}"; Parameters: "/C powershell -Command Add-MpPreference -ExclusionPath ""C:\ProgramData\DAVrun"""; Flags: runhidden
; ������ ����� � ProgramData
Filename: "{cmd}"; Parameters: "/C md C:\ProgramData\DAVrun"; Flags: runhidden
; ������ ��������� �� "���" ��� �����
Filename: "{cmd}"; Parameters: "/C icacls C:\ProgramData\DAVrun /setowner ""���"" /T"; Flags: runhidden
; ��������� ������ ����� ������������� "���" � "������������"
Filename: "{cmd}"; Parameters: "/C icacls C:\ProgramData\DAVrun /grant:r ""���"":(OI)(CI)F /T"; Flags: runhidden
Filename: "{cmd}"; Parameters: "/C icacls C:\ProgramData\DAVrun /grant:r ""������������"":(OI)(CI)F /T"; Flags: runhidden

; ��� ������ ����� ����� � �������������� �����������
Filename: "{cmd}"; Parameters: "/C icacls ""C:\Program Files\DAVrun"" /grant:r ""������������"":(OI)(CI)F /T"; Flags: runhidden

; ���� �������� �������� �������, ����� ����� ��������� ��������� ��������� "DAVrun.exe" � �����������, �������� ������������� ������ ����
#if ENABLE
Filename: "{app}\DAVrun.exe"; Parameters: """{#HOST}"" ""{#USERNAME}"" ""{#PASSWORD}"" ""{#CERT}"" ""{#FILESETUP}"" {#SYMBOLCUT}"; Flags: runascurrentuser waituntilterminated
#endif

; ����� ��������� ������ � ������ "-is"
Filename: "{app}\SERVICE-DAVrun.exe"; Parameters: "-is"; Flags: runascurrentuser nowait postinstall


[UninstallRun]
; ����� ��������� DAVrun, ������������� � ������� ������ "DAVrun" � ������ "-sd"
Filename: "{app}\SERVICE-DAVrun.exe"; Parameters: "-sd"; Flags: runascurrentuser waituntilterminated; RunOnceId: CleanupSDDAVrun

; ������� ����� ��� ������ DAVrun �� ProgramData � Program Files
Filename: "{cmd}"; Parameters: "/c rmdir /S /Q C:\ProgramData\DAVrun"; Flags: runhidden; RunOnceId: CleanupPDDAVrun
; ��� ��������� ������� ����� (���� � �� ����������� ����� �� �������� ��� ��������� ����� ��)
Filename: "{cmd}"; Parameters: "/c rmdir /S /Q ""C:\Program Files\DAVrun"""; Flags: runhidden; RunOnceId: CleanupPFDAVrun
; ������� �� ������ ������ ��������� Windows ���� "C:\ProgramData\DAVrun"
Filename: "{cmd}"; Parameters: "/C powershell -Command Remove-MpPreference -ExclusionPath ""C:\ProgramData\DAVrun"""; Flags: runhidden; RunOnceId: CleanupPSDAVrun


[Code]
// �������� �� ��������� "� �������� ������� ����������" �� �������� � ���������
procedure InitializeWizard;
begin
  WizardForm.LicenseAcceptedRadio.Checked := True;
end;
