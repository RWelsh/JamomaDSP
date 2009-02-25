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

#include "sfstress.h"

//
//  constructor for transform class
//
CAudioTransform::CAudioTransform(
    IUnknown * punk,
    HRESULT  * phr)
    : m_fPropsChanged(TRUE)
    , m_fDirty(FALSE)
    , m_fInitialize(TRUE)
    , m_pwfxIn(NULL)
    , m_padLargePreset(NULL)
    , m_pwfxOut(NULL)
    , m_fCooked(FALSE)
    , m_fWriteFail(FALSE)
    , m_fReadFail(FALSE)
{  
    ZeroMemory(&m_prop, sizeof(m_prop));
    ZeroMemory(&m_aNotify, sizeof(m_aNotify));
    ZeroMemory(&m_Cooked, sizeof(m_Cooked));
}

CAudioTransform::~CAudioTransform()
{
    //
    //  be sure stuff is freed!
    //
    TransformEnd();

    if (NULL != m_padLargePreset)
    {
        HeapFree(GetProcessHeap(), 0, m_padLargePreset);
        m_padLargePreset = NULL;
    }
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
    UINT            csDelay;
    UINT            nChannels    = m_pwfxIn->nChannels;

    csDelay = MulDiv(pProps->lDelayMillisec, m_pwfxIn->nSamplesPerSec, 1000);
    m_Cooked.csDelay = m_Cooked.csDelayOrig = csDelay;
   
    //Clear current memory
    TransformEnd();

    DWORD cb = sizeof(FLOAT) * csDelay;
    m_Cooked.pafDelay = (LPFLOAT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
    if (!m_Cooked.pafDelay)
        return E_OUTOFMEMORY;

    if (1 < nChannels)
    {
        m_Cooked.pafDelay2 = (LPFLOAT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
        if ( ! m_Cooked.pafDelay2)
            return E_OUTOFMEMORY;
    }

    m_Cooked.lOutSize = m_Cooked.lBufferSize = pProps->lBufferSize;
    cb = m_Cooked.lBufferSize * sizeof(float);

    m_Cooked.pafInput = (LPFLOAT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
    if(!m_Cooked.pafInput)
        return E_OUTOFMEMORY;

    m_Cooked.pafOutput = (LPFLOAT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
    if(!m_Cooked.pafOutput)
        return E_OUTOFMEMORY;
   
    if ( 1 < nChannels)
    {
        m_Cooked.pafInput2 = (LPFLOAT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
        if( ! m_Cooked.pafInput2)
            return E_OUTOFMEMORY;

        m_Cooked.pafOutput2 = (LPFLOAT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
        if( ! m_Cooked.pafOutput2)
            return E_OUTOFMEMORY;
    }

    m_Cooked.lLoad = pProps->lLoad;

    m_Cooked.ix = 0;    

    return NOERROR;
}

HRESULT CAudioTransform::TransformCookProperties
(
    LPTRANSFORM_PROPS       pProps,
    TRANSFORM_STATE         nState
)
{
    HRESULT             hr;

    m_Cooked.lCount = 0;

    m_Cooked.dWetLevel = SfGetRatioFromdB(pProps->lWetLevel, SFSTRESS_WET_GAIN_MIN);
    m_Cooked.dDryLevel = SfGetRatioFromdB(pProps->lDryLevel, SFSTRESS_WET_GAIN_MIN);

    m_Cooked.fOverrun  = pProps->fOverrun;
    m_Cooked.fFloaterr = pProps->fFloaterr;


    if (WAVE_FORMAT_PCM == m_pwfxIn->wFormatTag)
    {
        switch (m_pwfxIn->wBitsPerSample)
        {
           #if !defined FLOAT_SAMPLES_ONLY
            case 16:
                m_fnEmptyBuffer = EmptyOutputBuffer_16;
                m_fnFillBuffer  = FillInputBuffer_16;
                break;
           #elif defined FLOAT_SAMPLES_SUPPORTED
            case 16:
                m_fnEmptyBuffer = EmptyOutputBuffer_Float;
                m_fnFillBuffer  = FillInputBuffer_Float;
                break;
           #endif
            default:
                return E_FAIL;
        }
    }
    else
    {
        switch (m_pwfxIn->wBitsPerSample)
        {
           #ifdef FLOAT_SAMPLES_SUPPORTED
            case 32:
                m_fnEmptyBuffer = EmptyOutputBuffer_Float;
                m_fnFillBuffer  = FillInputBuffer_Float;
                break;
           #endif

            default:
                return E_FAIL;
        }
    }


    hr = InitState(pProps, nState);

    //Buffer Management
    m_Cooked.lOutputIndex = m_Cooked.lInputIndex = 0;
    m_Cooked.fWorkStatus  = INPUT_DATA;

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
     
    //
    // Variable to eliminate double cooking and also force cooking with 
    // FirstBuffer notification in Transform16 or TransformFloat
    //
    m_fInitialize = FALSE;


    m_fInEOS              = FALSE;
    
    return hr;
}

HRESULT CAudioTransform::TransformEnd()
{
    if (!m_pwfxIn)
        return NOERROR;
    
    if (NULL != m_Cooked.pafDelay)
    {
        HeapFree(GetProcessHeap(), 0, m_Cooked.pafDelay);
        m_Cooked.pafDelay = NULL;
    }

    if (NULL != m_Cooked.pafInput)
    {
        HeapFree(GetProcessHeap(), 0, m_Cooked.pafInput);
        m_Cooked.pafInput = NULL;
    }

    if (NULL != m_Cooked.pafOutput)
    {
        HeapFree(GetProcessHeap(), 0, m_Cooked.pafOutput);
        m_Cooked.pafOutput = NULL;
    }

    if (2 == m_pwfxIn->nChannels)
    {
        if (NULL != m_Cooked.pafDelay2)
        {
            HeapFree(GetProcessHeap(), 0, m_Cooked.pafDelay2);
            m_Cooked.pafDelay2 = NULL;
        }

        if (NULL != m_Cooked.pafInput2)
        {
            HeapFree(GetProcessHeap(), 0, m_Cooked.pafInput2);
            m_Cooked.pafInput2 = NULL;
        }

        if (NULL != m_Cooked.pafOutput2)
        {
            HeapFree(GetProcessHeap(), 0, m_Cooked.pafOutput2);
            m_Cooked.pafOutput2 = NULL;
        } 
    }


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
//  is correct, the function should return S_OK, if it is not correct, if
//  not it should return S_FALSE
//
HRESULT CAudioTransform::TransformCheckOutputFormat (
    LPWAVEFORMATEX pwfxIn,
    LPWAVEFORMATEX pwfxOut)
{
    // we require the number of input channenls and the number of
    // output channels to be the same.
    //
    if (pwfxIn->nChannels != pwfxOut->nChannels) 
        return S_FALSE;

    //
    //  it's doubtful that _most_ plug-ins deal with other than mono
    //  or stereo...
    //
    ASSERT((1 == pwfxOut->nChannels) || (2 == pwfxOut->nChannels));

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
    ASSERT((1 == pwfx->nChannels) || (2 == pwfx->nChannels));

    return S_OK;
}

#endif // !NEED_CHANNELS_OUT




#if !defined FLOAT_SAMPLES_ONLY

void CAudioTransform::EmptyOutputBuffer_16(LPVOID pvOut, LPDWORD pccOut)
{
    DWORD cc,c;
    LPSHORT pafOut = (LPSHORT) pvOut;
     
    c = m_Cooked.lOutputUsed * m_pwfxIn->nChannels;
     
    for (cc = m_Cooked.lOutputUsed; cc < *pccOut; cc++)
    {
        pafOut[c++] = BOUND16I((m_Cooked.pafOutput[m_Cooked.lOutputIndex] * 32768.0));
        if (1 < m_pwfxIn->nChannels)
        {
            //stereo
            pafOut[c++] = BOUND16I((m_Cooked.pafOutput2[m_Cooked.lOutputIndex] * 32768.0));
        }

        m_Cooked.lOutputUsed++;
        m_Cooked.lOutputIndex++;
        if (m_Cooked.lOutputIndex == m_Cooked.lOutSize)
           break;
    }
}

void CAudioTransform::FillInputBuffer_16(LPVOID pvIn, LPDWORD pccIn)
{
     DWORD cc, c;
     LPSHORT psIn = (LPSHORT)pvIn;

     c = m_Cooked.lInputUsed * m_pwfxIn->nChannels;

     for (cc = m_Cooked.lInputUsed; cc < *pccIn; cc++)
     {
         m_Cooked.pafInput[m_Cooked.lInputIndex] = (float)(psIn[c++] * (1.0/32768.0));
         
         if (1 < m_pwfxIn->nChannels)
         {
            //stereo
            m_Cooked.pafInput2[m_Cooked.lInputIndex] = (float)(psIn[c++] * (1.0/32768.0));
         }

         m_Cooked.lInputUsed++;
         m_Cooked.lInputIndex++;
         if (m_Cooked.lInputIndex == m_Cooked.lOutSize)
            break;
     }
}

HRESULT CAudioTransform::Transform16bit
(
    TRANSFORM_STATE nState,
    LPSHORT psIn,
    LPDWORD pccIn,
    LPSHORT psOut,
    LPDWORD pccOut
)
{
    BOOL                fFirstBuffer = (TRANSFORM_STATE_FIRSTBUFFER == nState);
    BOOL                fTail        = (TRANSFORM_STATE_ENDOFSTREAM == nState);
    long                cChannels    = m_pwfxIn->nChannels;
    HRESULT             hr           = S_OK;
    BOOL                fPropsChanged;
    TRANSFORM_PROPS     props;
    DWORD               ccIn         = 0;     
    DWORD               ccOut        = 0;        

    if (pccIn)
        ccIn = *pccIn;

    if (pccOut)
        ccOut = *pccOut;

    m_fInEOS = fTail;
    
    //
    //  If the properties have changed, or we have never set them up at
    //  all, or on notification of discontinuity, init the transform 
    //  state here (if necessary)...
    //  NOTICE: We have choosen to not allow property cooking during tail
    //
    fPropsChanged = TransformGetProperties(&props);
    if (((fPropsChanged || !m_fCooked) && (!fTail)) || (fFirstBuffer && m_fInitialize))
    {
        hr = TransformCookProperties(&props, nState);
        if (FAILED(hr))
            return hr;
    }
    m_fInitialize = TRUE;

    if(m_Cooked.fOverrun)
        ccIn += m_Cooked.lCount * 10;

    if(m_Cooked.fFloaterr)
    {
        for(long ii = 0; ii < m_Cooked.lCount; ii++)
        {
            long blah  = 0/ii;
            long blah2 = blah++;
        }
    }

    hr = Process(psIn, &ccIn, psOut, &ccOut);     

    // return number of samples to write out and the number that we
    // have consumed from the input stream.
    //
    if (pccIn)        
        *pccIn  = ccIn;
    if (pccOut)
        *pccOut = ccOut;

    m_Cooked.lCount++;  //debug test variable

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
    long                cChannels    = m_pwfxIn->nChannels;
    HRESULT             hr           = S_OK;
    BOOL                fPropsChanged;
    TRANSFORM_PROPS     props;
    DWORD               ccIn         = 0;     
    DWORD               ccOut        = 0;        

    if (pccIn)
        ccIn = *pccIn;

    if (pccOut)
        ccOut = *pccOut;

    m_fInEOS = fTail;
    
    //
    //  If the properties have changed, or we have never set them up at
    //  all, or on notification of discontinuity, init the transform 
    //  state here (if necessary)...
    //  NOTICE: We have choosen to not allow property cooking during tail
    //
    fPropsChanged = TransformGetProperties(&props);
    if (((fPropsChanged || !m_fCooked) && (!fTail)) || (fFirstBuffer && m_fInitialize))
    {
        hr = TransformCookProperties(&props, nState);
        if (FAILED(hr))
            return hr;
    }
    m_fInitialize = TRUE;

    if(m_Cooked.fOverrun)
        ccIn += m_Cooked.lCount * 10;

    if(m_Cooked.fFloaterr)
    {
        for(long ii = 0; ii < m_Cooked.lCount; ii++)
        {
            long blah  = 0/ii;
            long blah2 = blah++;
        }
    }

    hr = Process(psIn, &ccIn, psOut, &ccOut);     

    // return number of samples to write out and the number that we
    // have consumed from the input stream.
    //
    if (pccIn)        
        *pccIn  = ccIn;
    if (pccOut)
        *pccOut = ccOut;

    m_Cooked.lCount++;  //debug test variable

    return hr;
}

#endif // FLOAT_SAMPLES_SUPPORTED

void CAudioTransform::EmptyOutputBuffer_Float(LPVOID pvOut, LPDWORD pccOut)
{
    DWORD cc,c;
    LPFLOAT pafOut = (LPFLOAT) pvOut;
     
    c = m_Cooked.lOutputUsed * m_pwfxIn->nChannels;
     
    for (cc = m_Cooked.lOutputUsed; cc < *pccOut; cc++)
    {
        pafOut[c++] = (float)m_Cooked.pafOutput[m_Cooked.lOutputIndex];
        if (1 < m_pwfxIn->nChannels)
        {
            //stereo
            pafOut[c++] = (float)m_Cooked.pafOutput2[m_Cooked.lOutputIndex];
        }

        m_Cooked.lOutputUsed++;
        m_Cooked.lOutputIndex++;
        if (m_Cooked.lOutputIndex == m_Cooked.lOutSize)
           break;
    }
}

void CAudioTransform::FillInputBuffer_Float(LPVOID pvIn, LPDWORD pccIn)
{
     DWORD cc, c;
     LPFLOAT psIn = (LPFLOAT)pvIn;

     c = m_Cooked.lInputUsed * m_pwfxIn->nChannels;

     for (cc = m_Cooked.lInputUsed; cc < *pccIn; cc++)
     {
         m_Cooked.pafInput[m_Cooked.lInputIndex] = psIn[c++];
         
         if (1 < m_pwfxIn->nChannels)
         {
            //stereo
            m_Cooked.pafInput2[m_Cooked.lInputIndex] = psIn[c++];
         }

         m_Cooked.lInputUsed++;
         m_Cooked.lInputIndex++;
         if (m_Cooked.lInputIndex == m_Cooked.lOutSize)
            break;
     }
}

HRESULT CAudioTransform::Process
(
    LPVOID                  psIn,
    LPDWORD                 pccIn,
    LPVOID                  psOut,
    LPDWORD                 pccOut
)
{   
    float   dDry, dDry2;
    float   dWet, dWet2;
    BOOL    fStereo = (m_pwfxIn->nChannels == 2) ? TRUE : FALSE;


    m_Cooked.lInputUsed = 0;
    m_Cooked.lOutputUsed = 0;

    // Check for very end
    if ((*pccIn == 0) && (m_Cooked.fWorkStatus == INPUT_DATA) && (m_Cooked.lInputIndex == 0) && (m_Cooked.csDelay <= 0))
    {
        *pccOut = 0;
        return (S_FALSE);
    }
    
    while (1)
    {
        // Empty output buff if filled with data
        if (m_Cooked.fWorkStatus == OUTPUT_DATA)
        {
            (this->*m_fnEmptyBuffer)(psOut, pccOut);
            if (m_Cooked.lOutputIndex == m_Cooked.lOutSize)
            {
                m_Cooked.lOutputIndex = 0;
                m_Cooked.lInputIndex = 0;
                m_Cooked.fWorkStatus = INPUT_DATA;
            }
            else
            {
                break; //didn't get to completely empty work buffer
            }
        }  


        if ((!psIn)  && (m_Cooked.lInputIndex == 0) && (m_Cooked.csDelay == 0))
            break;


        // Fill input buffer, zero tail if last, set InputUsed
        (this->*m_fnFillBuffer)(psIn, pccIn);

        // If input buffer is not completely filled, wait till next call
        if ((m_Cooked.lInputIndex != m_Cooked.lOutSize) && (!m_fInEOS))   
            break;

        if (m_fInEOS)
        {
           if (m_Cooked.lInputIndex != 0)
           {
               m_Cooked.lOutSize = m_Cooked.lInputIndex;
           }
           else
           {
               m_Cooked.lOutSize = min((DWORD)m_Cooked.lBufferSize, m_Cooked.csDelay);
               m_Cooked.csDelay -= m_Cooked.lOutSize;
               ZeroMemory(&(m_Cooked.pafInput[0]), m_Cooked.lBufferSize * sizeof(float));
               if (fStereo)
                   ZeroMemory(&(m_Cooked.pafInput2[0]), m_Cooked.lBufferSize * sizeof(float));
           }
        }

        // At this point, the work buffer is filled and output buffer is empty,
        ///////////////////PROCESS DATA////////////////////////////////
        if( ! fStereo)
        {
            for (long lTemp = 0; lTemp < m_Cooked.lOutSize; lTemp++)
            {
                dDry = m_Cooked.pafInput[lTemp];
                dWet = m_Cooked.pafDelay[m_Cooked.ix];
                m_Cooked.pafDelay[m_Cooked.ix] = dDry;

                if ((m_Cooked.ix += 1) >= m_Cooked.csDelayOrig)
                    m_Cooked.ix = 0;

                //Load simulates intensive processing
                for (UINT nLoadCounter = 0; nLoadCounter < ((UINT)m_Cooked.lLoad); nLoadCounter++)
                {
                    m_Cooked.pafOutput[lTemp] = (dWet * (float)m_Cooked.dWetLevel)
                                                + (dDry * (float)m_Cooked.dDryLevel);
                }
            }
        }
        else
        {
            for (long lTemp = 0; lTemp < m_Cooked.lOutSize; lTemp++)
            {
                dDry = m_Cooked.pafInput[lTemp];
                dWet = m_Cooked.pafDelay[m_Cooked.ix];

                dDry2 = m_Cooked.pafInput2[lTemp];
                dWet2 = m_Cooked.pafDelay2[m_Cooked.ix];

                m_Cooked.pafDelay[m_Cooked.ix] = dDry;
                m_Cooked.pafDelay2[m_Cooked.ix] = dDry2;

                if ((m_Cooked.ix += 1) >= m_Cooked.csDelayOrig)
                    m_Cooked.ix = 0;

                // Load simulates intensive processing
                //
                for (UINT nLoadCounter = 0; nLoadCounter < ((UINT)m_Cooked.lLoad); nLoadCounter++)
                {
                    m_Cooked.pafOutput[lTemp] = (dWet * (float)m_Cooked.dWetLevel)
                                                + (dDry * (float)m_Cooked.dDryLevel);

                    m_Cooked.pafOutput2[lTemp] = (dWet2 * (float)m_Cooked.dWetLevel)
                                                 + (dDry2 * (float)m_Cooked.dDryLevel);
                }
            }
        }
        ////////////////DONE PROCESS DATA////////////////////////////////

        m_Cooked.fWorkStatus = OUTPUT_DATA;

    }

    *pccIn  = m_Cooked.lInputUsed;
    *pccOut = m_Cooked.lOutputUsed;

    return (NOERROR);
}
