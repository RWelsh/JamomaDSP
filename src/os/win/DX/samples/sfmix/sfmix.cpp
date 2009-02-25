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

#include "sfmix.h"

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

    m_dMonoLevel  = SfGetRatioFromdB(pProps->lMonoLevel, SFMIX_OUT_GAIN_MIN);
    m_dLeftLevel  = SfGetRatioFromdB(pProps->lLeftLevel, SFMIX_LEFT_GAIN_MIN);
    m_dRightLevel = SfGetRatioFromdB(pProps->lRightLevel, SFMIX_RIGHT_GAIN_MIN);

    if (pProps->fStereoOut)
    {
        m_dLeftLevel  *= m_dMonoLevel;
        m_dRightLevel *= m_dMonoLevel;
    }

    if (pProps->fInvertLeft)
       m_dLeftLevel = -m_dLeftLevel;
    if (pProps->fInvertRight)
       m_dRightLevel = -m_dRightLevel;

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
    #define BUFFER_BYTES_MINIMUM        512 // whatever...

    HRESULT                 hr;
    ALLOCATOR_PROPERTIES    Actual;

    //
    //  we will either shrink or leave the size unchanged
    //  (shrink if stereo to mono, unchanged of stereo to stereo)
    //
    if ((2 == m_pwfxIn->nChannels) &&
        (1 == m_pwfxOut->nChannels))
    {
        pProp->cbBuffer = pProp->cbBuffer / 2;
    }

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
//  is correct, the function should return S_OK, if it is not correct, if
//  not it should return S_FALSE
//
HRESULT CAudioTransform::TransformCheckOutputFormat (
    LPWAVEFORMATEX pwfxIn,
    LPWAVEFORMATEX pwfxOut)
{
    if (m_prop.fStereoOut && pwfxOut->nChannels != 2)
        return S_FALSE;
    else if ( ! m_prop.fStereoOut && pwfxOut->nChannels != 1)
        return S_FALSE;

    return S_OK;
}


//
//  This function is called with the current output format it should change
//  the output format to match that requested by TRANSFORM_PROPS and return
//  success/fail
//
HRESULT CAudioTransform::TransformTweakOutputFormat (
    LPWAVEFORMATEX pwfx)
{
    //
    //  change channels and fix up remainder of format structure
    //
    if (m_prop.fStereoOut)
        pwfx->nChannels = 2;
    else
        pwfx->nChannels = 1;

    // fixup nBlockAlign for changed number of channels
    //
    pwfx->nBlockAlign = pwfx->nChannels * (pwfx->wBitsPerSample / 8);
    pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;

    return S_OK;
}

#endif // NEED_CHANNELS_OUT


#if !defined FLOAT_SAMPLES_ONLY

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
    DWORD               ccIn         = (pccIn)  ? *pccIn  : 0; 
    DWORD               ccOut        = (pccOut) ? *pccOut : 0;
    HRESULT             hr           = S_OK;
    DWORD               ii;
    BOOL                fPropsChanged;
    TRANSFORM_PROPS     props;
    double              ss;

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

    //
    //  The number of sample cells (count cells == cc) that we can process
    //  is the smaller of input and output buffers.
    //
    ASSERT (m_pwfxIn->nChannels == 2);

    // if fStereoOut, we are not mixing, just scaleing
    //
    if (props.fStereoOut)
    {
        if (2 != m_pwfxOut->nChannels)
        {
            //
            //  a reconnect still has not occured, but we attach a media
            //  sample specifying a 'mono' output... so, we must treat this
            //  'mono' output buffer as 'stereo'
            //
            ccOut /= 2;
        }

        ccIn = min(ccIn, ccOut);

        for (ii = 0; ii < ccIn; ++ii, psIn += 2, psOut += 2)
        {
            // left channel
            //
            ss = (double)psIn[0] * m_dLeftLevel;
            psOut[0] = BOUND16I(ss);

            // right channel
            //
            ss = (double)psIn[1] * m_dRightLevel;
            psOut[1] = BOUND16I(ss);
        }
    }
    else // we are mixing
    {
        if (1 != m_pwfxOut->nChannels)
        {
            //
            //  a reconnect still has not occured, but we attach a media
            //  sample specifying a 'stereo' output... so, we must treat this
            //  'stereo' output buffer as 'mono'
            //
            ccOut *= 2;
        }

        ccIn = min(ccIn, ccOut);

        for (ii = 0; ii < ccIn; ++ii, psIn += 2, psOut += 1)
        {
            ss = (double)psIn[0] * m_dLeftLevel;
            ss += (double)psIn[1] * m_dRightLevel;
            ss *= m_dMonoLevel;

            psOut[0] = BOUND16I(ss);
        }
    }

    //
    //  return number of cells to write out and the number that we
    //  have consumed from the input stream.
    //
    *pccIn  = ccIn;
    *pccOut = ccIn;

    return hr;
}

#endif // FLOAT_SAMPLES_ONLY


#ifdef FLOAT_SAMPLES_SUPPORTED

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
    DWORD               ccIn         = (pccIn)  ? *pccIn  : 0; 
    DWORD               ccOut        = (pccOut) ? *pccOut : 0;
    HRESULT             hr           = S_OK;
    DWORD               ii;
    BOOL                fPropsChanged;
    TRANSFORM_PROPS     props;
    double              ss;

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

    //
    //  The number of sample cells (count cells == cc) that we can process
    //  is the smaller of input and output buffers.
    //
    ASSERT (m_pwfxIn->nChannels == 2);

    // are we mixing or not, if fStereoOut, we are not mixing
    //
    if (props.fStereoOut)
    {
        if (2 != m_pwfxOut->nChannels)
        {
            //
            //  a reconnect still has not occured, but we attach a media
            //  sample specifying a 'mono' output... so, we must treat this
            //  'mono' output buffer as 'stereo'
            //
            ccOut /= 2;
        }

        ccIn = min(ccIn, ccOut);

        for (ii = 0; ii < ccIn; ++ii, psIn += 2, psOut += 2)
        {
            // left channel
            //
            ss = psIn[0] * m_dLeftLevel;
            psOut[0] = (float)ss;

            // right channel
            //
            ss = psIn[1] * m_dRightLevel;
            psOut[1] = (float)ss;
        }
    }
    else // we are mixing
    {
        if (1 != m_pwfxOut->nChannels)
        {
            //
            //  a reconnect still has not occured, but we attach a media
            //  sample specifying a 'stereo' output... so, we must treat this
            //  'stereo' output buffer as 'mono'
            //
            ccOut *= 2;
        }

        ccIn = min(ccIn, ccOut);

        for (ii = 0; ii < ccIn; ++ii, psIn += 2, psOut += 1)
        {
            ss  = psIn[0] * m_dLeftLevel;
            ss += psIn[1] * m_dRightLevel;
            ss *= m_dMonoLevel;

            psOut[0] = (float)ss;
        }
    }

    //
    //  return number of cells to write out and the number that we
    //  have consumed from the input stream.
    //
    *pccIn  = ccIn;
    *pccOut = ccIn;

    return hr;
}

#endif // FLOAT_SAMPLES_SUPPORTED
