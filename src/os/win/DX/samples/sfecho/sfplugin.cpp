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

#include "sfecho.h"              // ISfPlugInProp (properties) interface
#include "sfppage.h"             // CSfPlugInPropPage

#ifndef _WTEXT
#define _WTEXT(str) L##str
#define WTEXT(str) _WTEXT(str)
#endif

//
//
//
class CSfPlugIn
    // Inherited classes
    : public CTransInPlaceFilter       // Main ActiveMovie interfaces

    , public CAudioTransform           // this class does the actual
                                       // transform.  you should
                                       // NOT need to mess with any
                                       // of the code in the CSfPlugin class

    , public ISpecifyPropertyPages     // so that someone can find out what our
                                       // property page interface GUID(s) are

    , public IPersistStream            // Implements IPersistStream
                                       // to alow saving of properties
                                       // in a saved graph.

    , public IStaticFilterPreset       // allows filter owner to get preset
                                       // names and use presets.
#ifdef TRACK_INTERFACE_SUPPORTED
    , public ISfAudioTransformFloat    // simplified buffer processing interface                                       
#endif
{

public:

    static CUnknown *CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    DECLARE_IUNKNOWN;

    //
    // --- CTransInPlaceFilter Overrides --
    //

    // check if an input media type can be supported
    HRESULT CheckInputType(const CMediaType *mtIn);

    // basic flow-of control methods
    //
    HRESULT Receive(IMediaSample *pSample);
   #ifdef NEED_END_OF_STREAM
    HRESULT EndOfStream(void);
   #endif
    HRESULT StartStreaming(void);
    HRESULT StopStreaming(void);

    // Overrides a CTransformInPlace function.  Called as part of connecting.
    HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);

    // Basic COM - used here to reveal our property interface.
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    // --- ISpecifyPropertyPages ---
    STDMETHODIMP GetPages(CAUUID * pPages);

    // ---- IPersistStream ------
    STDMETHODIMP IsDirty() {
       return TransformIsDirty();
       };
    STDMETHODIMP Load(LPSTREAM pStm) {
       return TransformRead(pStm);
       };
    STDMETHODIMP Save(LPSTREAM pStm, BOOL fClearDirty) {
       return TransformWrite(pStm, fClearDirty);
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

#ifdef TRACK_INTERFACE_SUPPORTED
    // ---- ISfAudioTransformFloat ----
    STDMETHODIMP FloatStart (UINT cChannels, UINT nSamplesPerSec);
    STDMETHODIMP FloatTransform (float* pfBuffer, LONG ccBuffer, BOOL fSilence);
    STDMETHODIMP FloatEndOfStream (float * pfBuffer, long * pccBuffer);
    STDMETHODIMP FloatStop (void);
    STDMETHODIMP FloatHaveParamsChanged (void);
    STDMETHODIMP FloatCookParams (BOOL fDiscontinuity, double dTempo);
#endif

private:

    // Constructor
    CSfPlugIn(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);
    // Destructor
    ~CSfPlugIn();

    // Overrides the PURE virtual Transform of CTransInPlaceFilter base class
    // This is where the "real work" is done.
    HRESULT Transform(IMediaSample *pSample);

    // If there are multiple instances of this filter active, it's
    // useful for debug messages etc. to know which one this is.
    // This variable has no other purpose.
    static int          m_nInstanceCount;     // total instances
    int                 m_nThisInstance;

    WAVEFORMATEX        m_wfx;                // current wave format (not including compressor specific data).
   #ifdef NEED_END_OF_STREAM
    BOOL                m_fInEOS;             // true while in EndOfStream
   #endif

    BOOL                m_fGotABuffer;        // TRUE if we got at least 1 buffer
};

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------

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
       , &g_sudPinTypes      // lpTypes
       }
     , { L"Output"           // strName
       , FALSE               // bRendered
       , TRUE                // bOutput
       , FALSE               // bZero
       , FALSE               // bMany
       , &CLSID_NULL         // clsConnectsToFilter
       , L"Input"            // strConnectsToPin
       , 1                   // nTypes
       , &g_sudPinTypes      // lpTypes
       }
   };

   static AMOVIESETUP_FILTER g_sudSfPlugIn =
   {
         &CLSID_SfPlugIn      // class id
       , WTEXT(SFPLUGIN_NAME) // strName
       , MERIT_DO_NOT_USE     // dwMerit
       , 2                    // nPins
       , g_psudPins           // lpPin
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
    }
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
    : CTransInPlaceFilter (tszName, punk, CLSID_SfPlugIn, phr)
    , CAudioTransform(punk, phr)
{
    // Useful for debug, no other purpose
    m_nThisInstance = ++m_nInstanceCount;
    DbgFunc("CSfPlugIn");

    // default filter properties to preset 0
    //
    UsePreset (0);
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
#ifdef TRACK_INTERFACE_SUPPORTED
    else if (riid == IID_ISfAudioTransformFloat)
        return GetInterface((ISfAudioTransformFloat *) this, ppv);
#endif
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

//
// Transform
//
// Override CTransformFilter method.
// Convert the input ActiveMovie sample into the output ActiveMovie sample.
// when processing in end of stream, this function returns fails to
// indicate that there is no more end of stream data.
//
HRESULT CSfPlugIn::Transform(IMediaSample *pSample)
{
    DbgFunc("Transform");

    // Get the details of the data (address, length)
    //
    LPSHORT psBuffer;
    DWORD cb = pSample->GetActualDataLength();
    pSample->GetPointer((BYTE **)&psBuffer);
    m_fGotABuffer = TRUE;

   #ifdef FLOAT_SAMPLES_SUPPORTED
    ASSERT((16 == m_wfx.wBitsPerSample && WAVE_FORMAT_PCM == m_wfx.wFormatTag) ||
           (32 == m_wfx.wBitsPerSample && WAVE_FORMAT_IEEE_FLOAT == m_wfx.wFormatTag));
   #else
    ASSERT(16 == m_wfx.wBitsPerSample && WAVE_FORMAT_PCM == m_wfx.wFormatTag);
   #endif

    TRANSFORM_STATE nState;
   #ifdef NEED_END_OF_STREAM
    if (m_fInEOS)
    {
        nState = TRANSFORM_STATE_ENDOFSTREAM;
        cb = pSample->GetSize();

        //
        //  !!! you may not need this if you don't care if the EOS buffers
        //      are zero filled--kill if necessary...
        //
        ZeroMemory(psBuffer, cb);
    }
    else
   #endif
    nState = (S_OK == pSample->IsDiscontinuity()) ?
                    TRANSFORM_STATE_FIRSTBUFFER : TRANSFORM_STATE_NORMAL;

    HRESULT     hr;
    DWORD       ccBuffer;

    //
    //  convert count in bytes (cb) to count in cells (cc)
    //
    ccBuffer = cb / m_wfx.nBlockAlign;

    switch (m_wfx.wFormatTag)
    {
        case WAVE_FORMAT_PCM:
           #if !defined FLOAT_SAMPLES_ONLY
            hr = Transform16bit(nState, psBuffer, &ccBuffer);
           #else
            //
            //  if we have a transform for float samples only, but we
            //  connected as 16 bit PCM, then convert 16 bit to float
            //  before doing the transform and then back after
            //
            //  !!! this temp buffer allocation should NOT be done here !!!
            //      this is a serious performance hit!!!
            //
            hr = E_OUTOFMEMORY;
            DWORD   csBuffer = ccBuffer * m_wfx.nChannels;
            DWORD   cbTemp   = csBuffer * sizeof(float);
            LPFLOAT psTemp   = (LPFLOAT)HeapAlloc(GetProcessHeap(), 0, cbTemp);
            if (NULL != psTemp)
            {
                Convert16ToFloat(psTemp, psBuffer, csBuffer);
                hr = TransformFloat(nState, psTemp, &ccBuffer, FALSE);
                ConvertFloatTo16(psBuffer, psTemp, csBuffer);
                HeapFree(GetProcessHeap(), 0, psTemp);
            }
           #endif
            break;

       #ifdef FLOAT_SAMPLES_SUPPORTED
        case WAVE_FORMAT_IEEE_FLOAT:
            hr = TransformFloat(nState, (LPFLOAT)psBuffer, &ccBuffer, FALSE);
            break;
       #endif
    }

    //
    //  convert count in cells (cc) to count in bytes (cb)
    //
    cb = ccBuffer * m_wfx.nBlockAlign;
    pSample->SetActualDataLength(cb);

    return hr;
}

HRESULT CSfPlugIn::Receive(IMediaSample *pSample)
{
    HRESULT hr;

    // do the transform (and time it if PERF is defined)
    //
    MSR_START(m_idTransInPlace);
    hr = Transform(pSample);
    MSR_STOP(m_idTransInPlace);

    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 1, TEXT("Error from Transform")));
        return hr;
    }

    // the Transform() function can return S_FALSE to indicate that the
    // sample should not be delivered; we only deliver the sample if it's
    // really S_OK (same as NOERROR, of course.)
    //
    if (NOERROR == hr)
        hr = OutputPin()->Deliver (pSample);
    else
    {
        //  But it would be an error to return this private hack
        //  to the caller ...
        if (S_FALSE == hr)
            hr = NOERROR;
    }

    return hr;
}

#ifdef NEED_END_OF_STREAM
//
// override EndOfStream so that we can put out the tail
//
HRESULT CSfPlugIn::EndOfStream(void)
{
    HRESULT hr, hrT;
    BOOL    fDone;

    if ( ! OutputPin() || ! m_fGotABuffer)
       return NOERROR;

    hr = NOERROR;
    fDone = FALSE;
    m_fInEOS = TRUE;
    for (;;)
    {
        IMediaSample * pSample;

        // this may block for an indeterminate amount of time
        //
        hr = OutputPin()->GetDeliveryBuffer (&pSample, NULL, NULL, 0);
        if (FAILED(hr))
           return hr;

        // give the transform function an empty buffer so that
        // it can generate a tail.
        //
        pSample->SetActualDataLength (0);

        MSR_START(m_idTransInPlace);
        hr = Transform (pSample);
        MSR_STOP(m_idTransInPlace);

        if (FAILED(hr))
           break;

        // if transform returns S_FALSE, there is no more tail.
        //
        if (S_FALSE == hr)
           fDone = TRUE;

        hr = OutputPin()->Deliver(pSample);
        pSample->Release();

        if (FAILED(hr) || fDone)
           break;
    }
    m_fInEOS = FALSE;
    m_fGotABuffer = FALSE;

    // now tell the downstream filter about end of stream
    //
    hrT = OutputPin()->DeliverEndOfStream();
    if ( ! FAILED(hr))
       hr = hrT;

    return hr;
}
#endif


HRESULT CSfPlugIn::StartStreaming(void)
{
    DbgFunc("StartStreaming");
   #ifdef NEED_END_OF_STREAM
    m_fInEOS = FALSE;
   #endif
    m_fGotABuffer = FALSE;
    return TransformBegin(&m_wfx);
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
        DbgLog((LOG_TRACE,2,TEXT("CheckInputType - success (32 bit float)")));
        return NOERROR;
    }
   #endif

    return E_INVALIDARG;
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

    pwfx  = &m_wfx;
    *pwfx = wfx;

    //
    //  Call the base class to do its thing
    //
    CTransformFilter::SetMediaType(direction, pmt);

    //
    //  Tell the transform about the format change
    //
    TransformSetFormat(pwfx);
    TransformDispatchMessage(direction);

    return NOERROR;
}

#ifdef TRACK_INTERFACE_SUPPORTED
//--------------------------------------------------------------------------;
//
// ISfAudioTransformFloat Implementation
//
//--------------------------------------------------------------------------;

STDMETHODIMP CSfPlugIn::FloatStart (
    UINT cChannels,
    UINT nSamplesPerSec)
{
    WAVEFORMATEX wfx,*pwfx;

    wfx.wFormatTag      = WAVE_FORMAT_IEEE_FLOAT;
    wfx.wBitsPerSample  = sizeof(float) * 8;
    wfx.nChannels       = cChannels;
    wfx.nSamplesPerSec  = nSamplesPerSec;
    wfx.nBlockAlign     = wfx.nChannels * (wfx.wBitsPerSample / 8);
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
    wfx.cbSize          = 0;

    pwfx  = &m_wfx;
    *pwfx = wfx;

    TransformSetFormat(&wfx);

    return StartStreaming ();
}

STDMETHODIMP CSfPlugIn::FloatTransform (
    float * pfBuffer,
    LONG    ccBuffer,
    BOOL    fSilence)
{
    m_fGotABuffer = TRUE;
    DWORD  ccPass = ccBuffer;
    return TransformFloat(TRANSFORM_STATE_NORMAL, pfBuffer, &ccPass, fSilence);
}

STDMETHODIMP CSfPlugIn::FloatEndOfStream (
    float * pfBuffer,
    LONG  * pccBuffer)
{
    HRESULT hr = S_FALSE;
    DWORD   cc = 0;

   #ifdef NEED_END_OF_STREAM
    if (m_fGotABuffer)
        {
        cc = *pccBuffer;
        hr = TransformFloat(TRANSFORM_STATE_ENDOFSTREAM, pfBuffer, &cc, FALSE);
        }
   #endif

    *pccBuffer = cc;
    return hr;
}

STDMETHODIMP CSfPlugIn::FloatStop (void)
{
   return StopStreaming();
}

STDMETHODIMP CSfPlugIn::FloatHaveParamsChanged (void)
{
   return Get_m_fPropsChanged() ? S_OK : S_FALSE;
}

STDMETHODIMP CSfPlugIn::FloatCookParams (
    BOOL fDiscontinuity,
    double dTempo)
{
    TRANSFORM_PROPS props;
    BOOL fPropsChanged = TransformGetProperties(&props);

    if (fPropsChanged || fDiscontinuity)
    {
        TRANSFORM_STATE eState = TRANSFORM_STATE_NORMAL;
        if (fDiscontinuity)
           eState = TRANSFORM_STATE_FIRSTBUFFER;

        return TransformCookProperties (&props, eState);
    }

    return S_FALSE;
}
#endif // TRACK_INTERFACE_SUPPORTED

//--------------------------------------------------------------------------;
//
//
//
//
//--------------------------------------------------------------------------;

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
    HRESULT hr = pStream->Read(&version, sizeof(version), NULL);
    if (FAILED(hr))
        return hr;
    if (version.cb != (sizeof(version) + sizeof(m_prop)) ||
        version.guid != IID_ISfPlugInProp)
        return E_FAIL;
    hr = pStream->Read(&prop, sizeof(prop), NULL);
    if (SUCCEEDED(hr))
        hr = put_all(&prop);

    if (SUCCEEDED(hr))
        hr = TransformDispatchMessage(IDC_SFPLUGIN_NOTIFY_FORCE_UPDATE);

    return hr;
};

HRESULT CAudioTransform::TransformWrite(LPSTREAM pStream, BOOL fClearDirty)
{
    struct _persist_version version;
    version.cb    = sizeof(version) + sizeof(m_prop);
    version.guid  = IID_ISfPlugInProp;
    HRESULT hr = pStream->Write(&version, sizeof(version), NULL);
    if (FAILED(hr))
       return hr;
    hr = pStream->Write(&m_prop, sizeof(m_prop), NULL);
    if ( ! FAILED(hr))
       m_fDirty = !fClearDirty;
    return hr;
};

DWORD CAudioTransform::TransformPersistSize(void)
{
    return sizeof(struct _persist_version) + sizeof(m_prop);
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

    return put_all(&g_aPresets[index].prop);
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
    pPages->cElems = 2;
   #endif

    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID) * pPages->cElems);
    if ( ! pPages->pElems)
        return E_OUTOFMEMORY;

    (pPages->pElems)[0] = CLSID_SfPlugInPropPage;
   #ifdef MULTIPLE_PROPERTY_PAGES
    (pPages->pElems)[1] = CLSID_SfPlugInPropPage2;
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
