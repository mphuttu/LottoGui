#define MyAppName "LottoGui"
#define MyAppVersion "1.1"
#define MyAppPublisher "Mika Huttunen"
#define MyAppURL "https://github.com/mphuttu/LottoGui"
#define MyAppExeName "LottoGui.exe"
#define MyAppSourceDir "..\x64\Release"
#define MyDataDir "..\data"
#define MyHelpEn "..\help\en\LottoGui_en.chm"
#define MyHelpFi "..\help\LottoGui.chm"

[Setup]
; AppId must remain unchanged across all versions so Inno Setup recognises
; an existing installation and performs an in-place upgrade automatically.
AppId={{BBA74D52-2894-4EC1-87D5-C76A03A0019B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputDir=Output
OutputBaseFilename=LottoGuiSetup_x64
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\{#MyAppExeName}
SetupIconFile=..\LottoGui\res\LottoGuiIcon.ico

; ------------------------------------------------------------------
; Upgrade behaviour
; ------------------------------------------------------------------
; When the same AppId is already installed, Inno Setup will offer to
; upgrade (replace only changed files) without requiring a prior
; uninstall.  CloseApplications=yes closes a running instance so
; the executable can be replaced in place.
CloseApplications=yes
CloseApplicationsFilter=*.exe

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

; ------------------------------------------------------------------
; Let the user choose the help language during installation.
; The selected component installs LottoGui.chm from the English or
; Finnish CHM source so the application always finds the file by its
; fixed name.
; ------------------------------------------------------------------
[Components]
Name: "help"; Description: "Help file"; Types: full compact custom
Name: "help\eng"; Description: "English"; Flags: exclusive; Types: full compact custom
Name: "help\fin"; Description: "Suomeksi (Finnish)"; Flags: exclusive; Types: full compact custom

[Tasks]
Name: "desktopicon"; Description: "Create a desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked

[Registry]
Root: HKLM; Subkey: "SOFTWARE\LottoGui"; ValueType: string; ValueName: "Language"; ValueData: "en"; Components: help\eng; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\LottoGui"; ValueType: string; ValueName: "Language"; ValueData: "fi"; Components: help\fin; Flags: uninsdeletevalue

[Files]
; Main executable – always replaced with the new version.
Source: "{#MyAppSourceDir}\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; Data CSV templates – installed the first time only (when the file
; does not yet exist).  The uninsneveruninstall flag ensures the
; user's own draw data survives both upgrades and uninstallation.
Source: "{#MyDataDir}\EurojackpotData.csv"; DestDir: "{app}\data"; \
    Flags: uninsneveruninstall skipifsourcedoesntexist; \
    Check: not FileExists(ExpandConstant('{app}\data\EurojackpotData.csv'))
Source: "{#MyDataDir}\JokeriData.csv"; DestDir: "{app}\data"; \
    Flags: uninsneveruninstall skipifsourcedoesntexist; \
    Check: not FileExists(ExpandConstant('{app}\data\JokeriData.csv'))
Source: "{#MyDataDir}\KenoData.csv"; DestDir: "{app}\data"; \
    Flags: uninsneveruninstall skipifsourcedoesntexist; \
    Check: not FileExists(ExpandConstant('{app}\data\KenoData.csv'))
Source: "{#MyDataDir}\KTEM.csv"; DestDir: "{app}\data"; \
    Flags: uninsneveruninstall skipifsourcedoesntexist; \
    Check: not FileExists(ExpandConstant('{app}\data\KTEM.csv'))
Source: "{#MyDataDir}\MilliData.csv"; DestDir: "{app}\data"; \
    Flags: uninsneveruninstall skipifsourcedoesntexist; \
    Check: not FileExists(ExpandConstant('{app}\data\MilliData.csv'))
Source: "{#MyDataDir}\SuomenLottoData.csv"; DestDir: "{app}\data"; \
    Flags: uninsneveruninstall skipifsourcedoesntexist; \
    Check: not FileExists(ExpandConstant('{app}\data\SuomenLottoData.csv'))
Source: "{#MyDataDir}\VikingData.csv"; DestDir: "{app}\data"; \
    Flags: uninsneveruninstall skipifsourcedoesntexist; \
    Check: not FileExists(ExpandConstant('{app}\data\VikingData.csv'))

; English help (installed as LottoGui.chm – the name the app expects).
Source: "{#MyHelpEn}"; DestDir: "{app}"; DestName: "LottoGui.chm"; \
    Flags: ignoreversion; Components: help\eng

; Finnish help (installed as LottoGui.chm – the name the app expects).
Source: "{#MyHelpFi}"; DestDir: "{app}"; DestName: "LottoGui.chm"; \
    Flags: ignoreversion; Components: help\fin

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch {#MyAppName}"; Flags: nowait postinstall skipifsilent

; ------------------------------------------------------------------
; Uninstall notes
; ------------------------------------------------------------------
; Data files carry the 'uninsneveruninstall' flag, so the user's CSV
; data is never deleted on uninstall.  The data\ subfolder itself is
; left in place by the installer for the same reason.
; [UninstallDelete] is intentionally empty.
