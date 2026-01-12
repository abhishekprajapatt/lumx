; LumX Installer Script
; NSIS 3.x

!include "MUI2.nsh"
!include "x64.nsh"
!include "WinVer.nsh"

; Basic Settings
Name "LumX"
OutFile "LumX-v1.0.0-Setup.exe"
InstallDir "$PROGRAMFILES64\LumX"
InstallDirRegKey HKLM "Software\LumX" "Install_Dir"

; Request administrator privileges
RequestExecutionLevel admin

; Pages
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

; Installer Sections
Section "LumX Application" SecApp
  SetOutPath "$INSTDIR"
  
  ; Copy executable
  File "bin\Release\LumX.exe"
  
  ; Create Start Menu shortcuts
  CreateDirectory "$SMPROGRAMS\LumX"
  CreateShortcut "$SMPROGRAMS\LumX\LumX.lnk" "$INSTDIR\LumX.exe"
  CreateShortcut "$SMPROGRAMS\LumX\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  
  ; Create Desktop shortcut
  CreateShortcut "$DESKTOP\LumX.lnk" "$INSTDIR\LumX.exe"
  
  ; Write registry keys
  WriteRegStr HKLM "Software\LumX" "Install_Dir" "$INSTDIR"
  WriteRegStr HKLM "Software\LumX" "Version" "1.0.0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LumX" "DisplayName" "LumX"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LumX" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LumX" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LumX" "DisplayVersion" "1.0.0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LumX" "URLInfoAbout" "https://github.com/abhishekprajapatt/lumx"
  
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  ; Start LumX after installation
  SetShellVarContext all
  Exec "$INSTDIR\LumX.exe"
SectionEnd

; Uninstaller
Section "Uninstall"
  SetShellVarContext all
  
  ; Kill running LumX process
  nsExec::Exec "taskkill /IM LumX.exe /F"
  
  ; Remove files
  Delete "$INSTDIR\LumX.exe"
  Delete "$INSTDIR\Uninstall.exe"
  
  ; Remove shortcuts
  Delete "$SMPROGRAMS\LumX\*.*"
  RMDir "$SMPROGRAMS\LumX"
  Delete "$DESKTOP\LumX.lnk"
  
  ; Remove installation directory
  RMDir "$INSTDIR"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\LumX"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LumX"
SectionEnd

; Version Information
VIProductVersion "1.0.0.0"
VIAddVersionKey "ProductName" "LumX"
VIAddVersionKey "ProductVersion" "1.0.0"
VIAddVersionKey "FileDescription" "LumX - macOS-style UI Framework for Windows"
VIAddVersionKey "FileVersion" "1.0.0.0"
VIAddVersionKey "LegalCopyright" "Copyright 2026 Abhishek Prajapatt"
VIAddVersionKey "CompanyName" "Abhishek Prajapatt"
VIAddVersionKey "OriginalFilename" "LumX-v1.0.0-Setup.exe"
