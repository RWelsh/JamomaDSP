//==========================================================================;
//
//  This material has been supplied as part of the Sonic Foundry Plug-In
//  Development Kit (PIDK). Under copyright laws, this material may not be
//  duplicated in whole or in part, except for personal use, without the
//  express written consent of Sonic Foundry, Inc. Refer to the license
//  agreement contained with the PIDK before using any part of this material.
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Web:    www.sonicfoundry.com
//  Email:  pidk@sonicfoundry.com
//
//  Copyright (C) 1996-1999 Sonic Foundry, Inc. All Rights Reserved.
//  Portions Copyright (C) 1992-1999 Microsoft Corporation.
//
//--------------------------------------------------------------------------;
//
//  This is an example DirectX Audio Plug-In.
//
//
//==========================================================================;

Sonic Foundry Mix Plug-In (SFMIX.DLL)

The Sonic Foundry Mix Plug-In demonstrates a non-in-place transform
with a media type change. This example changes a stereo input to a
mono or stereo output with independant mixing control of the left and
right channels. This is a good starting point for effects that need to
change the number of channels from input to output (stereo to mono
converters).

Files contained in this example:

    readme.txt      - this file
    sfmix.h         - classes and defines for the transform
    sfmix.cpp       - transform (the real work) is in here
    sfmix.def       - generic linker definition file
    sfplugin.cpp    - DirectX 'glue' (usually doesn't need changes)
    sfppage.h       - classes/defines for the property page(s) (the GUI)
    sfppage.cpp     - implements the property page(s)
    sfmix.rc        - dialog templates and stuff for the property pages
    sfoundry.bmp    - logo bitmap for Sonic Foundry
    resource.h      - dialog id's
    context.h       - help file context id's
    sfmix.hpj       - project file for building help file
    sfmix.rtf       - help file text in rich text format
    makefile        - makefile
    _depend         - makefile dependencies

