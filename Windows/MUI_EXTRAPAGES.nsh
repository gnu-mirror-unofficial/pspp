#   MUI_EXTRAPAGES.nsh
#   By Red Wine Jan 2007

!verbose push
!verbose 3

!ifndef _MUI_EXTRAPAGES_NSH
!define _MUI_EXTRAPAGES_NSH

!ifmacrondef MUI_EXTRAPAGE_README & MUI_PAGE_README & MUI_UNPAGE_README & ReadmeLangStrings

!macro MUI_EXTRAPAGE_README UN ReadmeFile
!verbose push
!verbose 3
   !define MUI_PAGE_HEADER_TEXT "Read Me"
   !define MUI_PAGE_HEADER_SUBTEXT "Please read the following important information!"
   !define MUI_LICENSEPAGE_TEXT_TOP ""
   !define MUI_LICENSEPAGE_TEXT_BOTTOM "Click on scrollbar arrows or press Page Down to review the entire text."
   !define MUI_LICENSEPAGE_BUTTON "$(^NextBtn)"
   !insertmacro MUI_${UN}PAGE_LICENSE "${ReadmeFile}"
!verbose pop
!macroend

!define ReadmeRun "!insertmacro MUI_EXTRAPAGE_README"


!macro MUI_PAGE_README ReadmeFile
!verbose push
!verbose 3
    ${ReadmeRun} "" "${ReadmeFile}"
!verbose pop
!macroend


!macro MUI_UNPAGE_README ReadmeFile
!verbose push
!verbose 3
    ${ReadmeRun} "UN" "${ReadmeFile}"
!verbose pop
!macroend


!macro ReadmeLangStrings UN MUI_LANG ReadmeHeader ReadmeSubHeader ReadmeTextTop ReadmeTextBottom
!verbose push
!verbose 3
    LangString ${UN}ReadmeHeader     ${MUI_LANG} "${ReadmeHeader}"
    LangString ${UN}ReadmeSubHeader  ${MUI_LANG} "${ReadmeSubHeader}"
    LangString ${UN}ReadmeTextTop    ${MUI_LANG} "${ReadmeTextTop}"
    LangString ${UN}ReadmeTextBottom ${MUI_LANG} "${ReadmeTextBottom}"
!verbose pop
!macroend

!define ReadmeLanguage `!insertmacro ReadmeLangStrings ""`

!define Un.ReadmeLanguage `!insertmacro ReadmeLangStrings "UN"`

!endif
!endif

!verbose pop
