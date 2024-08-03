;��������, ��������� ��������� "DAVrun" ������ ����� Inno Setup v6.3.3


;������ ��������� �������� ��������� ����������� ������������, ������������������ �� �������� MIT.
;����� ��������: https://opensource.org/licenses/MIT

;Copyright (c) 2024 Otto
;�����: Otto
;������: 02.08.24
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
#define MyAppVersion "02.08.24"
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
#define ENABLE true
; ������� ������ ���� �� ������������ "LocalSystem"(�������) (true - ������� �� "�������", false - ������� �� �������� ������������)
#define CreateConfLocalSystem false
; ����
#define HOST      "https://77.77.77.77:7777/webdav/"
; �����
#define USERNAME  "User"
; ������
#define PASSWORD  "Password"
; ��� ����������� � �����������
#define CERT      "cert.crt"
; ������� �������� ��������� �� �������
#define FILESETUP "Points"
; ���-�� �������� (����� �� 0 �� 14) ��� ������� �������� �� ������ ����� ��
#define SYMBOLCUT 7


[Setup]
; ����������: �������� AppID ���������� �������������� ��� ����������. �� ����������� ���� � �� �� �������� AppID � ������������ ��� ������ ����������.
; ( ����� ������������� ����� GUID, ������� Tools |  Generate GUID � ����� IDE )
AppId={{F64A2126-4A3C-4CEF-830B-7E882F6097D1}
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
; ��������� ����������� ������������� ��������� ����� ��������� ���������
RestartIfNeededByRun=no


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

; ���� �������� �������� �������, ����� ����� ��������� ��������� �������� "CreateConfLocalSystem" � ������ ������������� ������ ���� �� ����� "�������" ��� �������� ������������
#if ENABLE
  #if CreateConfLocalSystem
    ; �������� ������� �� ����� "�������"
    Filename: "{app}\Launch_LocalSystem.exe"; Parameters: """DAVrun.exe {#HOST} {#USERNAME} {#PASSWORD} {#CERT} {#FILESETUP} {#SYMBOLCUT}"""; Flags: runascurrentuser waituntilterminated
  #else
    ; �������� ������� �� �������� ������������
    Filename: "{app}\DAVrun.exe"; Parameters: """{#HOST}"" ""{#USERNAME}"" ""{#PASSWORD}"" ""{#CERT}"" ""{#FILESETUP}"" {#SYMBOLCUT}"; Flags: runascurrentuser waituntilterminated
  #endif
#endif


; ����� ��������� ������ � ������ "-is"
Filename: "{app}\SERVICE-DAVrun.exe"; Parameters: "-is"; Flags: runascurrentuser waituntilterminated


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


// �������� � �������� �� ��������� "���, � ��������� ������������ �����" �� �������� ����������
procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpFinished then
  begin
    WizardForm.PreparingNoRadio.Checked := True;
    WizardForm.NoRadio.Checked := True;
  end;
end;
