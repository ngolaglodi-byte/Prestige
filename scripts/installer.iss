; ============================================================
; Prestige AI — Inno Setup Installer Script
; Copyright (c) 2024-2026 Prestige Technologie Company
; All rights reserved.
; ============================================================

#define MyAppName "Prestige AI"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Prestige Technologie Company"
#define MyAppURL "https://github.com/ngolaglodi-byte/Prestige"
#define MyAppExeName "prestige_control.exe"

[Setup]
AppId={{B8F3A2D1-7E4C-4A9B-8D5F-1C2E3F4A5B6C}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=LICENSE.txt
OutputDir=..\dist
OutputBaseFilename=PrestigeAI-{#MyAppVersion}-Setup-Windows-x64
SetupIconFile=..\resources\prestige.ico
UninstallDisplayIcon={app}\prestige.ico
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardImageFile=compiler:WizModernImage-IS.bmp
WizardSmallImageFile=compiler:WizModernSmallImage-IS.bmp

[Languages]
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "startmenu"; Description: "Créer un raccourci dans le menu Démarrer"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkedonce

[Files]
Source: "..\dist\PrestigeAI\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\resources\prestige.ico"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\PrestigeAI.bat"; IconFilename: "{app}\prestige.ico"; Comment: "Prestige AI — Broadcast Overlay System"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\PrestigeAI.bat"; IconFilename: "{app}\prestige.ico"; Tasks: desktopicon; Comment: "Prestige AI — Broadcast Overlay System"

[Run]
Filename: "{app}\PrestigeAI.bat"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent shellexec

[UninstallDelete]
Type: filesandordirs; Name: "{app}"

[Code]
procedure InitializeWizard;
begin
  WizardForm.WelcomeLabel1.Caption := 'Bienvenue dans l''installation de Prestige AI';
  WizardForm.WelcomeLabel2.Caption :=
    'Ce programme va installer Prestige AI v' + '{#MyAppVersion}' + ' sur votre ordinateur.' + #13#10 + #13#10 +
    'Prestige AI est un logiciel professionnel de broadcast avec reconnaissance faciale IA.' + #13#10 + #13#10 +
    '© 2024-2026 Prestige Technologie Company' + #13#10 +
    'Tous droits réservés.';
end;
