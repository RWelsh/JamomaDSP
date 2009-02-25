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

#include "sflength.h"

//
//  constructor for transform class
//
CAudioTransform::CAudioTransform(
    IUnknown * punk,
    HRESULT  * phr)
    : m_fPropsChanged(TRUE)
    , m_fDirty(FALSE)
    , m_pwfxIn(NULL)
    , m_pwfxOut(NULL)
    , m_fCooked(FALSE)
{
    ZeroMemory(&m_prop, sizeof(m_prop));
    ZeroMemory(&m_aNotify, sizeof(m_aNotify));
}

CAudioTransform::~CAudioTransform()
{
    //
    //  be sure stuff is freed!
    //
    TransformEnd();
}

HRESULT CAudioTransform::TransformCookProperties
(
    LPTRANSFORM_PROPS       pProps,
    TRANSFORM_STATE         nState
)
{
    HRESULT             hr;

   #ifdef NEED_CHANNELS_IN
    ASSERT(NEED_CHANNELS_IN == m_pwfxIn->nChannels);
   #endif
   #ifdef NEED_CHANNELS_OUT
    ASSERT(NEED_CHANNELS_OUT == m_pwfxOut->nChannels);
   #endif

    ASSERT(m_pwfxIn->nChannels == m_pwfxOut->nChannels);

    //
    //  change from a fixed point (one decimal place) percentage value
    //  to a ratio for calculations...
    //
    m_dCookedRatio = (double)pProps->dwPercentChange / 1000.0;

    hr = NOERROR;

    m_fCooked = !FAILED(hr);

    return hr;
}

HRESULT CAudioTransform::TransformBegin(
    LPWAVEFORMATEX pwfxIn,
    LPWAVEFORMATEX pwfxOut)
{
    HRESULT             hr;
    TRANSFORM_PROPS     props;

    //
    //  do initial allocations and other time consuming stuff here...
    //
    m_pwfxIn  = pwfxIn;
    m_pwfxOut = pwfxOut;

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
    return NOERROR;
}


//
//  This function is called with an input buffer size and should return
//  an output buffer size sufficient to process an input buffer of the
//  specified size.
//
HRESULT CAudioTransform::TransformDecideBufferSize
(
    IMemAllocator          *pAlloc,
    ALLOCATOR_PROPERTIES   *pProp
)
{
    #define BUFFER_BYTES_MINIMUM        512     // whatever...

    HRESULT                 hr;
    ALLOCATOR_PROPERTIES    Actual;

    //
    //  at most, we can quadruple the size of the data
    //
    ASSERT(pProp->cbBuffer != 0);
    pProp->cbBuffer *= 4;

    //
    //  ask the allocator to reserve buffer memory, NOTE the function
    //  can succeed (that is return NOERROR) but still not have allocated
    //  the memory that we requested, so we must check we got whatever we
    //  wanted...
    //
    hr = pAlloc->SetProperties(pProp, &Actual);
    if ( ! FAILED(hr))
    {
        //
        //  copy whatever the result was back into pProp...
        //
        *pProp = Actual;

        //
        //  NOTE! you must be prepared to deal with not getting your exact
        //  specifications--or fail (don't be stingy!). For example,
        //  Sound Forge will not let buffers get larger than its default
        //  size. So, if you are asking the buffer to 'grow' from input
        //  to output, then you may not get it. The proper way to handle
        //  this is to loop in your Receive method using GetDeliveryBuffer
        //  until all data in the input buffer has been processed.
        //
        //  make sure that the allocator can at least provide our
        //  minimum requirement...
        //
        ASSERT(Actual.cbBuffer >= BUFFER_BYTES_MINIMUM);
        if (Actual.cbBuffer < BUFFER_BYTES_MINIMUM)
        {
            hr = E_FAIL;
        }
    }

    return hr;

    #undef BUFFER_BYTES_MINIMUM
}


#if !defined NEED_CHANNELS_OUT

//
//  This function is called with the current output format if this format
//  is correct, the function should return S_OK, if it is not correct,
//  it should return S_FALSE
//
HRESULT CAudioTransform::TransformCheckOutputFormat(
    LPWAVEFORMATEX pwfxIn,
    LPWAVEFORMATEX pwfxOut)
{
    // the caller should have already guranteed that most of
    // the media type is the same for in and out...
    //
    ASSERT (pwfxIn->wFormatTag == pwfxOut->wFormatTag);
    ASSERT (pwfxIn->wBitsPerSample == pwfxOut->wBitsPerSample);
    ASSERT (pwfxIn->nSamplesPerSec == pwfxOut->nSamplesPerSec);

    // we require that input and output have the same number of channels
    //
    if (pwfxIn->nChannels != pwfxOut->nChannels)
        return S_FALSE;

    return S_OK;
}

//
//  This function is called with the current input format it should change
//  the output format to match that requested by TRANSFORM_PROPS and return
//  success/fail
//
HRESULT CAudioTransform::TransformTweakOutputFormat(
    LPWAVEFORMATEX pwfx)
{
    return S_OK;
}

#endif // NEED_CHANNELS_OUT


static void WINAPI ResampleComputeInOut
(
    LPDWORD                 pccIn,
    LPDWORD                 pccOut,
    double                  dRatio
)
{
    DWORD               ccIn  = *pccIn;
    DWORD               ccOut = *pccOut;
    DWORD               ccNewOut;

    // figure out how many samples to write and how many
    // source samples we will 'consume'
    //
    if (dRatio < 1.0)
    {
        ccIn     = min(ccIn, ccOut);
        ccNewOut = (DWORD)(ccIn * dRatio);
        ccNewOut = max(ccNewOut, 1);
    }
    else if (dRatio > 1.0)
    {
        ccNewOut = (DWORD)(ccIn * dRatio);
        if (ccNewOut > ccOut)
        {
           ccIn     = ((ccIn * ccOut) + ccNewOut/2) / ccNewOut;
           ccIn     = min(*pccIn, ccIn);
           ccNewOut = ccOut;
        }
    }
    else
    {
        ccNewOut = min(ccIn, ccOut);
        ccIn     = ccNewOut;
    }

    *pccIn  = ccIn;
    *pccOut = ccNewOut;
}


#if !defined FLOAT_SAMPLES_ONLY

#pragma optimize("agtw", on) // optimize agressively for time
static void WINAPI Resample16(
    LPSHORT psIn,
    UINT    ccIn,
    LPSHORT psOut,
    UINT    ccOut,
    UINT    cChannels)
{
    LONG lReset;
    LONG lAccum;
    LONG lStep;

    if (ccIn > ccOut)
    {
        lReset = ccIn;
        lStep  = ccOut;
        lAccum = lReset / 2;

        while (ccIn--)
        {
            int ss = psIn[0];
            psIn += cChannels;

            lAccum -= lStep;
            if (lAccum < 0)
            {
                lAccum += lReset;
                psOut[0] = (short)ss;
                psOut += cChannels;
            }
        }
    }
    else
    {
        lReset = ccOut;
        lStep  = ccIn;
        lAccum = lReset / 2;

        while (ccIn--)
        {
            int ss = psIn[0];
            psIn += cChannels;

            do
            {
                psOut[0] = (short)ss;
                psOut += cChannels;
                lAccum -= lStep;

            } while (lAccum >= 0);

            lAccum += lReset;
        }
    }
}
#pragma optimize("", on) // revert to default optimization options

HRESULT CAudioTransform::Transform16bit
(
    TRANSFORM_STATE         nState,
    LPSHORT                 psIn,
    LPDWORD                 pccIn,
    LPSHORT                 psOut,
    LPDWORD                 pccOut
)
{
    BOOL                fFirstBuffer = (TRANSFORM_STATE_FIRSTBUFFER == nState);
    BOOL                fTail        = (TRANSFORM_STATE_ENDOFSTREAM == nState);
    UINT                cChannels    = m_pwfxIn->nChannels;
    DWORD               ccIn         = (pccIn)  ? *pccIn  : 0; 
    DWORD               ccOut        = (pccOut) ? *pccOut : 0;
    HRESULT             hr           = S_OK;
    UINT                nChan;
    BOOL                fPropsChanged;
    TRANSFORM_PROPS     props;

   #ifdef NEED_END_OF_STREAM
    //
    //  If there is no input data, there is no output data. psIn == NULL
    //  when we are in end-of-stream processing. Returning S_FALSE
    //  completes end-of-stream.
    //
    if (NULL == psIn)
    {
        *pccOut = 0;
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

    ResampleComputeInOut(&ccIn, &ccOut, m_dCookedRatio);

    // loop through channels, resampling each
    //
    for (nChan = 0; nChan < cChannels; ++nChan)
    {
        Resample16(psIn + nChan, ccIn, psOut + nChan, ccOut, cChannels);
    }

    // return number of samples to write out and the number that we
    // have consumed from the input stream.
    //
    *pccIn  = ccIn;
    *pccOut = ccOut;

    return hr;
}

#endif // FLOAT_SAMPLES_ONLY


#ifdef FLOAT_SAMPLES_SUPPORTED

#pragma optimize("agtw", on) // optimize agressively for time
static void WINAPI ResampleFloat(
    LPFLOAT psIn,
    UINT    ccIn,
    LPFLOAT psOut,
    UINT    ccOut,
    UINT    cChannels)
{
    LONG        lReset;
    LONG        lAccum;
    LONG        lStep;
    LPDWORD     pdwIn  = (LPDWORD)psIn;
    LPDWORD     pdwOut = (LPDWORD)psOut;

    //
    //  Note because we are just moving bits around (and not really
    //  processing), we use a generic 32-bit DWORD type rather than
    //  floats. The optimizer often does better in situations like
    //  this...
    //
    if (ccIn > ccOut)
    {
        lReset = ccIn;
        lStep  = ccOut;
        lAccum = lReset / 2;

        while (ccIn--)
        {
            DWORD ss = pdwIn[0];
            pdwIn += cChannels;

            lAccum -= lStep;
            if (lAccum < 0)
            {
                lAccum += lReset;
                pdwOut[0] = ss;
                pdwOut += cChannels;
            }
        }
    }
    else
    {
        lReset = ccOut;
        lStep  = ccIn;
        lAccum = lReset / 2;

        while (ccIn--)
        {
            DWORD ss = pdwIn[0];
            pdwIn += cChannels;

            do
            {
                pdwOut[0] = ss;
                pdwOut += cChannels;
                lAccum -= lStep;

            } while (lAccum >= 0);

            lAccum += lReset;
        }
    }
}
#pragma optimize("", on) // revert to default optimization options

HRESULT CAudioTransform::TransformFloat
(
    TRANSFORM_STATE         nState,
    LPFLOAT                 psIn,
    LPDWORD                 pccIn,
    LPFLOAT                 psOut,
    LPDWORD                 pccOut
)
{
    BOOL                fFirstBuffer = (TRANSFORM_STATE_FIRSTBUFFER == nState);
    BOOL                fTail        = (TRANSFORM_STATE_ENDOFSTREAM == nState);
    UINT                cChannels    = m_pwfxIn->nChannels;
    DWORD               ccIn         = (pccIn)  ? *pccIn  : 0; 
    DWORD               ccOut        = (pccOut) ? *pccOut : 0;
    HRESULT             hr           = S_OK;
    BOOL                fPropsChanged;
    TRANSFORM_PROPS     props;
    UINT                nChan;

   #ifdef NEED_END_OF_STREAM
    //
    //  If there is no input data, there is no output data. psIn == NULL
    //  when we are in end-of-stream processing. Returning S_FALSE
    //  completes end-of-stream.
    //
    if (NULL == psIn)
    {
        *pccOut = 0;
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

    ResampleComputeInOut(&ccIn, &ccOut, m_dCookedRatio);

    // loop through channels, resampling each
    //
    for (nChan = 0; nChan < cChannels; ++nChan)
    {
        ResampleFloat(psIn + nChan, ccIn, psOut + nChan, ccOut, cChannels);
    }

    // return number of samples to write out and the number that we
    // have consumed from the input stream.
    //
    *pccIn  = ccIn;
    *pccOut = ccOut;

    return hr;
}

#endif // FLOAT_SAMPLES_SUPPORTED
