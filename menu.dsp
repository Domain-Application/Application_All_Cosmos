# Microsoft Developer Studio Project File - Name="menu" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=menu - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "menu.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "menu.mak" CFG="menu - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "menu - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "menu - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "Desktop"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "menu - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W2 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 wlib32.lib wininet.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib"
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "menu - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dlib32.lib /nologo /stack:0x100000,0x100000 /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"mfc42.lib" /nodefaultlib:"mfcs42.lib" /nodefaultlib:"msvcrt.lib" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "menu - Win32 Release"
# Name "menu - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ACCODE.CPP
# End Source File
# Begin Source File

SOURCE=.\ADAM.CPP
# End Source File
# Begin Source File

SOURCE=.\BARRIER.CPP
# End Source File
# Begin Source File

SOURCE=.\Category.cpp
# End Source File
# Begin Source File

SOURCE=.\ChangeWeighingType.cpp
# End Source File
# Begin Source File

SOURCE=.\CLIENT.CPP
# End Source File
# Begin Source File

SOURCE=.\Export4ASCII.cpp
# End Source File
# Begin Source File

SOURCE=.\FastSearch.cpp
# End Source File
# Begin Source File

SOURCE=.\INIT_WB.CPP
# End Source File
# Begin Source File

SOURCE=.\IT1.CPP
# End Source File
# Begin Source File

SOURCE=.\IT2.CPP
# End Source File
# Begin Source File

SOURCE=.\LOGERR.CPP
# End Source File
# Begin Source File

SOURCE=.\Logo.cpp
# End Source File
# Begin Source File

SOURCE=.\LORRY.CPP
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\menu.cpp
# End Source File
# Begin Source File

SOURCE=.\menu.rc
# End Source File
# Begin Source File

SOURCE=.\MENUVIEW.CPP
# End Source File
# Begin Source File

SOURCE=.\NVIEW.CPP
# End Source File
# Begin Source File

SOURCE=.\PACKING.CPP
# End Source File
# Begin Source File

SOURCE=.\PASS.CPP
# End Source File
# Begin Source File

SOURCE=.\PMTDAT.CPP
# End Source File
# Begin Source File

SOURCE=.\PMWeighing.cpp
# End Source File
# Begin Source File

SOURCE=.\PROFILE.CPP
# End Source File
# Begin Source File

SOURCE=.\ResetPassword.cpp
# End Source File
# Begin Source File

SOURCE=.\SECURITY.CPP
# End Source File
# Begin Source File

SOURCE=.\SENSOR.CPP
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\tcpServer.cpp
# End Source File
# Begin Source File

SOURCE=.\TriggerLPR.cpp
# End Source File
# Begin Source File

SOURCE=.\Wb.cpp
# End Source File
# Begin Source File

SOURCE=.\WBDATA.CPP
# End Source File
# Begin Source File

SOURCE=.\weighbridge.CPP
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\DateTimeDuration.h
# End Source File
# Begin Source File

SOURCE=.\DateTimePeriod.h
# End Source File
# Begin Source File

SOURCE=.\Export4ASCII.h
# End Source File
# Begin Source File

SOURCE=.\MENU.H
# End Source File
# Begin Source File

SOURCE=.\NVIEW.H
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\RES\14.ico
# End Source File
# Begin Source File

SOURCE=.\RES\17.ico
# End Source File
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\CURSOR1.CUR
# End Source File
# Begin Source File

SOURCE=.\res\hotbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00003.ico
# End Source File
# Begin Source File

SOURCE=.\RES\icon3.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon7.ico
# End Source File
# Begin Source File

SOURCE=.\RES\MENUDOC.ICO
# End Source File
# Begin Source File

SOURCE=.\RES\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\truck.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\DLL1.dll
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
# Section menu : {8BD21D13-EC42-11CE-9E0D-00AA006002F3}
# 	2:5:Class:CMdcText
# 	2:10:HeaderFile:mdctext.h
# 	2:8:ImplFile:mdctext.cpp
# End Section
# Section menu : {B09DE713-87C1-11D1-8BE3-0000F8754DA1}
# 	2:5:Class:CAnimation
# 	2:10:HeaderFile:animation.h
# 	2:8:ImplFile:animation.cpp
# End Section
# Section menu : {978C9E23-D4B0-11CE-BF2D-00AA003F40D0}
# 	2:21:DefaultSinkHeaderFile:labelcontrol1.h
# 	2:16:DefaultSinkClass:CLabelControl
# End Section
# Section menu : {04598FC4-866C-11CF-AB7C-00AA00C08FCF}
# 	2:5:Class:CCommandButton
# 	2:10:HeaderFile:commandbutton.h
# 	2:8:ImplFile:commandbutton.cpp
# End Section
# Section menu : {72ADFD7B-2C39-11D0-9903-00A0C91BC942}
# 	1:17:ID_INDICATOR_TIME:103
# 	1:17:ID_INDICATOR_DATE:102
# 	2:2:BH:
# 	2:17:ID_INDICATOR_TIME:ID_INDICATOR_TIME
# 	2:17:ID_INDICATOR_DATE:ID_INDICATOR_DATE
# End Section
# Section menu : {5F4DF280-531B-11CF-91F6-C2863C385E30}
# 	2:5:Class:CMSFlexGrid
# 	2:10:HeaderFile:msflexgrid.h
# 	2:8:ImplFile:msflexgrid.cpp
# End Section
# Section menu : {6262D3A0-531B-11CF-91F6-C2863C385E30}
# 	2:21:DefaultSinkHeaderFile:msflexgrid.h
# 	2:16:DefaultSinkClass:CMSFlexGrid
# End Section
# Section menu : {BEF6E003-A874-101A-8BBA-00AA00300CAB}
# 	2:5:Class:COleFont
# 	2:10:HeaderFile:font1.h
# 	2:8:ImplFile:font1.cpp
# End Section
# Section menu : {4D6CC9A0-DF77-11CF-8E74-00A0C90F26F8}
# 	2:5:Class:CMSMask
# 	2:10:HeaderFile:msmask1.h
# 	2:8:ImplFile:msmask1.cpp
# End Section
# Section menu : {D7053240-CE69-11CD-A777-00DD01143C57}
# 	2:21:DefaultSinkHeaderFile:commandbutton.h
# 	2:16:DefaultSinkClass:CCommandButton
# End Section
# Section menu : {7BF80981-BF32-101A-8BBB-00AA00300CAB}
# 	2:5:Class:CPicture
# 	2:10:HeaderFile:picture1.h
# 	2:8:ImplFile:picture1.cpp
# End Section
# Section menu : {B09DE715-87C1-11D1-8BE3-0000F8754DA1}
# 	2:21:DefaultSinkHeaderFile:animation.h
# 	2:16:DefaultSinkClass:CAnimation
# End Section
# Section menu : {8BD21D10-EC42-11CE-9E0D-00AA006002F3}
# 	2:21:DefaultSinkHeaderFile:mdctext.h
# 	2:16:DefaultSinkClass:CMdcText
# End Section
# Section menu : {04598FC1-866C-11CF-AB7C-00AA00C08FCF}
# 	2:5:Class:CLabelControl
# 	2:10:HeaderFile:labelcontrol1.h
# 	2:8:ImplFile:labelcontrol1.cpp
# End Section
# Section menu : {648A5600-2C6E-101B-82B6-000000000014}
# 	2:21:DefaultSinkHeaderFile:mscomm.h
# 	2:16:DefaultSinkClass:CMSComm
# End Section
# Section menu : {C932BA85-4374-101B-A56C-00AA003668DC}
# 	2:21:DefaultSinkHeaderFile:msmask1.h
# 	2:16:DefaultSinkClass:CMSMask
# End Section
# Section menu : {E6E17E90-DF38-11CF-8E74-00A0C90F26F8}
# 	2:5:Class:CMSComm
# 	2:10:HeaderFile:mscomm.h
# 	2:8:ImplFile:mscomm.cpp
# End Section
# Section menu : {9F6AA700-D188-11CD-AD48-00AA003C9CB6}
# 	2:5:Class:CRowCursor
# 	2:10:HeaderFile:rowcursor.h
# 	2:8:ImplFile:rowcursor.cpp
# End Section
