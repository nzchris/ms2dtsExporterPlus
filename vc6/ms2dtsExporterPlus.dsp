# Microsoft Developer Studio Project File - Name="ms2dtsExporterPlus" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ms2dtsExporterPlus - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ms2dtsExporterPlus.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ms2dtsExporterPlus.mak" CFG="ms2dtsExporterPlus - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ms2dtsExporterPlus - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ms2dtsExporterPlus - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ms2dtsExporterPlus - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "out.VC6.RELEASE"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MS2DTSEXPORTERPLUS_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\dtssdk" /I "..\dtssdkplus" /I "..\msLIB" /I "..\mNode" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MS2DTSEXPORTERPLUS_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1409 /d "NDEBUG"
# ADD RSC /l 0x1409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msModelLib.lib dtsSDK.lib dtsSDKPlus.lib htmlhelp.lib /nologo /dll /machine:I386 /out:"C:\Program Files\MilkShape 3D 1.7.8\ms2dtsExporterPlus.dll" /libpath:"..\msLIB\lib6" /libpath:"..\msLIB" /libpath:"..\out.vc6.RELEASE"

!ELSEIF  "$(CFG)" == "ms2dtsExporterPlus - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../out.vc6.DEBUG"
# PROP Intermediate_Dir "../out.vc6.DEBUG"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MS2DTSEXPORTERPLUS_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\dtssdk" /I "..\dtssdkplus" /I "..\msLIB" /I "..\mNode" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MS2DTSEXPORTERPLUS_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1409 /d "_DEBUG"
# ADD RSC /l 0x1409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msModelLib.lib dtsSDK_DEBUG.lib dtsSDKPlus_DEBUG.lib htmlhelp.lib /nologo /dll /debug /machine:I386 /out:"C:\Program Files\MilkShape 3D 1.7.8\ms2dtsExporterPlus_DEBUG.dll" /pdbtype:sept /libpath:"..\msLIB\lib6" /libpath:"..\msLIB" /libpath:"..\out.vc6.DEBUG"

!ENDIF 

# Begin Target

# Name "ms2dtsExporterPlus - Win32 Release"
# Name "ms2dtsExporterPlus - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "mNode"

# PROP Default_Filter "cpp;h"
# Begin Source File

SOURCE=..\mNode\MilkshapeBone.cpp
# End Source File
# Begin Source File

SOURCE=..\mNode\MilkshapeBone.h
# End Source File
# Begin Source File

SOURCE=..\mNode\MilkshapeMaterial.cpp
# End Source File
# Begin Source File

SOURCE=..\mNode\MilkshapeMaterial.h
# End Source File
# Begin Source File

SOURCE=..\mNode\MilkshapeMesh.cpp
# End Source File
# Begin Source File

SOURCE=..\mNode\MilkshapeMesh.h
# End Source File
# Begin Source File

SOURCE=..\mNode\MilkshapeNode.cpp
# End Source File
# Begin Source File

SOURCE=..\mNode\MilkshapeNode.h
# End Source File
# Begin Source File

SOURCE=..\mNode\MilkshapeSequence.cpp
# End Source File
# Begin Source File

SOURCE=..\mNode\MilkshapeSequence.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\DllEntry.cpp
# End Source File
# Begin Source File

SOURCE=..\ms2dtsExporter.def
# End Source File
# Begin Source File

SOURCE=..\ms2dtsExporterPlus.cpp
# End Source File
# Begin Source File

SOURCE=..\msAppMesh.cpp
# End Source File
# Begin Source File

SOURCE=..\msAppNode.cpp
# End Source File
# Begin Source File

SOURCE=..\msConfig.cpp
# End Source File
# Begin Source File

SOURCE=..\msSceneEnum.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\ms2dtsExporterPlus.h
# End Source File
# Begin Source File

SOURCE=..\msAppMesh.h
# End Source File
# Begin Source File

SOURCE=..\msAppNode.h
# End Source File
# Begin Source File

SOURCE=..\msConfig.h
# End Source File
# Begin Source File

SOURCE=..\msLIB\msLib.h
# End Source File
# Begin Source File

SOURCE=..\msLIB\msPlugIn.h
# End Source File
# Begin Source File

SOURCE=..\msSceneEnum.h
# End Source File
# Begin Source File

SOURCE=..\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\ms2dtsExporterPlus.rc
# End Source File
# End Group
# End Target
# End Project
