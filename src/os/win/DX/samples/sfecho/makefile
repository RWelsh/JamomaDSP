#===========================================================================;
#
#   This material has been supplied as part of the Sonic Foundry Plug-In
#   Development Kit (PIDK). Under copyright laws, this material may not be
#   duplicated in whole or in part, except for personal use, without the
#   express written consent of Sonic Foundry, Inc. Refer to the license
#   agreement contained with the PIDK before using any part of this material.
#
#   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
#   KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
#   IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
#   PURPOSE.
#
#   Web:    www.sonicfoundry.com
#   Email:  pidk@sonicfoundry.com
#
#   Copyright (C) 1996-1999 Sonic Foundry, Inc. All Rights Reserved.
#   Portions Copyright (C) 1992-1999 Microsoft Corporation.
#
#---------------------------------------------------------------------------;
#
#   DIRECTSHOW Project Makefile
#
#   Targets are provided by DSHOWSDK.MAK
#
#   all     Create executables (default).
#   clean   Remove all files created by any goal.
#
#   This makefile stub simplifies makefile creation for those
#   cases where you want to use standard options.
#
#===========================================================================;

# Target
#
TARGET_NAME = sfecho
TARGET_TYPE = DYNLINK
TARGET_EXT  = dll

# Source files
#
SRC_FILES   = $(TARGET_NAME).cpp sfplugin.cpp sfppage.cpp
RC_FILE     = $(TARGET_NAME).rc
DEF_FILE    = $(TARGET_NAME).def

# DLL specific defines
#
DLL_BASE    = 0x1D160000
LINK_LIBS   =

# setup environment
#
!ifndef DXMEDIASDK
DXMEDIASDK = \dxmedia
!endif

!ifndef DSHOWSAMPLE
DSHOWSAMPLE = $(DXMEDIASDK)\samples\multimedia\dshow\src
!endif

# comment (#) the next line to copy .dll and .hlp files to $(DSHOWSAMPLE)\..\bin\$(DBG_RTL)
NO_COPY = 1

INCLUDE = ..\include;$(INCLUDE);$(DXMEDIASDK)\lib

# stuff to build after the dll has been built.
#
END_TASK = $(TARGET_BIN_GOAL)\$(TARGET_NAME).hlp
!ifdef NO_COPY
END_TASK = $(DBG_RTL)\$(TARGET_NAME).hlp
!endif

# include DSHOWSDK.MAK, main goal is here
#
!include "$(DSHOWSAMPLE)\dshowsdk.mak"

# project specific goals

# build the help file
#
$(DBG_RTL)\$(TARGET_NAME).hlp: $(TARGET_NAME).hpj $(TARGET_NAME).rtf
    echo Compiling help file using $(TARGET_NAME).hpj...   > $(MAKE_ECHO)
    $(TYPE_ECHO)
    $(FILE_ECHO)
    echo hcw /C /M /E $(TARGET_NAME).hpj > $(MAKE_ECHO)
    $(TYPE_ECHO2)
    $(FILE_ECHO)
    hcw /C /M /E $(TARGET_NAME).hpj
    echo. > $(MAKE_ECHO)
    $(TYPE_ECHO2)
    $(FILE_ECHO)
    copy $(TARGET_NAME).hlp $(DBG_RTL) > $(MAKE_ECHO)
    del $(TARGET_NAME).hlp > $(MAKE_ECHO)

!ifndef NO_COPY
$(TARGET_BIN_GOAL)\$(TARGET_NAME).hlp: $(DBG_RTL)\$(TARGET_NAME).hlp context.h
    echo Copy $(DBG_RTL)\$(TARGET_NAME).hlp $(TARGET_BIN_GOAL)\$(TARGET_NAME).hlp > $(MAKE_ECHO)
    $(TYPE_ECHO2)
    $(FILE_ECHO)
    copy $(DBG_RTL)\$(TARGET_NAME).hlp $(TARGET_BIN_GOAL)\$(TARGET_NAME).hlp > $(MAKE_ECHO)
    $(TYPE_ECHO2)
    $(FILE_ECHO)
    echo. > $(MAKE_ECHO)
    $(TYPE_ECHO2)
    $(FILE_ECHO)
!endif
