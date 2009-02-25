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

Sonic Foundry Pan Plug-In (SFPAN.DLL)

The Sonic Foundry Pan Plug-In demonstrates a non-in-place transform
with a media type change. This example changes a mono or stereo input
to a stereo output with left and right channel balance control. This
is a good starting point for effects that need to change the number of
channels from input to output (mono to stereo converters, panning,
3D audio processors, mono to stereo reverbs, etc.).

Files contained in this example:

    readme.txt      - this file
    sfpan.h         - classes and defines for the transform
    sfpan.cpp       - transform (the real work) is in here
    sfpan.def       - generic linker definition file
    sfplugin.cpp    - DirectX 'glue' (usually doesn't need changes)
    sfppage.h       - classes/defines for the property page(s) (the GUI)
    sfppage.cpp     - implements the property page(s)
    sfpan.rc        - dialog templates and stuff for the property pages
    sfoundry.bmp    - logo bitmap for Sonic Foundry
    resource.h      - dialog id's
    context.h       - help file context id's
    sfpan.hpj       - project file for building help file
    sfpan.rtf       - help file text in rich text format
    makefile        - makefile
    _depend         - makefile dependencies

