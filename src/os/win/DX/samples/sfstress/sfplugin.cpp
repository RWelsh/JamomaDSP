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
//
//==========================================================================;

#ifndef STRICT
#define STRICT
#endif
#ifndef INC_OLE2
#define INC_OLE2
#endif
#include <streams.h>              // ActiveMovie, includes windows.h
#include <initguid.h>
#include <olectl.h>

#include <sfiface.h>             // IStaticFilterPreset interface

#include "sfstress.h"            // ISfPlugInProp (properties) interface
#include "sfppage.h"             // CSfPlugInPropPage

#ifndef _WTEXT
#define _WTEXT(str) L##str
#define WTEXT(str) _WTEXT(str)
#endif

//
// JMK: 11-July-97
//
// This was created to fix the bug where an application was calling
// QueryAccept on our input pin while the output pin was connected.
// the CTransformFilter behavior was to fail QueryAccept if we cannot
// accept the media type RIGHT NOW. (i.e. if we can't do the transform
// from that media type to the currently connected output type).
//
// Unfortunately, filters based on CInPlaceTransform, don't bother to
// check to see if the output is connected on QueryAccept, so THEY work
// in these applications, so this is arguably a bug in CTransformFilter 
// rather than these applications.
//
// Our fix is to override CTransformInputPin::CheckMediaType (called by
// QueryAccept) so that if our output pin IS connected, and we CANT do
// the transform, ask the downstream input pin if it could tolerate
// being reconnected so that our input type and our output type match
// if this succeeds, then succeed QueryAccept.
//
// To make this work, however, we now have to be looking out for someone
// connecting our input pin with a format that we can't transform and when
// THAT happens, call the graph to force a reconnect of our output pin.
//
// We override CSfPlugIn::CompleteConnect to handle that chore. the other
// mods you see are plumbing to override creation of the input pin so that
// it is our class rather than the standard CTransformInputPin class.
//

// create a new classed based on CTransformInputPin
//
class CSfPlugInInputPin : public CTransformInputPin
{
   public:
      CSfPlugInInputPin (
         TCHAR *            pObjectName,
         CTransformFilter * pFilter,
         HRESULT          * phr,
         LPCWSTR            pName)
      : CTransformInputPin (pObjectName, pFilter, phr, pName)
      {
      };

      HRESULT CheckMediaType(const CMediaType* pmt);
};

//
//
//
class CSfPlugIn
    // Inherited classes
    : public CTransformFilter          // Main ActiveMovie interfaces

    , public CAudioTransform           // this class does the actual
                                       // transform.  you should
                                       // NOT need to mess with any
                                       // of the code in the CSfPlugIn class

    , public ISpecifyPropertyPages     // so that someone can find out what our
                                       // property page interface GUID(s) are

    , public IPersistStream            // Implements IPersistStream
                                       // to alow saving of properties
                                       // in a saved graph.

    , public IStaticFilterPreset       // allows filter owner to get preset
                                       // names and use presets.
{
public:

    // override GetPin so that we can return more specialized pin objects
    // (specifically so that we can override the input pin class)
    //
    virtual CBasePin *GetPin(int n);

    // override CompleteConnect so that we can check for input connections
    // that require us to force an output reconnect
    //
    HRESULT CompleteConnect(PIN_DIRECTION dir, IPin * pReceivePin);

protected:

    // this hack is so that we can get at the output pin from the input pin
    // methods.
    //
    friend class CSfPlugInInputPin;
    CTransformOutputPin * OutputPin() {
       return (CTransformOutputPin *) m_pOutput;
       };

public:
    static CUnknown *CreateInstance(LPUNKNOWN punk, HRESULT *phr);
    DECLARE_IUNKNOWN;

    //
    // --- CTransformFilter Overrides --
    //
    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
    HRESULT DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES *pprop);

    // basic flow-of control methods
    //
    HRESULT Receive(IMediaSample *pSample);
   #ifdef NEED_END_OF_STREAM
    HRESULT EndOfStream(void);
   #endif
    HRESULT StartStreaming(void);
    HRESULT StopStreaming(void);

    // override to suggest OUTPUT pin media types
    HRESULT GetMediaType(int iPosition, CMediaType * pmt);

    // override to know what media type was chosen
    HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);

    // Basic COM - used here to reveal our property interface.
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    // --- ISpecifyPropertyPages ---
    STDMETHODIMP GetPages(CAUUID * pPages);

    // ---- IPersistStream ------
    STDMETHODIMP IsDirty() {
       return TransformIsDirty();
       };
    STDMETHODIMP Load(LPSTREAM pStm) {
        HRESULT hr;
        BOOL fReadFail;

        hr = TransformRead(pStm);
        fReadFail = (FAILED(hr))? TRUE : FALSE;
        put_ReadFail(fReadFail);
       return hr; 
       };
    STDMETHODIMP Save(LPSTREAM pStm, BOOL fClearDirty) {
        HRESULT hr;
        BOOL fWriteFail;

        hr = TransformWrite(pStm, fClearDirty);
        fWriteFail = (FAILED(hr))? TRUE : FALSE;
        put_WriteFail(fWriteFail);
       return hr;
       };
    STDMETHODIMP GetSizeMax (ULARGE_INTEGER * pcbSize) {
       pcbSize->QuadPart = TransformPersistSize();
       return NOERROR;
       };
    STDMETHODIMP GetClassID (CLSID * pClsid);

    // setup helper
    LPAMOVIESETUP_FILTER GetSetupData();

    // IStaticFilterPreset - interface to control filter use of static presets
    //
    STDMETHODIMP GetPresetCount(long * piCount);
    STDMETHODIMP GetPresetName(long index, LPOLESTR pszName, DWORD cchName);
    STDMETHODIMP UsePreset(long index);

    // Helper for GetMediaType & CheckMediaType
    //
    HRESULT SuggestOutputFormat (CMediaType * pmt);

private:
    // Constructor/destructor
    CSfPlugIn(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);
    ~CSfPlugIn();

    // Overrides the PURE virtual Transform of CTransformFilter base class
    // This is where the "real work" is done.
    HRESULT Transform(IMediaSample *pSampleIn, IMediaSample *pSampleOut);

    // If there are multiple instances of this filter active, it's
    // useful for debug messages etc. to know which one this is.
    // This variable has no other purpose.
    static int          m_nInstanceCount;     // total instances
    int                 m_nThisInstance;

    WAVEFORMATEX        m_wfxIn;              // current wave format (not including compressor specific data).
    WAVEFORMATEX        m_wfxOut;             // ouput wave format
   #ifdef NEED_END_OF_STREAM
    BOOL                m_fInEOS;             // true while in EndOfStream
   #endif
    BOOL                m_fGotABuffer;        // TRUE if we got at least 1 buffer

    BOOL                m_fSendDiscontinuity; // Variable used to propagate a FIRSTBUFFER
                                              // is necessary after a 0 sample 
    BOOL                m_fSyncPoint;
    REFERENCE_TIME      m_tStart;
    REFERENCE_TIME      m_tStop;
    REFERENCE_TIME      m_tMediaStart;
    REFERENCE_TIME      m_tMediaStop;
};

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------

#ifdef FLOAT_SAMPLES_ONLY

  static void __inline ConvertFloatTo16 (
      LPSHORT psOut,
      LPFLOAT psIn,
      DWORD    cs)
  {
      while (cs > 0)
      {
          int s = (int)(*psIn * 32768.0);
          *psOut = BOUND16I(s);
          ++psOut;
          ++psIn;
          --cs;
      }
  }

  static void __inline Convert16ToFloat (
      LPFLOAT psOut,
      LPSHORT psIn,
      DWORD    cs)
  {
      while (cs > 0)
      {
          *psOut = (float)(*psIn / 32768.0);
          ++psOut;
          ++psIn;
          --cs;
      }
  }

#endif

// Put out the name of a function and instance on the debugger.
// Invoke this at the start of functions to allow a trace.
//
#define DbgFunc(a) DbgLog(( LOG_TRACE                   \
                          , 2                           \
                          , TEXT("CSfPlugIn(%d)::%s")   \
                          , m_nThisInstance             \
                          , TEXT(a)                     \
                         ));

   // Self-registration data structures
   //
   static AMOVIESETUP_MEDIATYPE g_sudPinTypes =
   {
         &MEDIATYPE_Audio         // clsMajorType
       , &MEDIASUBTYPE_NULL       // clsMinorType
   };

   static AMOVIESETUP_PIN g_psudPins[] =
   {
       { L"Input"            // strName
       , FALSE               // bRendered
       , FALSE               // bOutput
       , FALSE               // bZero
       , FALSE               // bMany
       , &CLSID_NULL         // clsConnectsToFilter
       , L"Output"           // strConnectsToPin
       , 1                   // nTypes
       , &g_sudPinTypes        // lpTypes
       }
     , { L"Output"           // strName
       , FALSE               // bRendered
       , TRUE                // bOutput
       , FALSE               // bZero
       , FALSE               // bMany
       , &CLSID_NULL         // clsConnectsToFilter
       , L"Input"            // strConnectsToPin
       , 1                   // nTypes
       , &g_sudPinTypes        // lpTypes
       }
   };

   static AMOVIESETUP_FILTER g_sudSfPlugIn =
   {
         &CLSID_SfPlugIn      // class id
       , WTEXT(SFPLUGIN_NAME) // strName
       , MERIT_DO_NOT_USE     // dwMerit
       , 2                    // nPins
       , g_psudPins             // lpPin
   };


// Needed for the CreateInstance mechanism
CFactoryTemplate g_Templates[] =
{
    {
        WTEXT(SFPLUGIN_NAME),
        &CLSID_SfPlugIn,
        CSfPlugIn::CreateInstance,
        NULL,
        &g_sudSfPlugIn
    },

    {
        WTEXT(SFPLUGIN_NAME) L" Property Page",
        &CLSID_SfPlugInPropPage,
        CSfPlugInPropPage::CreateInstance
    },

    #ifdef MULTIPLE_PROPERTY_PAGES
    {
        WTEXT(SFPLUGIN_NAME) L" Property Page 2",
        &CLSID_SfPlugInPropPage2,
        CSfPlugInPropPage2::CreateInstance
    },

    {
        WTEXT(SFPLUGIN_NAME) L" Property Page 3",
        &CLSID_SfPlugInPropPage3,
        CSfPlugInPropPage3::CreateInstance               
    },
    #endif
};

int g_cTemplates = NUMELMS(g_Templates);

// initialize the static instance count.
//
int CSfPlugIn::m_nInstanceCount = 0;

#ifdef _DEBUG
void DbgDumpMediaType(const CMediaType *pmt)
{
    DbgLog((LOG_TRACE,2,TEXT("Format length %d"),pmt->cbFormat));
    DbgLog((LOG_TRACE,2,TEXT("Major type %s"),GuidNames[pmt->majortype]));
    DbgLog((LOG_TRACE,2,TEXT("Subtype %s"),GuidNames[pmt->subtype]));
    DbgLog((LOG_TRACE,2,TEXT("Fixed size sample %d"),pmt->bFixedSizeSamples));
    DbgLog((LOG_TRACE,2,TEXT("Temporal compression %d"),pmt->bTemporalCompression));
    DbgLog((LOG_TRACE,2,TEXT("Sample size %d"),pmt->lSampleSize));
    DbgLog((LOG_TRACE,2,TEXT("Format size %d"),pmt->cbFormat));
}
void DbgDumpWfx(LPWAVEFORMATEX pwfx)
{
    DbgLog((LOG_TRACE,2,TEXT("  wFormatTag %u"), pwfx->wFormatTag));
    DbgLog((LOG_TRACE,2,TEXT("  nSamplesPerSec %lu"), pwfx->nSamplesPerSec));
    DbgLog((LOG_TRACE,2,TEXT("  wBitsPerSample %u"), pwfx->wBitsPerSample));
    DbgLog((LOG_TRACE,2,TEXT("  nChannels %u"), pwfx->nChannels));
    DbgLog((LOG_TRACE,2,TEXT("  nBlockAlign %u"), pwfx->nBlockAlign));
    DbgLog((LOG_TRACE,2,TEXT("  nAvgBytesPerSec %lu"), pwfx->nAvgBytesPerSec));
    if (WAVE_FORMAT_PCM != pwfx->wFormatTag)
    {
        DbgLog((LOG_TRACE,2,TEXT("  cbSize %u"), pwfx->cbSize));
    }
}
#else
#define DbgDumpMediaType(pmt)
#define DbgDumpWfx(pwfx)
#endif


//
// CSfPlugIn::Constructor
//
// Construct a CSfPlugIn object.
//
CSfPlugIn::CSfPlugIn(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr)
    : CTransformFilter (tszName, punk, CLSID_SfPlugIn)
    , CAudioTransform(punk, phr)
{
    // Useful for debug, no other purpose
    m_nThisInstance = ++m_nInstanceCount;
    DbgFunc("CSfPlugIn");

    // default filter properties to preset 0
    //
    UsePreset(0);

    m_fSendDiscontinuity    = FALSE;
    m_fSyncPoint = FALSE;
    m_tStart = m_tStop = 0;
    m_tMediaStart = m_tMediaStop = 0;
}

CSfPlugIn::~CSfPlugIn()
{
}

//
// CreateInstance
//
// Override CClassFactory method.
// Provide the way for COM to create a CSfPlugIn object.
//
CUnknown *CSfPlugIn::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CSfPlugIn *pNewObject = new CSfPlugIn(NAME(SFPLUGIN_NAME), punk, phr);
    if (pNewObject == NULL)
        *phr = E_OUTOFMEMORY;

    return pNewObject;
}


//
// GetSetupData -
//
// Override CBaseFilter method.
// Part of the self-registration mechanism.
//
LPAMOVIESETUP_FILTER CSfPlugIn::GetSetupData ()
{  
   return &g_sudSfPlugIn;
}


//
// NonDelegatingQueryInterface
//
// Override CUnknown method.
// Reveal our persistent stream, property pages and ISfPlugInProp interfaces.
// Anyone can call our private interface so long as they know the secret UUID.
//
STDMETHODIMP CSfPlugIn::NonDelegatingQueryInterface (
    REFIID  riid,
    void ** ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_ISfPlugInProp)
        return GetInterface((ISfPlugInProp *) this, ppv);
    else if (riid == IID_ISpecifyPropertyPages)
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
    else if (riid == IID_IPersistStream)
        return GetInterface((IPersistStream *) this, ppv);
    else if (riid == IID_IStaticFilterPreset)
        return GetInterface((IStaticFilterPreset *) this, ppv);
    else
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
}


// GetClassID
//
// Override CBaseMediaFilter method for interface IPersist
// Part of the persistent file support.  We must supply our class id
// which can be saved in a graph file and used on loading a graph with
// a SfPlugIn in it to instantiate this filter via CoCreateInstance.
//
STDMETHODIMP CSfPlugIn::GetClassID(CLSID *pClsid)
{
    if (pClsid==NULL)
        return E_POINTER;
    *pClsid = CLSID_SfPlugIn;
    return NOERROR;
}

// GetPin
//
// return a non-addrefed CBasePin * for the user to addref if he holds onto it
// for longer than his pointer to us. We create the pins dynamically when they
// are asked for rather than in the constructor. This is because we want to
// give the derived class an oppportunity to return different pin objects
//
// As soon as any pin is needed we create both since we are going to need
// the other pin shortly anyway...
//
CBasePin * CSfPlugIn::GetPin(int n)
{
    HRESULT hr = S_OK;

    // Create an input pin if not already done
    //
    if ( ! m_pInput)
    {
        m_pInput = new CSfPlugInInputPin(NAME("SfPlugIn input pin"),
                                         this,       // Owner filter
                                         &hr,        // Result code
                                         L"Input");  // Pin name

        // a failed return code should delete the object
        //
        if (FAILED(hr))
        {
            delete m_pInput;
            m_pInput = NULL;
        }
    }

    // Create an output pin if not already done

    if ( ! m_pOutput)
    {
        m_pOutput = new CTransformOutputPin(NAME("SfPlugIn output pin"),
                                            this,        // Owner filter
                                            &hr,          // Result code
                                            L"Output");  // Pin name

        // a failed return code should delete the object
        //
        if (FAILED(hr))
        {
            delete m_pOutput;
            m_pOutput = NULL;
        }
    }

    // Return the appropriate pin
    //
    ASSERT ( n >= 0 && n <= 1);
    if (0 == n)
        return m_pInput;
    else if (1 == n)
        return m_pOutput;

    return NULL;
}

//
// Transform
//
// Override CTransformFilter method.
// Convert the input ActiveMovie sample into the output ActiveMovie sample.
//
HRESULT CSfPlugIn::Transform (
    IMediaSample *pSampleIn,
    IMediaSample *pSampleOut)
{
    HRESULT  hr = E_INVALIDARG;
    DbgFunc("Transform");
    ASSERT (0);
    return hr;
}


//
// Receive
//
// Override CTransformFilter method convert input sample into
// one or more output samples.
//
HRESULT CSfPlugIn::Receive(
    IMediaSample   *pSampleIn)
{
    IMediaSample   *pSampleOut;
    HRESULT         hr;
    LPBYTE          pbIn;
    DWORD           cbIn;
    DWORD           ccIn;
    LPBYTE          pbOut;
    DWORD           cbOut;
    DWORD           ccOut;
    DWORD           cbUsed;

    // If no output to deliver to then no point sending us data
    //
    ASSERT(pSampleIn && m_pOutput);
    
    // this may block for an indeterminate amount of time
    //
    hr = m_pOutput->GetDeliveryBuffer(&pSampleOut, NULL, NULL, 0);
    if (FAILED(hr))
    {
       return hr;
    }

    TRANSFORM_STATE nState = (S_OK == pSampleIn->IsDiscontinuity()) ?
                TRANSFORM_STATE_FIRSTBUFFER : TRANSFORM_STATE_NORMAL;

    ASSERT(pSampleOut);

    m_fSyncPoint |= (S_OK == pSampleIn->IsSyncPoint());

    if (TRANSFORM_STATE_FIRSTBUFFER == nState)
    {
        m_fSendDiscontinuity = TRUE;

        // default - times are the same
        //
        pSampleIn->GetTime(&m_tStart, &m_tStop);

        // Copy the media times
        //
        pSampleIn->GetMediaTime(&m_tMediaStart, &m_tMediaStop);
    }


    // Start timing the transform (if PERF is defined)
    //
    MSR_START(m_idTransform);

    // Get the details of the data (address, length)
    //
    cbIn = pSampleIn->GetActualDataLength();
    pSampleIn->GetPointer(&pbIn);
    DbgLog((LOG_TRACE,3,TEXT("Receive in cbBuffer=%d, cbData=%d"),pSampleIn->GetSize(), cbIn));

    // if the GUI is allowed to change the output channel format,
    // then we need to check here that the current format is
    // valid.
    //
   #if !defined NEED_CHANNELS_OUT
    if (S_OK != TransformCheckOutputFormat(&m_wfxIn, &m_wfxOut))
    {
        CMediaType mt;
        GetMediaType (0, &mt);
        pSampleOut->SetMediaType (&mt);
    }
   #endif
    
    for (;;)
    {
        cbOut = pSampleOut->GetSize();
        pSampleOut->GetPointer(&pbOut);
        ASSERT(pbOut && cbOut);

        //
        //  convert count in bytes (cb) to count in cells (cc)
        //
        ccOut = cbOut / m_wfxOut.nBlockAlign;
        ccIn  = cbIn  / m_wfxIn.nBlockAlign;

        switch (m_wfxIn.wFormatTag)
        {
            case WAVE_FORMAT_PCM:
               #if !defined FLOAT_SAMPLES_ONLY
                hr = Transform16bit(nState, (LPSHORT)pbIn, &ccIn, (LPSHORT)pbOut, &ccOut);
               #else
                //
                //  if we have a transform for float samples only, but we
                //  connected as 16 bit PCM, then convert 16 bit to float
                //  before doing the transform and then back after
                //
                //  !!! this temp buffer allocation should NOT be done here !!!
                //      this is a serious performance hit!!!
                //
                {
                    hr = E_OUTOFMEMORY;
                    DWORD   csIn        = ccIn * m_wfxIn.nChannels;
                    DWORD   cbTempIn    = csIn * sizeof(float);
                    LPFLOAT psTempIn    = (LPFLOAT)HeapAlloc(GetProcessHeap(), 0, cbTempIn);
                    DWORD   csOut       = ccOut * m_wfxOut.nChannels;
                    DWORD   cbTempOut   = csOut * sizeof(float);
                    LPFLOAT psTempOut   = (LPFLOAT)HeapAlloc(GetProcessHeap(), 0, cbTempOut);

                    if ((NULL != psTempIn) && (NULL != psTempOut))
                    {
                        Convert16ToFloat(psTempIn, (LPSHORT)pbIn, csIn);
                        hr = TransformFloat(nState, psTempIn, &ccIn, psTempOut, &ccOut);
                        ConvertFloatTo16((LPSHORT)pbOut, psTempOut, csOut);
                        HeapFree(GetProcessHeap(), 0, psTempIn);
                        HeapFree(GetProcessHeap(), 0, psTempOut);
                    }
                }
               #endif
                break;           

                
           #ifdef FLOAT_SAMPLES_SUPPORTED
            case WAVE_FORMAT_IEEE_FLOAT:
                hr = TransformFloat(nState, (LPFLOAT)pbIn, &ccIn, (LPFLOAT)pbOut, &ccOut);
                break;
           #endif
        }

        //
        //  convert count in cells (cc) to count in bytes (cb)
        //
        cbOut  = ccOut * m_wfxOut.nBlockAlign;
        cbUsed = ccIn  * m_wfxIn.nBlockAlign;

        DbgLog((LOG_TRACE,3,TEXT("Receive out cbBuffer=%d, cbData=%d (cbUsed=%d)"),
                pSampleOut->GetSize(), cbOut, cbUsed));

        // did the transform succeed?
        //
        if (FAILED(hr))
        {
            DbgLog((LOG_TRACE,1,TEXT("Error from transform %.08lXh"),hr));
            break;
        }
        else if (cbOut > 0) // was there anything in the output buffer?
        {
            if (m_fSendDiscontinuity)
            {
                pSampleOut->SetDiscontinuity(TRUE);
                pSampleOut->SetTime(&m_tStart, &m_tStop);
                pSampleOut->SetMediaTime(&m_tMediaStart, &m_tMediaStop);
                m_fSendDiscontinuity = FALSE;
            }

            if (m_fSyncPoint)
            {
                pSampleOut->SetSyncPoint(TRUE);
                m_fSyncPoint = FALSE;
            }

            pSampleOut->SetActualDataLength(cbOut);
            hr = m_pOutput->Deliver(pSampleOut);
            if (FAILED(hr))
            {
                DbgLog((LOG_TRACE,1,TEXT("Error from Deliver %.08lXh"),hr));
                break;
            }
        }
        
        
        // did we use all of the input buffer? if so, then we are done
        // with the loop.
        //
        if (cbUsed >= cbIn)
        {
            //ASSERT(cbUsed <= cbIn);
            break;
        }

        // update buffer pointers to account for the input data
        // that has already been consumed
        //
        pbIn += cbUsed;
        cbIn -= cbUsed;

        nState = TRANSFORM_STATE_NORMAL;

        // if we delivered the old output buffer, release it
        // and get a new one, otherwise, we will re-use it.
        //
        if (cbOut > 0)
        {
            // release the current output buffer. If the connected pin
            // still needs it, it will have addrefed it itself.
            //
            pSampleOut->Release();
            pSampleOut = NULL;

            // get another delivery buffer
            //
            hr = m_pOutput->GetDeliveryBuffer(&pSampleOut, NULL, NULL, 0);
            if (FAILED(hr))
            {
                DbgLog((LOG_TRACE,1,TEXT("Failed to get delivery buffer %.08lXh"),hr));
                break;
            }
        }
    }

    //
    //  Stop the clock and log it (if PERF is defined)
    //
    MSR_STOP(m_idTransform);

    //
    //  Release the output buffer. If the connected pin still needs it,
    //  it will have addrefed it itself.
    //
    if (NULL != pSampleOut)
    {
        pSampleOut->Release();
    }

    return hr;
}


#ifdef NEED_END_OF_STREAM
//
//  EndOfStream received. Spit out all queued data and then tell the
//  downstream filter about EOS.
//
HRESULT CSfPlugIn::EndOfStream(
    void)
{
    HRESULT         hr = NOERROR;

    if (NULL == m_pOutput)
       return NOERROR;

    DbgFunc("EndOfStream");

    for (;;)
    {
        IMediaSample   *pSample;
        DWORD           cc;
        DWORD           cb;
        LPSHORT         pbBuffer;

        // this may block for an indeterminate amount of time
        //
        hr = m_pOutput->GetDeliveryBuffer(&pSample, NULL, NULL, 0);
        if (FAILED(hr))
           return hr;

        cb = pSample->GetSize();
        pSample->GetPointer((LPBYTE *)&pbBuffer);
        ASSERT(pbBuffer && cb);

        //
        //  convert count in bytes (cb) to count in cells (cc)
        //
        cc = cb / m_wfxOut.nBlockAlign;

        switch (m_wfxIn.wFormatTag)
        {
           #if !defined FLOAT_SAMPLES_ONLY
            case WAVE_FORMAT_PCM:
                hr = Transform16bit(TRANSFORM_STATE_ENDOFSTREAM, NULL, NULL, (LPSHORT)pbBuffer, &cc);
                break;
           #endif

           #ifdef FLOAT_SAMPLES_SUPPORTED
            case WAVE_FORMAT_IEEE_FLOAT:
                hr = TransformFloat(TRANSFORM_STATE_ENDOFSTREAM, NULL, NULL, (LPFLOAT)pbBuffer, &cc);
                break;
           #endif

        }

        //
        //  convert count in cells (cc) to count in bytes (cb)
        //
        cb = cc * m_wfxOut.nBlockAlign;

        // the transform will return S_FALSE when there is no more
        // output data, keep looping intil that occurs or until
        // the transform fails.
        //
        if (hr == S_FALSE)
        {
            hr = NOERROR;
            if (cb > 0)
            {
                pSample->SetActualDataLength(cb);
                hr = m_pOutput->Deliver(pSample);
            }

            pSample->Release();
            break;
        }
        else if ( ! FAILED(hr))
        {
            pSample->SetActualDataLength(cb);
            hr = m_pOutput->Deliver(pSample);
            pSample->Release();
            if (FAILED(hr))
            {
                DbgLog((LOG_TRACE,1,TEXT("Error from Deliver (EOS) %.08lXh"),hr));
                break;
            }
        }
        else
        {
            DbgLog((LOG_TRACE,1,TEXT("Transform failed (EOS) %.08lXh"),hr));
            pSample->Release();
        }
    }

    // now tell the downstream filter about end of stream
    //
    HRESULT hrDeliver = m_pOutput->DeliverEndOfStream();

    return FAILED(hr) ? hr : hrDeliver;
}

#endif // NEED_END_OF_STREAM

HRESULT CSfPlugIn::StartStreaming(void)
{
    DbgFunc("StartStreaming");
   #ifdef NEED_END_OF_STREAM
    m_fInEOS = FALSE;
   #endif
    m_fGotABuffer = FALSE;
    return TransformBegin(&m_wfxIn, &m_wfxOut);
}

// called on a pause or run -> stop transition
//
HRESULT CSfPlugIn::StopStreaming(void)
{
    DbgFunc("StopStreaming");
    return TransformEnd();
}

//
// CheckInputType
//
// Override CTransformFilter method.
// Part of the Connect process.
// Ensure that we do not get connected to formats that we can't handle.
// We only work for wave 16 bit wave audio, uncompressed.
//
HRESULT CSfPlugIn::CheckInputType(const CMediaType *pmt)
{
    DbgFunc("CheckInputType");
    DbgDumpMediaType(pmt);

    // Reject non-Audio types.
    //
    if (pmt->majortype != MEDIATYPE_Audio)
        return E_INVALIDARG;

    if (pmt->bTemporalCompression)
        return E_INVALIDARG;

    // Reject invalid format blocks
    //
    if (pmt->cbFormat < sizeof(PCMWAVEFORMAT))
        return E_INVALIDARG;

    //  Now that we know it's audio we can dump the contents of the
    //  WAVEFORMATEX type-specific format structure
    //
    LPWAVEFORMATEX pwfx = (LPWAVEFORMATEX)pmt->pbFormat;
    DbgDumpWfx(pwfx);

    // Reject mono/stereo based on NEED_CHANNELS_IN
    //
   #ifdef NEED_CHANNELS_IN
    if (NEED_CHANNELS_IN != pwfx->nChannels)
        return E_INVALIDARG;
   #else
    //
    //  it's doubtful that _most_ plug-ins deal with other than mono
    //  or stereo...
    //
    if ((1 != pwfx->nChannels) &&
        (2 != pwfx->nChannels))
    {
        return E_INVALIDARG;
    }
   #endif

    // Accept uncompressed 16 bit audio
    //
    if ((WAVE_FORMAT_PCM == pwfx->wFormatTag) &&
        (16 == pwfx->wBitsPerSample))
    {
        DbgLog((LOG_TRACE,2,TEXT("CheckInputType - success (16 bit pcm)")));
        return NOERROR;
    }

    // Accept uncompressed FLOAT audio
    //
   #ifdef FLOAT_SAMPLES_SUPPORTED
    if ((WAVE_FORMAT_IEEE_FLOAT == pwfx->wFormatTag) &&
        (32 == pwfx->wBitsPerSample))
    {
        DbgLog((LOG_TRACE,2,TEXT("CheckInputType - success (float)")));
        return NOERROR;
    }
   #endif

    return E_INVALIDARG;
}

//
// return success if the given transform is possible
//
HRESULT CSfPlugIn::CheckTransform (
    const CMediaType *pmtIn,
    const CMediaType *pmtOut)
{
    WAVEFORMATEX *pwfxIn  = (WAVEFORMATEX *)pmtIn->pbFormat;
    WAVEFORMATEX *pwfxOut = (WAVEFORMATEX *)pmtOut->pbFormat;

    DbgFunc("CheckTransform");

    // Reject non-Audio types.
    //
    if (pmtIn->majortype != MEDIATYPE_Audio ||
        pmtOut->majortype != MEDIATYPE_Audio)
        return E_INVALIDARG;

    // Reject invalid format blocks
    //
    if (pmtIn->cbFormat < sizeof(PCMWAVEFORMAT) ||
        pmtOut->cbFormat < sizeof(PCMWAVEFORMAT))
        return E_INVALIDARG;

    // Reject compressed audio
    //
    if (pmtIn->bTemporalCompression ||
        pmtOut->bTemporalCompression)
        return E_INVALIDARG;

    // Reject any format change other than changing # of channels
    //
    if (pwfxIn->wFormatTag     != pwfxOut->wFormatTag     ||
        pwfxIn->wBitsPerSample != pwfxOut->wBitsPerSample ||
        pwfxIn->nSamplesPerSec != pwfxOut->nSamplesPerSec)
        return E_INVALIDARG;

    // check the number of output channels
    //
   #ifdef NEED_CHANNELS_OUT
    if (NEED_CHANNELS_OUT != pwfxOut->nChannels)
        return E_INVALIDARG;
   #else
    if (S_OK != TransformCheckOutputFormat(pwfxIn, pwfxOut))
        return E_INVALIDARG;
   #endif

    DbgLog((LOG_TRACE,2,TEXT("CheckTransform - success")));
    return NOERROR;
}

// override CTransformInputPin so that we return true for CheckMediaType
// if we can do the requested transform, OR if we could do the transform
// provided that the output pin was reconnected to a new media type.
//
// (The base class will fail unless we can do the transform without
//  reconnecting the output pin)
//
HRESULT CSfPlugInInputPin::CheckMediaType (
    const CMediaType * pmt)
{
    // Check the input type
    //
    HRESULT hr = m_pTransformFilter->CheckInputType(pmt);
    if (S_OK != hr)
        return hr;

    // if the output pin is still connected, then we have
    // to check the transform not just the input format
    //
    CSfPlugIn * pFilter = (CSfPlugIn *)m_pTransformFilter;
    CTransformOutputPin * pOutput = pFilter->OutputPin();
    if (pOutput && pOutput->IsConnected())
    {
        const CMediaType * pmtOut = &(pOutput->CurrentMediaType());

        // if we can't do this transform, then assume we would
        // be able to handle the case where the input and the output
        // formats are the same.  check the downstream input pin
        // to see if it will handle the requested format for our input pin.
        //
        // by doing this, we emulate the behavior of the inplace transform
        //
        hr = m_pTransformFilter->CheckTransform (pmt, pmtOut);
        if (S_OK != hr)
        {
            CMediaType mt = *pmt;
            hr = pFilter->SuggestOutputFormat (&mt);
            if (S_OK == hr)
            {
                // ask the downstream input pin if it can connect
                // with this format.
                //
                hr = pOutput->GetConnected()->QueryAccept(&mt);
            }
        }
    }

    return hr;
}

// whenever the input pin is connected, if the output pin is connected
// and we can't do the indicated transform, we have to request a reconnect
// on the output pin.
//
HRESULT CSfPlugIn::CompleteConnect (
    PIN_DIRECTION dir,
    IPin *        pReceivePin)
{
    UNREFERENCED_PARAMETER(pReceivePin);
    ASSERT(m_pInput);
    ASSERT(m_pOutput);

    if (PINDIR_OUTPUT == dir)
       return NOERROR;

    ASSERT(PINDIR_INPUT == dir);

    if (m_pOutput->IsConnected())
    {
        if (S_OK != CheckTransform(&m_pInput->CurrentMediaType(),
                                   &m_pOutput->CurrentMediaType()))
        {
            if (m_pGraph)
                return m_pGraph->Reconnect( m_pOutput );

        return VFW_E_NOT_IN_GRAPH;
        }
    }

    return NOERROR;

}

// helper method for GetMediaType
//
HRESULT CSfPlugIn::SuggestOutputFormat (
    CMediaType * pmt)
{
    LPWAVEFORMATEX pwfx = (LPWAVEFORMATEX)pmt->Format();
    HRESULT        hr = S_OK;

    ASSERT (pmt->majortype == MEDIATYPE_Audio);
    ASSERT ( ! pmt->bTemporalCompression);
    ASSERT (WAVE_FORMAT_PCM == pwfx->wFormatTag || WAVE_FORMAT_IEEE_FLOAT == pwfx->wFormatTag);

    //  fix up damaged or wierded-out waveformats. note that we can do
    //  this because we _know_ we are not dealing with a compressed
    //  format where cbSize can be non-zero
    //
    if (pmt->cbFormat > sizeof(PCMWAVEFORMAT))
    {
        ASSERT(0 == pwfx->cbSize);
        pwfx->cbSize = 0;
    }

    // suggest output type based on input type and
    // the stereo out mode.
    //
   #ifdef NEED_CHANNELS_OUT
    pwfx->nChannels = NEED_CHANNELS_OUT;
   #else
    hr = TransformTweakOutputFormat(pwfx);
   #endif

    // fixup derived parts of wfx
    //
    pwfx->nBlockAlign = pwfx->nChannels * (pwfx->wBitsPerSample / 8);
    pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;

    return hr;
}

// override to suggest OUTPUT pin media types
HRESULT CSfPlugIn::GetMediaType (
    int          iPosition,
    CMediaType * pmt)
{
    DbgLog((LOG_TRACE,2,TEXT("GetMediaType(%d)"),iPosition));

    if ( ! m_pInput->IsConnected())
        return E_UNEXPECTED;

    if (iPosition < 0)
        return E_INVALIDARG;

    if (iPosition > 0)
        return VFW_S_NO_MORE_ITEMS;

    // suggest an output type that is the same as the input type
    //
    *pmt = m_pInput->CurrentMediaType();
    return SuggestOutputFormat (pmt);
}

//
// SetMediaType
//
// Override CTransformFilter method.
// Called when a connection attempt has succeeded. If the output pin
// is being connected and the input pin's media type does not agree then we
// reconnect the input (thus allowing its media type to change,) and vice versa.
//
HRESULT CSfPlugIn::SetMediaType (
    PIN_DIRECTION direction,
    const CMediaType *pmt)
{
    DbgLog((LOG_TRACE,2,TEXT("SetMediaType(%s)"),
           direction == PINDIR_INPUT ? "in" : "out"));

    //
    //  We know that this is a waveformat, because this function is only
    //  called when the connection has succeeded, and we insist on AUDIO
    //  in the connection negotitation code.
    //
    LPWAVEFORMATEX pwfx = (LPWAVEFORMATEX)pmt->Format();
    DbgDumpWfx(pwfx);

    //
    //  Copy the waveformat structure, we may need it later
    //
    //  Note the fixup of nBlockAlign... this is a workaround for a bug
    //  in Sound Forge 4.0a. The nBlockAlign field is not correct if
    //  the number of channels changes... this was really stupid of us.
    //
    WAVEFORMATEX    wfx;
    CopyMemory(&wfx, pwfx, sizeof(PCMWAVEFORMAT));
    wfx.nBlockAlign     = wfx.nChannels * (wfx.wBitsPerSample / 8);
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
    wfx.cbSize          = 0;

    pwfx  = (PINDIR_INPUT == direction) ? &m_wfxIn : &m_wfxOut;
    *pwfx = wfx;

    //
    //  Call the base class to do its thing
    //
    CTransformFilter::SetMediaType(direction, pmt);

    //
    //  Tell the transform about the format change
    //
    TransformSetFormat(pwfx, direction);
    TransformDispatchMessage(direction);

    //  Reconnect where necessary...
    //
    //  Execute this code only if we want to change the input media
    //  type based on the connected output media type. In the normal
    //  case for a DSP effect, we DON'T want to do this.
   #if 0
    if (m_pInput->IsConnected() &&
        m_pOutput->IsConnected())
    {
        FILTER_INFO fInfo;

        QueryFilterInfo (&fInfo);

        if (direction == PINDIR_OUTPUT &&
            *pmt != m_pInput->CurrentMediaType())
        {
            fInfo.pGraph->Reconnect (m_pInput);
        }

        QueryFilterInfoReleaseGraph (fInfo);

        ASSERT(!(direction == PINDIR_INPUT && *pmt != m_pOutput->CurrentMediaType()));
    }
   #endif

    return NOERROR;
}

HRESULT CSfPlugIn::DecideBufferSize (
    IMemAllocator        * pAlloc,
    ALLOCATOR_PROPERTIES * pProp)
{
    HRESULT                 hr;

    DbgFunc("DecideBufferSize");

    // if the input pin has an allocator, get it's
    // properties as a starting point.
    //
    if (m_pInput->IsConnected())
    {
        IMemAllocator * pInAlloc;
        hr = m_pInput->GetAllocator(&pInAlloc);
        if ( ! FAILED(hr))
        {
           pInAlloc->GetProperties(pProp);
           pInAlloc->Release();
        }
    }

    DbgLog((LOG_TRACE,0,TEXT("Default Allocator props %d,%d,%d,%d"),
           pProp->cBuffers, pProp->cbBuffer, pProp->cbAlign, pProp->cbPrefix));

    // ask the transform what effect it will have on buffer sizes
    //
    hr = TransformDecideBufferSize (pAlloc, pProp);

   #ifdef _DEBUG
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE,1,TEXT("pAlloc->SetProperties() FAILED=%.08lXh"), hr));
    }
    else
    {
        ALLOCATOR_PROPERTIES    Actual;

        pAlloc->GetProperties(&Actual);
        DbgLog((LOG_TRACE,0,TEXT("actual Allocator props %d,%d,%d,%d"),
                Actual.cBuffers, Actual.cbBuffer, Actual.cbAlign, Actual.cbPrefix));
    }
   #endif

    return hr;
}


//--------------------------------------------------------------------------;
//
//
//
//
//--------------------------------------------------------------------------;

STDMETHODIMP CAudioTransform::put_ReadFail(BOOL fReadFail)
    {
        CAutoLock foo(&m_Lock);
        m_fReadFail = fReadFail;

        // update the display
        //
        TransformDispatchMessage(IDC_SFPLUGIN_NOTIFY_FORCE_UPDATE);
        
        return NOERROR;
    }

STDMETHODIMP CAudioTransform::put_WriteFail(BOOL fWriteFail)
{
    CAutoLock foo(&m_Lock);
    m_fWriteFail = fWriteFail;

    // update the display
    //
    TransformDispatchMessage(IDC_SFPLUGIN_NOTIFY_FORCE_UPDATE);

    return NOERROR;
}

STDMETHODIMP CAudioTransform::put_NotifyWindow(HWND hwnd, UINT id)
{
    UINT    iInsert = NUMELMS(m_aNotify);
    UINT    ii;

    CAutoLock foo(&m_Lock);
    for (ii = 0; ii < NUMELMS(m_aNotify); ++ii)
    {
        if (hwnd == m_aNotify[ii].hwnd)
        {
            iInsert = ii;
            break;
        }
        else if (NULL == m_aNotify[ii].hwnd)
        {
            iInsert = ii;
        }
    }

    if (iInsert >= NUMELMS(m_aNotify))
    {
        if (0 != id)
        {
            DbgBreak("put_NotifyWindow out of memory!!");
            return E_OUTOFMEMORY;
        }

        return NOERROR;
    }

    // if the id is zero, release the entry
    // otherwise remember the hwnd and id for later notifications
    //
    m_aNotify[iInsert].id   = id;
    m_aNotify[iInsert].hwnd = (0 == id) ? NULL : hwnd;

    return NOERROR;
};


//
//  Used to send messages to UI.  Be Careful!!  This should be used
//  with caution, when control via GETPUT is not possible or relevant
//

HRESULT CAudioTransform::TransformDispatchMessage(UINT uMsg)
{
    CAutoLock foo(&m_Lock);
    for (UINT ii = 0; ii < NUMELMS(m_aNotify); ++ii)
    {
        if (NULL == m_aNotify[ii].hwnd)
            continue;

        if (IsWindow(m_aNotify[ii].hwnd))
        {
            WPARAM wParam = MAKELONG(m_aNotify[ii].id, uMsg);

            //
            //  LOWORD(wParam) -> notify id
            //  HIWORD(wParam) -> message
            //
            PostMessage(m_aNotify[ii].hwnd, WM_COMMAND, wParam, 0);
        }
        else
        {
            DbgBreak("Someone forgot to remove a notify window!");
            m_aNotify[ii].hwnd = NULL;
            m_aNotify[ii].id   = 0;
        }
    }

    return NOERROR;
}


BOOL CAudioTransform::TransformGetProperties(
    LPTRANSFORM_PROPS       pProps)
{
    BOOL                fPropsChanged;

    //
    //  Make a copy of the current properties so we don't lock out
    //  changes while processing a buffer. Remember that your property
    //  pages and processing code are almost always running on separate
    //  threads.
    //
    CAutoLock foo(&m_Lock);

    *pProps = m_prop;

    fPropsChanged = m_fPropsChanged;
    m_fPropsChanged = FALSE;

    return (fPropsChanged);
}


HRESULT CAudioTransform::TransformRead(LPSTREAM pStream)
{
    TRANSFORM_PROPS prop;
    struct _persist_version version;

    CAutoLock foo(&m_Lock);

    HRESULT hr = pStream->Read(&version, sizeof(version), NULL);
    if (FAILED(hr))
        return hr;
    if (version.cb < (sizeof(version) + sizeof(m_prop) ||
        version.guid != IID_ISfPlugInProp))
        return E_FAIL;
    
    hr = pStream->Read(&prop, sizeof(prop), NULL);
    if (SUCCEEDED(hr))
        put_all(&prop);

    // allocate space for the 'large preset' data
    //
    hr = put_LargeArraySize (m_prop.lPresetSize);
    if (FAILED(hr))
       return hr;

    // read the large preset and verify that it is correct
    //
    if (m_prop.lPresetSize > 0)
    {
        ZeroMemory (m_padLargePreset, m_prop.lPresetSize * sizeof(double));

        hr = pStream->Read(m_padLargePreset, m_prop.lPresetSize * sizeof(double), NULL);
        if (FAILED(hr))
            return hr;

        // verify the large preset data
        //
        if ( ! validate_LargeArray())
            return E_FAIL;


    }

    if (SUCCEEDED(hr))
        hr = TransformDispatchMessage(IDC_SFPLUGIN_NOTIFY_FORCE_UPDATE);

    return hr;
};

HRESULT CAudioTransform::TransformWrite(LPSTREAM pStream, BOOL fClearDirty)
{
    CAutoLock foo(&m_Lock);

    struct _persist_version version;
    DWORD dwTemp = (m_prop.lPresetSize * sizeof(double));
           
    version.cb    = sizeof(version) + sizeof(m_prop) + dwTemp;
    version.guid  = IID_ISfPlugInProp;
   
    HRESULT hr = pStream->Write(&version, sizeof(version), NULL);
    if (FAILED(hr))
       return hr;

    hr = pStream->Write(&m_prop, sizeof(m_prop), NULL);
    if (FAILED(hr))
       return hr;

    // Dont write out 0 bytes
    //
    if (0 < m_prop.lPresetSize)
    {
        hr = pStream->Write(m_padLargePreset, (m_prop.lPresetSize * sizeof(double)), NULL);
        if (FAILED(hr))
            return hr;
    }
    
    if ( ! FAILED(hr))
       m_fDirty = !fClearDirty;

    return hr;
};

DWORD CAudioTransform::TransformPersistSize(void)
{
    return sizeof(struct _persist_version) + sizeof(m_prop) + (m_prop.lPresetSize * sizeof(double));
};


//--------------------------------------------------------------------------;
//
//  IStaticFilterPresets
//
//
//--------------------------------------------------------------------------;

//  initialize table of presets here
//
static TRANSFORM_PRESETS g_aPresets[] =
{
    PRESET_INITIALIZER
};

STDMETHODIMP CSfPlugIn::GetPresetCount(
    long * piCount)
{
    *piCount = NUMELMS(g_aPresets);
    return NOERROR;
}

STDMETHODIMP CSfPlugIn::GetPresetName(
    long      index,
    LPOLESTR polestr,
    DWORD    cch)
{
    if (index < 0 || index >= NUMELMS(g_aPresets))
        return E_INVALIDARG;

    MultiByteToWideChar(CP_ACP, 0, g_aPresets[index].szName, -1, polestr, cch);
    return NOERROR;
}

STDMETHODIMP CSfPlugIn::UsePreset(
    long index)
{
    if (index < 0 || index >= NUMELMS(g_aPresets))
        return E_INVALIDARG;

    HRESULT hr = put_all(&g_aPresets[index].prop);
    put_ReadFail(FAILED(hr));
    return hr;
}


//--------------------------------------------------------------------------;
//
//  ISpecifyPropertyPages
//
//
//--------------------------------------------------------------------------;

STDMETHODIMP CSfPlugIn::GetPages(
    CAUUID * pPages)
{
    pPages->cElems = 1;
   #ifdef MULTIPLE_PROPERTY_PAGES
    //pPages->cElems = 2;
    pPages->cElems = 3;
   #endif

    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID) * pPages->cElems);
    if ( ! pPages->pElems)
        return E_OUTOFMEMORY;

    (pPages->pElems)[0] = CLSID_SfPlugInPropPage;
   #ifdef MULTIPLE_PROPERTY_PAGES
    (pPages->pElems)[1] = CLSID_SfPlugInPropPage2;
    (pPages->pElems)[2] = CLSID_SfPlugInPropPage3;
   #endif
    return NOERROR;

}


//--------------------------------------------------------------------------;
//
//  Exported entry points for registration and unregistration (in this case
//  they only call through to default implmentations).
//
//--------------------------------------------------------------------------;

//
//  sigh... you also need to specify __stdcall on the C/C++ page of
//  the Build Settings dialog in VC++ to get proper linking with
//  the AMovieDllxxxxServer functions.
//
STDAPI DllRegisterServer(void)
{
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer(void)
{
    return AMovieDllRegisterServer2(FALSE);
}


//
//  This little leap-frog is so building in VC++ does not require
//  specifying DllEntryPoint on the Link page of the Build Settings
//  dialog. This is very easy to forget, and things don't work
//  if you forget... this is the most bullet proof.
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
extern "C" BOOL WINAPI DllMain
(
    HINSTANCE               hinst,
    DWORD                   dwReason,
    LPVOID                  pReserved
)
{
    return DllEntryPoint(hinst, dwReason, pReserved);
}

//
//  "unreferenced inline function has been removed"
//
#pragma warning(disable: 4514)
