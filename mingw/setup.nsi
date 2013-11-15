# NSI script for Freeacq
#
# Written by
# Víctor Enríquez Miguel < victor dot quicksilver att GM >
# based on the installer.nsi created by Tadej Boroviak wich
# can be found in the following url:
# http://www.gtkforums.com/viewtopic.php?p=6528

# Version, Name and Filename
!define VERSION "0.0.1"
Name    "Freeacq ${VERSION}"
OutFile "Freeacq Setup - ${VERSION}.exe"

# Required includes
!include "MUI2.nsh"

# Default Install Dir
InstallDir $PROGRAMFILES\Freeacq-${VERSION}

# Check registry values for installation folder
InstallDirRegKey HKCU "Software\Freeacq" ""

# Application privileges for Windows Vista
RequestExecutionLevel user

# Interface Settings
!define MUI_ABORTWARNING

# Setup interface settings
; Store installation language
!define MUI_LANGDLL_REGISTRY_ROOT "HKCU"
!define MUI_LANGDLL_REGISTRY_KEY "Software\Freeacq"
!define MUI_LANGDLL_REGISTRY_VALUENAME "InstallerLanguage"

/* Pages */
; Installer
;   !insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Docs\Modern UI\License.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

; Uninstaller
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

/* Languages */
# List available languages
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Spanish"

/* Language strings */
# English
LangString seclFreeacq   ${LANG_ENGLISH} "Freeacq Suite"
LangString descFreeacq   ${LANG_ENGLISH} "Includes programs and libraries"
LangString typeFull      ${LANG_ENGLISH} "Full"
#LangString typeCustom    ${LANG_ENGLISH} "Custom"
LangString secDesktopSC  ${LANG_ENGLISH} "Desktop shortcuts"
LangString descDesktopSC ${LANG_ENGLISH} "Create desktop shortcuts"
LangString seclCapture   ${LANG_ENGLISH} "Freeacq Capture"
LangString descCapture   ${LANG_ENGLISH} "Create desktop shortcut for Capture"
LangString seclOscope    ${LANG_ENGLISH} "Freeacq Oscilloscope"
LangString descOscope    ${LANG_ENGLISH} "Create desktop shortcut for Oscilloscope"
LangString seclViewer    ${LANG_ENGLISH} "Freeacq Viewer"
LangString descViewer    ${LANG_ENGLISH} "Create a desktop shortcut for Viewer"
LangString seclPlethy    ${LANG_ENGLISH} "Freeacq Plethysmograph"
LangString descPlethy    ${LANG_ENGLISH} "Create a desktop shortcut for Plethysmograph"

# Spanish
LangString seclFreeacq   ${LANG_SPANISH} "Suite Freeacq"
LangString descFreeacq   ${LANG_SPANISH} "Incluye programas y bibliotecas"
LangString typeFull      ${LANG_SPANISH} "Completa"
#LangString typeCustom    ${LANG_SPANISH} "Personalizada"
LangString secDesktopSC  ${LANG_SPANISH} "Accesos directos"
LangString seclCapture   ${LANG_SPANISH} "Freeacq Capture"
LangString descCapture   ${LANG_SPANISH} "Crear acceso directo a Capture"
LangString seclOscope    ${LANG_SPANISH} "Freeacq Oscilloscope"
LangString descOscope    ${LANG_SPANISH} "Crear acceso directo a Oscilloscope"
LangString seclViewer    ${LANG_SPANISH} "Freeacq Viewer"
LangString descViewer    ${LANG_SPANISH} "Crear acceso directo a Viewer"
LangString seclPlethy    ${LANG_SPANISH} "Freeacq Plethysmograph"
LangString descPlethy    ${LANG_SPANISH} "Crear acceso directo a Plethysmograph"


/* Reserve files */
!insertmacro MUI_RESERVEFILE_LANGDLL

/* Installation types */
InstType $(typeFull)
#InstType $(typeCustom)

/* Sections */
# Freeacq files
Section !$(seclFreeacq) SecFreeacq
	SectionIn 1 RO
	; Files will be copied to the installation dir
	SetOutPath "$INSTDIR"

	; Specify Freeacq files here
	File /r "freeacq"

	; Store installation folder
	WriteRegStr HKCU "Software\Freeacq" "" $INSTDIR

	; Create a new Folder for Freeacq in the start menu
	CreateDirectory "$SMPROGRAMS\Freeacq"

	; Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"

	; Create shortcuts
	CreateShortCut "$SMPROGRAMS\Freeacq\Capture.lnk" "$INSTDIR\freeacq\bin\facqcapture.exe"
	
	CreateShortCut "$SMPROGRAMS\Freeacq\Oscilloscope.lnk" "$INSTDIR\freeacq\bin\facqoscilloscope.exe"
	
	CreateShortCut "$SMPROGRAMS\Freeacq\Viewer.lnk" "$INSTDIR\freeacq\bin\facqviewer.exe"

	CreateShortCut "$SMPROGRAMS\Freeacq\Plethysmograph.lnk" "$INSTDIR\freeacq\bin\facqplethysmograph.exe"

	CreateShortCut "$SMPROGRAMS\Freeacq\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

# Desktop Icons
SectionGroup /e $(secDesktopSC)
Section !$(seclCapture) SecCapture
	SectionIn 1
	CreateShortCut "$DESKTOP\Capture.lnk" "$INSTDIR\freeacq\bin\facqcapture.exe"
SectionEnd
Section !$(seclOscope) SecOscope
	SectionIn 1
	CreateShortCut "$DESKTOP\Oscilloscope.lnk" "$INSTDIR\freeacq\bin\facqoscilloscope.exe"
SectionEnd
Section !$(seclViewer) SecViewer
	SectionIn 1
	CreateShortCut "$DESKTOP\Viewer.lnk" "$INSTDIR\freeacq\bin\facqviewer.exe"
SectionEnd
Section !$(seclPlethy) SecPlethy
	SectionIn 1
	CreateShortCut "$DESKTOP\Plethysmograph.lnk" "$INSTDIR\freeacq\bin\facqplethysmograph.exe"
SectionEnd
SectionGroupEnd

/* Installer Functions */
# .onInit function
Function .onInit
	!insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

/* Section descriptions */
# Assign descriptions to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${SecFreeacq} $(descFreeacq)
	!insertmacro MUI_DESCRIPTION_TEXT ${secCapture} $(descCapture)
	!insertmacro MUI_DESCRIPTION_TEXT ${secOscope}  $(descOscope)
	!insertmacro MUI_DESCRIPTION_TEXT ${secViewer}  $(descViewer)
	!insertmacro MUI_DESCRIPTION_TEXT ${secPlethy}  $(descPlethy)
!insertmacro MUI_FUNCTION_DESCRIPTION_END


/* Uninstaller */
Section "Uninstall"
   
	; Desktop Icons
	Delete "$DESKTOP\Capture.lnk"
	Delete "$DESKTOP\Oscilloscope.lnk"
	Delete "$DESKTOP\Viewer.lnk"
	Delete "$DESKTOP\Plethysmograph.lnk"

	; Start menu
	RMDir /r "$SMPROGRAMS\Freeacq"

	; Freeacq and libraries
	RMDir /r "$INSTDIR"

	DeleteRegKey HKCU "Software\Freeacq"
SectionEnd

/* Uninstaller Functions */
# un.onInit function
Function un.onInit
	!insertmacro MUI_UNGETLANGUAGE
FunctionEnd
