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

#include "sfecho.h"

//
//  constructor for transform class
//
CAudioTransform::CAudioTransform(
    IUnknown * punk,
    HRESULT  * phr)
    : m_fPropsChanged(TRUE)
    , m_fDirty(FALSE)
    , m_fInitialize(TRUE)
    , m_pwfx(NULL)
    , m_fCooked(FALSE)
    , m_pdDelay(NULL)
    , m_csDelay(0)
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
//  init transform state whenever filter props changes
//
HRESULT CAudioTransform::InitState
(
    LPTRANSFORM_PROPS       pProps,
    TRANSFORM_STATE         nState
)
{
    BOOL            fFirstBuffer = (TRANSFORM_STATE_FIRSTBUFFER == nState);
    BOOL            fTail        = (TRANSFORM_STATE_ENDOFSTREAM == nState);
    UINT            ii;
    UINT            csDelay;

    csDelay = MulDiv(pProps->lDelayMillisec,
                     m_pwfx->nSamplesPerSec * m_pwfx->nChannels,
                     1000);

    if ((NULL != m_pdDelay) && (m_csDelay == csDelay))
    {
        if (fFirstBuffer)
            ZeroMemory(m_pdDelay, csDelay * sizeof(FLOAT));
        return NOERROR;
    }

    if (NULL != m_pdDelay)
    {
        HeapFree(GetProcessHeap(), 0, m_pdDelay);
        m_pdDelay = NULL;
    }

    DWORD cb = sizeof(FLOAT) * csDelay;
    m_csDelay = csDelay;
    m_pdDelay = (LPFLOAT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
    if (NULL == m_pdDelay)
    {
        return E_OUTOFMEMORY;
    }

    for (ii = 0; ii < NUMELMS(m_aChan); ++ii)
    {
        m_aChan[ii].ix = ii;
    }

    m_ccSilence = m_csDelay;
    m_fSilenceOk = FALSE;

    return NOERROR;
}

HRESULT CAudioTransform::TransformCookProperties
(
    LPTRANSFORM_PROPS       pProps,
    TRANSFORM_STATE         nState
)
{
    HRESULT             hr;

    m_dWetLevel = SfGetRatioFromdB(pProps->lWetLevel, SFECHO_WET_GAIN_MIN);
    m_dDryLevel = SfGetRatioFromdB(pProps->lDryLevel, SFECHO_WET_GAIN_MIN);

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

    //
    // Variable to eliminate double cooking and also force cooking with 
    // FirstBuffer notification in Transform16 or TransformFloat
    //
    m_fInitialize = FALSE;

    return hr;
}

HRESULT CAudioTransform::TransformEnd()
{
    if (NULL != m_pdDelay)
    {
        HeapFree(GetProcessHeap(), 0, m_pdDelay);
        m_pdDelay = NULL;
    }

    return NOERROR;
}

#if !defined FLOAT_SAMPLES_ONLY

HRESULT CAudioTransform::Transform16bit (
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

    //
    //  If the properties have changed, or we have never set them up at
    //  all, or on notification of discontinuity, init the transform 
    //  state here (if necessary)...
    //
    fPropsChanged = TransformGetProperties(&props);
    if((fPropsChanged || !m_fCooked) || (fFirstBuffer && m_fInitialize))
    {
        hr = TransformCookProperties(&props, nState);
        if (FAILED(hr))
            return hr;
    }
    m_fInitialize = TRUE;


    // loop through channels.
    //
    hr = S_OK;
    for (nChan = 0; nChan < cChannels; ++nChan)
    {
        struct _per_channel_state * pChanState = &m_aChan[nChan];
        LPSHORT ps = psBuffer + nChan;

        if (fFirstBuffer)
        {
            pChanState->ix = nChan;
            pChanState->ccTail = m_csDelay / cChannels;
        }

        if (fTail)
        {
            ccWrite = ccBuffer = min(ccBuffer, pChanState->ccTail);
            pChanState->ccTail -= ccBuffer;
            if (0 == pChanState->ccTail)
                hr = S_FALSE;
        }

        // walk through samples in the channel, processing each one
        //
        for (ii = 0; ii < ccBuffer; ++ii, ps += cChannels)
        {
            double ddry, dwet;

            ddry = (double)ps[0];
            dwet = m_pdDelay[pChanState->ix];
            m_pdDelay[pChanState->ix] = (float)ddry;

            if ((pChanState->ix += cChannels) >= m_csDelay)
                pChanState->ix = nChan;

            dwet  = (dwet * m_dWetLevel + ddry * m_dDryLevel);
            ps[0] = BOUND16I(dwet);
        }
    }

    //  Return number of cells processed. If we are in end of stream and we
    //  return S_FALSE, that means we are done with end of stream
    //
    *pccBuffer = ccWrite;

    return hr;
}

#endif // FLOAT_SAMPLES_ONLY


#ifdef FLOAT_SAMPLES_SUPPORTED

HRESULT CAudioTransform::TransformFloat
(
    TRANSFORM_STATE     nState,
    LPFLOAT             psBuffer,
    LPDWORD             pccBuffer,
    BOOL                fSilence
)
{
    BOOL            fFirstBuffer = (TRANSFORM_STATE_FIRSTBUFFER == nState);
    BOOL            fTail        = (TRANSFORM_STATE_ENDOFSTREAM == nState);
    UINT            cChannels    = m_pwfx->nChannels;
    UINT            ccBuffer     = *pccBuffer;
    UINT            ccWrite      = ccBuffer;    // we normally write all back out
    HRESULT         hr;
    UINT            nChan;
    UINT            ii;
    BOOL            fPropsChanged;
    TRANSFORM_PROPS props;

    // bail if ok for silence
    if (m_fSilenceOk)
    {
        if (fSilence)
        {
            *pccBuffer = 0;
            return S_FALSE;
        }
        else
        {
            // reset for next (possible) silence
            m_ccSilence = m_csDelay;
            m_fCooked = FALSE;
            m_fSilenceOk = FALSE;
        }
    }

    //
    //  If the properties have changed, or we have never set them up at
    //  all, or on notification of discontinuity, init the transform 
    //  state here (if necessary)...
    //
    fPropsChanged = TransformGetProperties(&props);
    if(fFirstBuffer && m_fInitialize)
    {
        hr = TransformCookProperties(&props, nState);
        if (FAILED(hr))
            return hr;
    }
    else
    {
        if((fPropsChanged || !m_fCooked) && (!fSilence))
        {
            // If a property changes in the tail then start over/stop
			// otherwise in a 20 second decay the user will become confused
			if(fTail)	
			{
				ccWrite = 0;
				return S_FALSE;
			}

			hr = TransformCookProperties(&props, nState);
			if (FAILED(hr))
				return hr;
		}
    }

    m_fInitialize = TRUE;

    if (fSilence)
    {
        ccWrite = ccBuffer = min((int) ccBuffer, (int) m_ccSilence);
        m_ccSilence -= ccBuffer;
        if (0 == (int) m_ccSilence)
        {
            m_fSilenceOk = TRUE;
        }
    }
    else
    {
        // if !fSilence occurs before m_fSilenceOk, restart the count
        m_ccSilence = m_csDelay;
    }

    // loop through channels
    //
    hr = S_OK;
    for (nChan = 0; nChan < cChannels; ++nChan)
    {
        struct _per_channel_state * pChanState = &m_aChan[nChan];
        LPFLOAT ps = psBuffer + nChan;

        if (fFirstBuffer)
        {
            pChanState->ix = nChan;
            pChanState->ccTail = m_csDelay / cChannels;
        }

        if (fTail)
        {
            ccWrite = ccBuffer = min(ccBuffer,pChanState->ccTail);
            pChanState->ccTail -= ccBuffer;
            if (0 == pChanState->ccTail)
                hr = S_FALSE;
        }

        // walk through samples in the channel, processing each one
        //
        for (ii = 0; ii < ccBuffer; ++ii, ps += cChannels)
        {
            double dwet, ddry;

            ddry = ps[0];
            dwet = m_pdDelay[pChanState->ix];
            m_pdDelay[pChanState->ix] = (float)ddry;

            if ((pChanState->ix += cChannels) >= m_csDelay)
                pChanState->ix = nChan;

            dwet = (dwet * m_dWetLevel) + (ddry * m_dDryLevel);

            // no need to clamp the output (with floats). The downstream
            // filter may apply a gain reduction or there also may be a
            // downstream filter trying to detect clipping.
            //
            ps[0] = (float) dwet;
        }
    }

    //  Return number of cells processed. If we are in end of stream and we
    //  return S_FALSE, that means we are done with end of stream
    //
    *pccBuffer = ccWrite;

    return hr;
}

#endif // FLOAT_SAMPLES_SUPPORTED
