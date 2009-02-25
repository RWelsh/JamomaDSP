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
#include <math.h>

#include "sfgargle.h"

//
//  constructor for transform class
//
CAudioTransform::CAudioTransform(
    IUnknown * punk,
    HRESULT  * phr)
    : m_fPropsChanged(TRUE)
    , m_fDirty(FALSE)
    , m_pwfx(NULL)
    , m_fCooked(FALSE)
    , m_pdShape(NULL)
    , m_cShapeEntries(0)
{
    ZeroMemory(&m_prop, sizeof(m_prop));
    ZeroMemory(&m_aNotify, sizeof(m_aNotify));

    ZeroMemory (m_aChan, sizeof(m_aChan));
}

CAudioTransform::~CAudioTransform()
{
    //
    //  be sure stuff is freed!
    //
    TransformEnd();
}

//
//
//
double SfGetRatioFromdB
(
    long                    ldB,
    long                    lInf
)
{
    if (ldB <= lInf)
    {
        return (0.0);  
    }

    return pow(10.0, (double)ldB / 200.0);
}

//
// init transform state whenever filter props changes
//
HRESULT CAudioTransform::InitState
(
    LPTRANSFORM_PROPS       pProps,
    TRANSFORM_STATE         nState
)
{
    BOOL                fFirstBuffer = (TRANSFORM_STATE_FIRSTBUFFER == nState);
    BOOL                fTail        = (TRANSFORM_STATE_ENDOFSTREAM == nState);
    UINT                ii;
    UINT                cShapeEntries;

    //
    //  compute space for one complete cycle
    //
    cShapeEntries = MulDiv(m_pwfx->nSamplesPerSec, 10, pProps->lFrequency);

    //  the logic below assumes the following minimums
    //
    switch (pProps->lShape)
    {
        case Gargle_Sawtooth:
        case Gargle_Square:
            cShapeEntries = max(2, cShapeEntries);
            break;

        case Gargle_Triangle:
        default:
            cShapeEntries = max(3, cShapeEntries);
            break;
    }

    // if we already have a shape buffer, but it's the wrong
    // size, free it now.
    //
    if ((NULL != m_pdShape) && (m_cShapeEntries != cShapeEntries))
    {
        HeapFree(GetProcessHeap(), 0, m_pdShape);
        m_pdShape       = NULL;
        m_cShapeEntries = 0;
    }

    // allocate a shape buffer if we dont have one.
    //
    if (NULL == m_pdShape)
    {
        DWORD       cb = cShapeEntries * sizeof(m_pdShape[0]);

        m_pdShape = (LPFLOAT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
        if (NULL == m_pdShape)
        {
            return E_OUTOFMEMORY;
        }

        m_cShapeEntries = cShapeEntries;
    }

    // init the wave shape buffer
    //
    switch (pProps->lShape)
    {
        case Gargle_Sawtooth:
            for (ii = 0; ii < cShapeEntries; ++ii)
                m_pdShape[ii] = (float)ii / (float)(cShapeEntries-1);
            break;

        case Gargle_Square:
            for (ii = 0; ii < cShapeEntries; ++ii)
                m_pdShape[ii] = (float)((ii < cShapeEntries/2) ? 0.0 : 1.0);
            break;

        case Gargle_Triangle:
        default:
            for (ii = 0; ii < (cShapeEntries+1)/2; ++ii)
            {
                m_pdShape[cShapeEntries-ii-1] =
                m_pdShape[ii] = (float)ii / (float)((cShapeEntries-1)/2);
            }
            break;
    }

    ZeroMemory(m_aChan, sizeof(m_aChan));

    return NOERROR;
}

HRESULT CAudioTransform::TransformCookProperties
(
    LPTRANSFORM_PROPS       pProps,
    TRANSFORM_STATE         nState
)
{
    HRESULT             hr;

    m_dDryLevel = SfGetRatioFromdB(pProps->lDryLevel, SFGARGLE_DRY_GAIN_MIN);
    m_dWetLevel = SfGetRatioFromdB(pProps->lWetLevel, SFGARGLE_WET_GAIN_MIN);

    hr = InitState(pProps, nState);

    m_fCooked = !FAILED(hr);

    return hr;
}

HRESULT CAudioTransform::TransformBegin(
    LPWAVEFORMATEX pwfx)
{
    HRESULT             hr;
    TRANSFORM_PROPS     props;

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
    TransformGetProperties(&props);
    hr = TransformCookProperties(&props, TRANSFORM_STATE_FIRSTBUFFER);

    return hr;
}

HRESULT CAudioTransform::TransformEnd()
{
    if (NULL != m_pdShape)
    {
        HeapFree(GetProcessHeap(), 0, m_pdShape);
        m_pdShape = NULL;
    }

    return NOERROR;
}


#if !defined FLOAT_SAMPLES_ONLY

HRESULT CAudioTransform::Transform16bit(
    TRANSFORM_STATE     nState,
    LPSHORT             psBuffer,
    LPDWORD             pccBuffer)
{
    BOOL            fFirstBuffer = (TRANSFORM_STATE_FIRSTBUFFER == nState);
    BOOL            fTail        = (TRANSFORM_STATE_ENDOFSTREAM == nState);
    UINT            cChannels    = m_pwfx->nChannels;
    UINT            ccBuffer     = *pccBuffer;
    UINT            ccWrite      = ccBuffer;
    HRESULT         hr;
    UINT            nChan;
    UINT            ii;
    BOOL            fPropsChanged;
    TRANSFORM_PROPS props;

   #ifdef NEED_END_OF_STREAM
    //
    // this code has no tail data
    //
    if (fTail)
    {
        *pcsBuffer = 0;
        return S_FALSE;
    }
   #endif

    //
    //  If the properties have changed, or we have never set them up at
    //  all, init the transform state here (if necessary)...
    //
    fPropsChanged = TransformGetProperties(&props);
    if (fPropsChanged || !m_fCooked)
    {
        hr = TransformCookProperties(&props, nState);
        if (FAILED(hr))
            return hr;
    }

    // loop through channels.
    //
    hr = S_OK;
    for (nChan = 0; nChan < cChannels; ++nChan)
    {
        struct _per_channel_state * pChanState = &m_aChan[nChan];
        LPSHORT ps = psBuffer + nChan;

        if (fFirstBuffer)
            pChanState->ix = 0;

        // walk through samples in the channel, processing each one
        //
        for (ii = 0; ii < ccBuffer; ++ii, ps += cChannels)
        {
            double ddry, dwet;

            ddry = (double)(ps[0]);
            dwet = ddry * m_pdShape[pChanState->ix];
            if ((++pChanState->ix) >= m_cShapeEntries)
                pChanState->ix = 0;

            dwet = dwet * m_dWetLevel + ddry * m_dDryLevel;
            ps[0] = BOUND16I(dwet);
        }
    }

    //  Return number of cells processed. If we are in end of stream and
    //  return S_FALSE, that means we are done with end of stream
    //
    *pccBuffer = ccWrite;

    return hr;
}

#endif // FLOAT_SAMPLES_ONLY


#ifdef FLOAT_SAMPLES_SUPPORTED

HRESULT CAudioTransform::TransformFloat (
    TRANSFORM_STATE     nState,
    LPFLOAT             psBuffer,
    LPDWORD             pccBuffer,
    BOOL                fSilence)
{
    BOOL            fFirstBuffer = (TRANSFORM_STATE_FIRSTBUFFER == nState);
    BOOL            fTail        = (TRANSFORM_STATE_ENDOFSTREAM == nState);
    UINT            cChannels    = m_pwfx->nChannels;
    UINT            ccBuffer     = *pccBuffer;
    UINT            ccWrite      = ccBuffer;
    HRESULT         hr;
    UINT            nChan;
    UINT            ii;
    BOOL            fPropsChanged;
    TRANSFORM_PROPS props;

   #ifdef NEED_END_OF_STREAM
    //
    // this code has no tail data
    //
    if (fTail)
    {
        *pcsBuffer = 0;
        return S_FALSE;
    }
   #endif

    if(fSilence)
    {
        *pccBuffer = 0;
        m_fCooked = FALSE;
        return S_FALSE;        
    }

    //
    //  If the properties have changed, or we have never set them up at
    //  all, init the transform state here (if necessary)...
    //
    fPropsChanged = TransformGetProperties(&props);
    if (fPropsChanged || !m_fCooked)
    {
        hr = TransformCookProperties(&props, nState);
        if (FAILED(hr))
            return hr;
    }

    // loop through channels
    //
    hr = S_OK;
    for (nChan = 0; nChan < cChannels; ++nChan)
    {
        struct _per_channel_state * pChanState = &m_aChan[nChan];
        LPFLOAT ps = psBuffer + nChan;

        if (fFirstBuffer)
            pChanState->ix = 0;

        // walk through samples in the channel, processing each one
        //
        for (ii = 0; ii < ccBuffer; ++ii, ps += cChannels)
        {
            double ddry, dwet;

            ddry = ps[0];
            dwet = ddry * m_pdShape[pChanState->ix];
            if ((++pChanState->ix) >= m_cShapeEntries)
                pChanState->ix = 0;

            dwet = dwet * m_dWetLevel + ddry * m_dDryLevel;

            //
            //  do NOT bound floats--that is for the renderer to deal
            //  with
            //
            ps[0] = (float)dwet;
        }
    }

    //  Return number of cells processed. If we are in end of stream and
    //  return S_FALSE, that means we are done with end of stream
    //
    *pccBuffer = ccWrite;

    return hr;
}

#endif // FLOAT_SAMPLES_SUPPORTED
