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

#include "sfpan.h"

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

    if (pProps->lOutGain <= SFPAN_OUT_GAIN_MIN)
    {
        m_dLevelL = 0.0;
        m_dLevelR = 0.0;
    }
    else
    {
        double      dOutGain;

        dOutGain  = SfGetRatioFromdB(pProps->lOutGain, SFPAN_OUT_GAIN_MIN);
        m_dLevelL = (double)(SFPAN_FULL_RIGHT - pProps->lPanLR) / SFPAN_FULL_RIGHT;
        m_dLevelR = (double)(SFPAN_FULL_RIGHT + pProps->lPanLR) / SFPAN_FULL_RIGHT;

        //
        //  combine these so we have only one multiply per sample
        //
        m_dLevelL *= dOutGain;
        m_dLevelR *= dOutGain;
    }

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
    //  we will grow by a maximum of two (mono in to stereo out)
    //  pProp is defaulted to the current allocator's defaults...
    //
    if ((1 == m_pwfxIn->nChannels) &&
        (2 == m_pwfxOut->nChannels))
    {
        pProp->cbBuffer = pProp->cbBuffer * 2;
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
HRESULT CAudioTransform::TransformCheckOutputFormat(
    LPWAVEFORMATEX pwfxIn,
    LPWAVEFORMATEX pwfxOut)
{
    //
    //  !!! this function is not used in SFPAN because we always give a
    //      stereo out. NEED_CHANNELS_OUT = 2
    //
    //  it's doubtful that _most_ plug-ins deal with other than mono
    //  or stereo...
    //
    ASSERT ((1 == pwfxOut->nChannels) || (2 == pwfxOut->nChannels));

    return S_OK;
}


//
//  This function is called with the current output format it should change
//  the output format to match that requested by TRANSFORM_PROPS and return
//  success/fail
//
HRESULT CAudioTransform::TransformTweakOutputFormat(
    LPWAVEFORMATEX pwfx)
{
    //
    //  !!! this function is not used in SFPAN because we always give a
    //      stereo out. NEED_CHANNELS_OUT = 2
    //
    //  it's doubtful that _most_ plug-ins deal with other than mono
    //  or stereo...
    //
    ASSERT ((1 == pwfx->nChannels) || (2 == pwfx->nChannels));
    pwfx->nChannels = 2;
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
    ccIn = min(ccIn, ccOut);

    //
    //  are we going from mono to stereo, or stereo to stereo?
    //
    if (1 == m_pwfxIn->nChannels)
    {
        //  -- mono to stereo --  //

        ASSERT((1 == m_pwfxIn->nChannels) && (2 == m_pwfxOut->nChannels));

        for (ii = 0; ii < ccIn; ++ii, psIn += 1, psOut += 2)
        {
            double  ssIn = psIn[0];

            ss = ssIn * m_dLevelL;
            psOut[0] = BOUND16I(ss);

            ss = ssIn * m_dLevelR;
            psOut[1] = BOUND16I(ss);
        }
    }
    else
    {
        //  -- stereo to stereo --  //

        ASSERT((2 == m_pwfxIn->nChannels) && (2 == m_pwfxOut->nChannels));

        if (SFPAN_MODE_PRESERVE_SEPARATION == props.lMode)
        {
            //
            //  SFPAN_MODE_PRESERVE_SEPARATION
            //
            for (ii = 0; ii < ccIn; ++ii, psIn += 2, psOut += 2)
            {
                ss = psIn[0] * m_dLevelL;
                psOut[0] = BOUND16I(ss);

                ss = psIn[1] * m_dLevelR;
                psOut[1] = BOUND16I(ss);
            }
        }
        else
        {
            //
            //  SFPAN_MODE_MIX_BEFORE_PAN
            //
            for (ii = 0; ii < ccIn; ++ii, psIn += 2, psOut += 2)
            {
                double ssMix = (double)psIn[0] + (double)psIn[1];

                ss = ssMix * m_dLevelL;
                psOut[0] = BOUND16I(ss);

                ss = ssMix * m_dLevelR;
                psOut[1] = BOUND16I(ss);
            }
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
    ccIn = min(ccIn, ccOut);

    //
    //  are we going from mono to stereo, or stereo to stereo?
    //
    if (1 == m_pwfxIn->nChannels)
    {
        //  -- mono to stereo --  //

        ASSERT((1 == m_pwfxIn->nChannels) && (2 == m_pwfxOut->nChannels));

        for (ii = 0; ii < ccIn; ++ii, psIn += 1, psOut += 2)
        {
            ss = psIn[0] * m_dLevelL;
            psOut[0] = (float)ss;

            ss = psIn[0] * m_dLevelR;
            psOut[1] = (float)ss;
        }
    }
    else
    {
        //  -- stereo to stereo --  //

        ASSERT((2 == m_pwfxIn->nChannels) && (2 == m_pwfxOut->nChannels));

        if (SFPAN_MODE_PRESERVE_SEPARATION == props.lMode)
        {
            //
            //  SFPAN_MODE_PRESERVE_SEPARATION
            //
            for (ii = 0; ii < ccIn; ++ii, psIn += 2, psOut += 2)
            {
                ss = psIn[0] * m_dLevelL;
                psOut[0] = (float)ss;

                ss = psIn[1] * m_dLevelR;
                psOut[1] = (float)ss;
            }
        }
        else
        {
            //
            //  SFPAN_MODE_MIX_BEFORE_PAN
            //
            for (ii = 0; ii < ccIn; ++ii, psIn += 2, psOut += 2)
            {
                ss = psIn[0] + psIn[1];

                psOut[0] = (float)(ss * m_dLevelL);
                psOut[1] = (float)(ss * m_dLevelR);
            }
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
