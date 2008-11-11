# Microsoft Developer Studio Project File - Name="dtsSDKPlus" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=dtsSDKPlus - Win32 Hybrid
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dtsSDKPlus.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dtsSDKPlus.mak" CFG="dtsSDKPlus - Win32 Hybrid"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dtsSDKPlus - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "dtsSDKPlus - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "dtsSDKPlus - Win32 Hybrid" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dtsSDKPlus - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../out.VC6.RELEASE"
# PROP Intermediate_Dir "../out.VC6.RELEASE"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\dtsSDK" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "dtsSDKPlus - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../out.VC6.DEBUG"
# PROP Intermediate_Dir "../out.VC6.DEBUG"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\dtsSDK" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../out.VC6.DEBUG/dtsSDKPlus_DEBUG.lib" /NODEFAULTLIB:LIBCD

!ELSEIF  "$(CFG)" == "dtsSDKPlus - Win32 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "dtsSDKPlus___Win32_Hybrid"
# PROP BASE Intermediate_Dir "dtsSDKPlus___Win32_Hybrid"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../out.VC6.HYBRID"
# PROP Intermediate_Dir "../out.VC6.HYBRID"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\dtsSDK" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT BASE CPP /Fr /YX
# ADD CPP /nologo /MT /W3 /Gm /GR /GX /ZI /Od /I "..\dtsSDK" /D "WIN32" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"../out.VC6.DEBUG/dtsSDKPlus_DEBUG.lib" /NODEFAULTLIB:LIBCD
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "dtsSDKPlus - Win32 Release"
# Name "dtsSDKPlus - Win32 Debug"
# Name "dtsSDKPlus - Win32 Hybrid"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\dtsSDKPlus\appConfig.cpp
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\appIfl.cpp
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\appMesh.cpp
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\appNode.cpp
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\appSceneEnum.cpp
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\appSequence.cpp
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\appTime.cpp
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\decomp\Decompose.cpp
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\DTSUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\nvStripWrap.cpp
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\nvtristrip\NvTriStrip.cpp
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\nvtristrip\NvTriStripObjects.cpp
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\ShapeMimic.cpp
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\stripper.cpp
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\translucentSort.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\dtsSDKPlus\appConfig.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\appIfl.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\appMesh.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\appNode.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\appSceneEnum.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\appSequence.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\appTime.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\decomp\Decompose.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\dtsBitMatrix.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\DTSPlusTypes.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\DTSUtil.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\nvStripWrap.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\nvtristrip\NvTriStrip.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\nvtristrip\NvTriStripObjects.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\sceneEnum.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\ShapeMimic.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\stripper.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\translucentSort.h
# End Source File
# Begin Source File

SOURCE=..\dtsSDKPlus\nvtristrip\VertexCache.h
# End Source File
# End Group
# End Target
# End Project
