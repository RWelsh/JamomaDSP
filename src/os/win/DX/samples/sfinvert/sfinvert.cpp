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
//
//==========================================================================;

#ifndef STRICT
#define STRICT
#endif
#ifndef INC_OLE2
#define INC_OLE2
#endif
#include <streams.h>
#include <olectl.h>

#include "sfinvert.h"

CAudioTransform::CAudioTransform (
    IUnknown * punk,
    HRESULT  * phr)
    : m_pwfx(NULL)
{
}

CAudioTransform::~CAudioTransform ()
{
    TransformEnd(); // be sure stuff is freed!
}

HRESULT CAudioTransform::TransformBegin(
    LPWAVEFORMATEX pwfx)
{
    //
    //  do initial allocations and other time consuming stuff here...
    //
    m_pwfx = pwfx;

    //
    //  precompute whatever we can so the when data starts streaming
    //  we start processing as fast as possible
    //
    //  the reason for this is simple: an application can create and init
    //  the 'graph' that your plug-in will be running in during a non-
    //  critical time... but when, for example, playback starts in a
    //  multi-track, you don't want to burn a bunch of cycles on the 
    //  first buffer
    //

    return NOERROR;
}

HRESULT CAudioTransform::TransformEnd ()
{
    return NOERROR;
}

#if !defined FLOAT_SAMPLES_ONLY

// inplace transform of interleaved buffer of n channels
// the data is assumed to be 16 bit pcm
//
HRESULT CAudioTransform::Transform16bit (
    LPSHORT                 psBuffer,
    DWORD                   ccBuffer)
{
    DWORD               csBuffer = ccBuffer * m_pwfx->nChannels;
    HRESULT             hr       = S_OK;
    DWORD               ii;

    // walk through samples in the channel, processing each one
    //
    LPSHORT ps = psBuffer;
    for (ii = 0; ii < csBuffer; ii++)
    {
        int ss = -ps[ii];
        ps[ii] = (ss > 32767) ? (short)32767 : (short)ss;
    }

    return hr;
}

#endif // FLOAT_SAMPLES_ONLY

#ifdef FLOAT_SAMPLES_SUPPORTED

// inplace transform of an interleaved buffer of n channels
// the data is assumed to be 16 bit pcm
//
HRESULT CAudioTransform::TransformFloat(
    LPFLOAT                 psBuffer,
    DWORD                   ccBuffer)
{
    DWORD               csBuffer = ccBuffer * m_pwfx->nChannels;
    HRESULT             hr       = S_OK;
    DWORD               ii;

    // walk through samples in the channel, processing each one
    //
   #if 0

    LPFLOAT ps = psBuffer;
    for (ii = 0; ii < csBuffer; ii++)
    {
        //!!! this is slow!
        ps[ii] = -ps[ii];
    }

   #else

    // Note that since a floating point number is a stored a signed
    // magnitude.  we can negate it merely by inverting the sign bit.
    // On x86 processors, negating the sign is MUCH faster. On
    // Alpha, MIPS, PPC, etc the cost is the same.
    //

    LPDWORD ps = (LPDWORD)psBuffer;
    for (ii = 0; ii < csBuffer; ii++)
    {
        // invert the sign bit... fast!
        ps[ii] ^= 0x80000000l;
    }

   #endif

    return hr;
}

#endif
