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
#include <streams.h>              // ActiveMovie, includes windows
#include <initguid.h>
#include <olectl.h>

#include <sfiface.h>             // WAVE_FORMAT_IEEE_FLOAT define
#include "sfinvert.h"            // CAudioTransform class

#ifndef WTEXT
#define _WTEXT(str) L##str
#define WTEXT(str) _WTEXT(str)
#endif

class CSfPlugIn
    // Inherited classes
    : public CTransInPlaceFilter       // Main ActiveMovie interfaces

    , public CAudioTransform           // this class does the actual
                                       // transform.  you should
                                       // NOT need to mess with any
                                       // of the code in the CSfPlugin class

    , public IPersistStream            // Implements IPersistStream
                                       // to alow saving of properties
                                       // in a saved graph.
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
    HRESULT StartStreaming(void);
    HRESULT StopStreaming(void);

    // Overrides a CTransformInPlace function.  Called as part of connecting.
    HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);

    // Basic COM - used here to reveal our property interface.
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    // ---- IPersistStream ------
    STDMETHODIMP IsDirty() {
       return FALSE;
       };
    STDMETHODIMP Load(LPSTREAM pStm) {
       return NOERROR;
       };
    STDMETHODIMP Save(LPSTREAM pStm, BOOL fClearDirty) {
       return NOERROR;
       };
    STDMETHODIMP GetSizeMax (ULARGE_INTEGER * pcbSize) {
       pcbSize->QuadPart = 0;
       return NOERROR;
       };
    STDMETHODIMP GetClassID (CLSID * pClsid);

    // setup helper
    LPAMOVIESETUP_FILTER GetSetupData();

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
    static int          m_nInstanceCount;   // total instances
    int                 m_nThisInstance;

    WAVEFORMATEX        m_wfx;              // current wave format (not including compressor specific data).
};

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------


// Put out the name of a function and instance on the debugger.
// Invoke this at the start of functions to allow a trace.
//
#define DbgFunc(a) DbgLog(( LOG_TRACE                       \
                          , 2                               \
                          , TEXT("CSfPlugIn(%d)::%s")       \
                          , m_nThisInstance                 \
                          , TEXT(a)                         \
                         ));

 // Self-registration data structures
   //
   static AMOVIESETUP_MEDIATYPE g_sudPinTypes = {
         &MEDIATYPE_Audio         // clsMajorType
       , &MEDIASUBTYPE_NULL       // clsMinorType
   };

   static AMOVIESETUP_PIN g_psudPins[] = {
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

   static AMOVIESETUP_FILTER g_sudSfPlugIn = {
         &CLSID_SfPlugIn      // class id
       , WTEXT(SFPLUGIN_NAME) // strName
       , MERIT_DO_NOT_USE     // dwMerit
       , 2                    // nPins
       , g_psudPins             // lpPin
   };


// Needed for the CreateInstance mechanism
CFactoryTemplate g_Templates[]=
{
    {
        WTEXT(SFPLUGIN_NAME),
        &CLSID_SfPlugIn,
        CSfPlugIn::CreateInstance
       , NULL
       , &g_sudSfPlugIn
    },
};

int g_cTemplates = NUMELMS(g_Templates);

// initialise the static instance count.
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
    if (riid == IID_IPersistStream)
        return GetInterface((IPersistStream *) this, ppv);
    else
        return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
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
      UINT    cs)
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
      UINT    cs)
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
    UINT cb = pSample->GetActualDataLength();
    pSample->GetPointer((BYTE **)&psBuffer);

   #ifdef FLOAT_SAMPLES_SUPPORTED
    ASSERT((16 == m_wfx.wBitsPerSample && WAVE_FORMAT_PCM == m_wfx.wFormatTag) ||
           (32 == m_wfx.wBitsPerSample && WAVE_FORMAT_IEEE_FLOAT == m_wfx.wFormatTag));
   #else
    ASSERT(16 == m_wfx.wBitsPerSample && WAVE_FORMAT_PCM == m_wfx.wFormatTag);
   #endif

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
            hr = Transform16bit(psBuffer, ccBuffer);
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
                hr = TransformFloat(psTemp, ccBuffer);
                ConvertFloatTo16(psBuffer, psTemp, csBuffer);
                HeapFree(GetProcessHeap(), 0, psTemp);
            }
           #endif
            break;

       #ifdef FLOAT_SAMPLES_SUPPORTED
        case WAVE_FORMAT_IEEE_FLOAT:
            hr = TransformFloat((LPFLOAT)psBuffer, ccBuffer);
            break;
       #endif
    }

    return hr;
}

// called on a stop -> pause transition
//
HRESULT CSfPlugIn::StartStreaming(void)
{
    return TransformBegin (&m_wfx);
}

// called on a pause or run -> stop transition
//
HRESULT CSfPlugIn::StopStreaming(void)
{
    return TransformEnd ();
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
