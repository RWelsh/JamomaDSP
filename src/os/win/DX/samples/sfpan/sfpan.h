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
//  **********************************************************************
//
//     IF YOU ARE USING THIS CODE AS A STARTING POINT FOR YOUR PLUG-IN
//     THEN MAKE CERTAIN YOU CHANGE ALL GUIDS USED BY THIS EXAMPLE TO
//     UNIQUE VALUES!!! 
//
//  **********************************************************************
//
//  Use GUIDGEN.EXE to generate new GUIDs for these identifiers:
//
//      CLSID_SfPlugIn
//      CLSID_SfPlugInPropPage
//      CLSID_SfPlugInPropPage2
//      IID_ISfPlugInProp
//
//==========================================================================;

#ifndef __SFPLUGIN__
#define __SFPLUGIN__

#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG
#endif

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;

//
//  Replace this with the name that should appear in the DirectX
//  compatible application (such as Sound Forge). This name is also
//  used in the registry.
//
#define SFPLUGIN_NAME           "Sonic Foundry PIDK Pan"

//
//  Help file information
//
#define SFPLUGIN_HELP_FILE      "SFPAN.HLP"
#define SFPLUGIN_HELP_CONTEXT   IDH_CONTENTS


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;

//
//  Define this if your plug-in is going to support audio data in the
//  WAVE_FORMAT_IEEE_FLOAT format. This format is highly recommended by
//  Sonic Foundry for supporting 24-bit precision. See the explanation in
//  the PIDK manual.
//
#define FLOAT_SAMPLES_SUPPORTED

//
//  Define this if you wish to write a Transform function that deals only
//  with floats. This can make your code much easier to maintain. By
//  defining FLOAT_SAMPLES_ONLY, SfPlugIn will convert buffers of 16-bit
//  PCM into float before calling your transform and convert the result
//  back after. This _may_ hurt performance when working in an environment
//  (i.e. NOT Sonic Foundry applications) that does not support passing
//  floats to DirectX plug-ins.
//
//#define FLOAT_SAMPLES_ONLY

//
//  Define this if you want to be notified in the TransformXX functions of
//  the end of stream (so that you can generate more output)
//
//#define NEED_END_OF_STREAM

//
//  Define this to force input format to be mono or stereo if not defined,
//  then the plug-in may connect with either (note that the number of
//  channels will NOT change in mid-stream)
//
//#define NEED_CHANNELS_IN    (1)     // force mono/stereo in

//
//  Define this to force the output format to be either mono or stereo
//  NOTE: by default this example allows the output format to change
//  as the result of user interaction with the property page, and
//  demonstrates how to deal correctly with format changes while streaming
//  defining this will disable that behavior.
//
#define NEED_CHANNELS_OUT   (2)     // force mono/stereo out

//
//  Define this to get 2 or more property pages. more than one property page
//  is easily extrapolated from the code provided.
//
//#define MULTIPLE_PROPERTY_PAGES

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;

#ifndef RC_INVOKED

//
//  Sanity checks for the most common mistakes when NOT using the standard
//  makefiles. For example, when building in VC++ 4.x you need to set up
//  a few things beyond using the defaults.
//
//  You *MUST* do the following in VC++ 4.x to build properly:
//  -   Add the .DEF file to the build (otherwise, a couple of
//      necessary functions will not be exported properly!)
//  -   Define both INC_OLE2 and STRICT for the C/C++ preprocessor
//  -   Select Multi-threaded Using DLL for Code Generation
//  -   Set the Calling Convention to __stdcall (very important!)
//  -   Add STRMBASE.LIB and VERSION.LIB to the link libraries
//  -   Unless DllMain is defined, you must also set the entry point
//      on the link page to DllEntryPoint
//
#if defined(_DEBUG) && !defined(DEBUG)
#error ("*** DEBUG needs to be defined for the base classes and version info")
#endif
#ifndef STRICT
#error ("*** STRICT needs to be defined for using the base classes")
#endif
#if !defined(WIN32) || !defined(_WIN32)
#error("*** WIN32 and _WIN32 should be defined")
#endif
#ifndef INC_OLE2
#error("*** INC_OLE2 needs to be defined for ActiveMovie")
#endif
#ifndef _MT
#error("*** _MT needs to be defined for multi-threading")
#endif
#ifndef _DLL
#error("*** _DLL needs to be defined for building plug-ins")
#endif

//
//  Eliminate two expected level 4 warnings from the Microsoft compiler.
//  The class does not have an assignment or copy operator, and so cannot
//  be passed by value. This is normal.
//
#pragma warning(disable: 4511 4512)

//
//  guid for the 'filter' (plug-in) object
//
//  {D616F3B0-D622-11CE-AAC5-0020AF0B99A3}
//
DEFINE_GUID(CLSID_SfPlugIn,
            0xD616F3B0, 0xD622, 0x11CE, 0xAA, 0xC5,
            0x00, 0x20, 0xAF, 0x0B, 0x99, 0xA3);

//
//  GUID for the plug-in's first property page
//
//  {D616F3B1-D622-11CE-AAC5-0020AF0B99A3}
//
DEFINE_GUID(CLSID_SfPlugInPropPage,
            0xD616F3B1, 0xD622, 0x11CE, 0xAA, 0xC5,
            0x00, 0x20, 0xAF, 0x0B, 0x99, 0xA3);

#ifndef MULTIPLE_PROPERTY_PAGES
    #define NUMBER_OF_PROPERTY_PAGES 1
#else
    #define NUMBER_OF_PROPERTY_PAGES 2

    //
    //  GUID for the plug-in's second property page (if your plug-in needs
    //  multiple property pages)
    //
    //  {D616F3B2-D622-11CE-AAC5-0020AF0B99A3}
    //
    DEFINE_GUID(CLSID_SfPlugInPropPage2,
                0xD616F3B2, 0xD622, 0x11CE, 0xAA, 0xC5,
                0x00, 0x20, 0xAF, 0x0B, 0x99, 0xA3);
#endif

//
//  GUID for the plug-in's property manipulation interface. This MUST exist
//  if your plug-in has property pages. It is recommended, but not required,
//  to make this interface public (i.e. document it for developers).
//
//  {D616F3BF-D622-11CE-AAC5-0020AF0B99A3}
//
DEFINE_GUID(IID_ISfPlugInProp,
            0xD616F3BF, 0xD622, 0x11CE, 0xAA, 0xC5,
            0x00, 0x20, 0xAF, 0x0B, 0x99, 0xA3);


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;

//
//  Define pointers to our two types of samples: 16-bit PCM and 32-bit IEEE
//  float.
//
#ifndef _LPSHORT_DEFINED
#define _LPSHORT_DEFINED
typedef short *LPSHORT;
#endif

#ifndef _LPFLOAT_DEFINED
#define _LPFLOAT_DEFINED
typedef float *LPFLOAT;
#endif

#ifndef NUMELMS
#define NUMELMS(aa)     (sizeof(aa)/sizeof((aa)[0]))
#endif

#ifndef BOUND
#define BOUND(x, l, h)  (((x) < (l)) ? (l) : ((x) > (h) ? (h) : (x)))
#endif

#ifndef IS_WITHIN
#define IS_WITHIN(x, start, end)    (((x) >= (start)) && ((x) <= (end)))
#endif

#ifndef IS_OUTSIDE
#define IS_OUTSIDE(x, start, end)   (((x) < (start)) || ((x) > (end)))
#endif

//
//  This macro will bound an 32-bit integer into the valid range for 16-bit
//  PCM samples
//
//  Critics of bounding, here are some things to think about:
//
//  1.  Try profiling and work on the areas that really burn the cycles.
//      Old beliefs that a branch (of this nature) is too expensive needs
//      reevaluation. We're not running on 386's anymore.
//  2.  You obviously don't value your ears or your monitors--let alone
//      the same for your customers
//  3.  A single wrapped sample almost always renders the audio useless--
//      a bounded sample is always more useable than a wrapped sample
//  4.  More often than not, a bounded sample 'here and there' is entirely
//      acceptable
//  5.  Properly written applications can report 'digital clipping' on
//      bounded material with very little overhead--wrapped samples are
//      difficult (expensive) to locate
//  6.  Mastering houses constantly complain about receiving material with
//      wrapped samples in it--bounded material can easily be normalized
//      and/or compressed if necessary
//  7.  Care should be taken (for performance and precision) to only bound
//      when absolutely necessary. That is, always use a bigger (32-bit?)
//      accumulator and bound only at the last minute.
//  8.  And the list can go on...
//
//  Note that bounding is far less of an issue when working with the
//  WAVE_FORMAT_IEEE_FLOAT format. Bounding need only be done once prior
//  to playback of the sample rather than every time you modify a sample.
//  That is, a transform plug-in should NEVER bound a float sample!
//
#define BOUND16I(x) (int)(((long)(x) < (-32768L)) ? (-32768) :  \
                        ((long)(x) > (32767L) ? (32767) : (long)(x)))

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;

//
//  This structure contains all of the variables that need to be exchanged
//  between the plug-in and it's property page(s). Also, anything that
//  you want to save in a preset should be here. The code provided for
//  dealing with presets just writes this structure as the preset data
//  (plus a small header for version maintenance).
//
typedef struct _transform_props
{
    long            lMode;
    long            lPanLR;
    long            lOutGain;

} TRANSFORM_PROPS, *LPTRANSFORM_PROPS;

#define SFPAN_MODE_PRESERVE_SEPARATION  0
#define SFPAN_MODE_MIX_BEFORE_PAN       1

#define SFPAN_FULL_LEFT                 -100
#define SFPAN_FULL_RIGHT                100

#define SFPAN_OUT_GAIN_MIN              -960
#define SFPAN_OUT_GAIN_MAX              200


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;

//
//  Give names and initial values to the presets (predefined values for
//  TRANSFORM_PROPS) here. At least one initializer is required. Note that
//  the first one is always assumed to be the 'default' or 'untitled'
//  preset. It is used when no other preset is specified.
//
//  The first field of each line is the preset name, and the second field
//  (in braces) is an instance of the TRANSFORM_PROPS structure.
//
typedef struct _transform_presets
{
    TCHAR               szName[64];
    TRANSFORM_PROPS     prop;

} TRANSFORM_PRESETS, *LPTRANSFORM_PRESETS;


#define PRESET_INITIALIZER \
    {"(Default)",                               {SFPAN_MODE_PRESERVE_SEPARATION, 0, 0}},    \
    {"25 % left (preserve stereo separation)",  {SFPAN_MODE_PRESERVE_SEPARATION, -25, 0}},  \
    {"25 % right (preserve stereo separation)", {SFPAN_MODE_PRESERVE_SEPARATION, 25, 0}},   \
    {"25 % left (mix before panning)",          {SFPAN_MODE_MIX_BEFORE_PAN, -25, -30}},     \
    {"25 % right (mix before panning)",         {SFPAN_MODE_MIX_BEFORE_PAN, 25, -30}},


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;

//
//  This interface is private, and exists only to allow properties to be
//  communicated between the property page and the filter
//
DECLARE_INTERFACE_(ISfPlugInProp, IUnknown)
{
    //
    //  Wholesale get/put property methods. This is the most efficient
    //  method for getting and setting all properties.
    //
    STDMETHOD(get_all) (THIS_ LPTRANSFORM_PROPS pProps) PURE;
    STDMETHOD(put_all) (THIS_ LPTRANSFORM_PROPS pProps) PURE;

    //
    //  Query filter for input/output WFX
    //
    STDMETHOD(get_WaveFormat)(THIS_ WAVEFORMATEX *pwfx, PIN_DIRECTION  dir) PURE;

    //
    //  Tell the filter to notify the window (with WM_COMMAND) whenever
    //  a new input or output waveformat is set. if id is zero, the notify
    //  handle is removed.
    //
    STDMETHOD(put_NotifyWindow)(THIS_ HWND hwnd, UINT id) PURE;

    //
    //  This macro makes it easy to declare get/set functions for individual
    //  members of TRANSFORM_PROPS
    //
    #define GETPUT_SFPROP(tag,type)                     \
        STDMETHOD(get_##tag)(THIS_ type * p##tag) PURE; \
        STDMETHOD(put_##tag)(THIS_ type x##tag) PURE;

    //
    //  Declare one of these for each member of the TRANSFORM_PROPS
    //  structure.
    //
    GETPUT_SFPROP(Mode,        long)
    GETPUT_SFPROP(PanLR,       long)
    GETPUT_SFPROP(OutGain,     long)

    #undef GETPUT_SFPROP
};


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;

// the TransformXX functions take this as the first praram so that
//
typedef enum
{
    TRANSFORM_STATE_FIRSTBUFFER,    // the first buffer of a stream
    TRANSFORM_STATE_NORMAL,         // non-first buffers in a stream
    TRANSFORM_STATE_ENDOFSTREAM     // cleanup (tail) of a stream

} TRANSFORM_STATE;


//
//  Class used to wrap an audio transform and isolate a programmer from the
//  details of ActiveMovie.
//
class CAudioTransform : public ISfPlugInProp
{
private:

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;

    //
    //  All members in this group require m_Lock unless you are absolutely
    //  certain of when/how the member is being accessed.
    //
    //  Use the following line to take the critical section:
    //
    //      CAutoLock foo(&m_Lock);
    //
    //  The lock is held until the destructor is called (the matching
    //  closing curly brace is executed).
    //
    CCritSec        m_Lock;      // serialize access to m_prop
    TRANSFORM_PROPS m_prop;      // current transform properties
    BOOL            m_fPropsChanged; // communicates changes with transform
    BOOL            m_fDirty;    // communicates dirty to outside world
    struct
    {
        HWND        hwnd;
        UINT        id;

    } m_aNotify[NUMBER_OF_PROPERTY_PAGES * 2];

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;

    //
    //  NOTE! the m_pwfxIn and m_pwfxOut members will only get modified
    //  on a [re]-connect and do NOT require taking the m_Lock to
    //  access... in order for a connect to happen, the filter needs to
    //  be stopped.
    //
    LPWAVEFORMATEX  m_pwfxIn;    // this is the waveformat of the data in
                                 // each buffer. it is set when we start
                                 // streaming and valid as long as we are
                                 // streaming

    LPWAVEFORMATEX  m_pwfxOut;   // this is the waveformat of the data in
                                 // each buffer. it is set when we start
                                 // streaming and valid as long as we are
                                 // streaming
    //
    //  these are the 'cooked' values used by the transform--they are
    //  'cooked' from the m_props structure... also, they are ONLY
    //  accessed by the filter and do not require the m_Lock to access.
    //
    BOOL            m_fCooked;       // set TRUE if properly cooked
    double          m_dLevelL;
    double          m_dLevelR;

public:
    CAudioTransform(IUnknown *punk, HRESULT *phr);
    ~CAudioTransform();

    //
    //  These functions are called when the plug-in is started and stopped.
    //  They are the perfect place to allocate and release temporary memory.
    //
    HRESULT TransformBegin(LPWAVEFORMATEX pwfxIn, LPWAVEFORMATEX pwfxOut);
    HRESULT TransformEnd();

    //
    //  This is called whenever either the input pin or output pin
    //  gets a new media format.
    //
    STDMETHODIMP TransformSetFormat(LPWAVEFORMATEX pwfx, PIN_DIRECTION dir)
    {
        if (PINDIR_INPUT == dir)
            m_pwfxIn = pwfx;
        else if (PINDIR_OUTPUT == dir)
            m_pwfxOut = pwfx;

        return NOERROR;
    };

    //
    //  This function is called with an input buffer size and should return
    //  an output buffer size sufficient to process an input buffer of that
    //  size.
    //
    HRESULT TransformDecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProp);

   #if !defined NEED_CHANNELS_OUT
    //
    //  This function is called with the current output format. If this
    //  format is correct, the function should return S_OK. If it is not
    //  correct it should return S_FALSE
    //
    HRESULT TransformCheckOutputFormat(LPWAVEFORMATEX pwfxIn, LPWAVEFORMATEX pwfxOut);

    //
    //  This function is called with the current output format. It should
    //  change the pwfx to be the desired output format.
    //
    HRESULT TransformTweakOutputFormat(LPWAVEFORMATEX pwfx);
   #endif

    BOOL TransformGetProperties(LPTRANSFORM_PROPS pProps);
    HRESULT TransformCookProperties(LPTRANSFORM_PROPS pProps, TRANSFORM_STATE nState);

   #if !defined FLOAT_SAMPLES_ONLY
    //
    //  Called to process a buffer of 16-bit PCM samples. Return S_FALSE to
    //  cause buffer not to be written (or to indicate no more EOS data).
    //  EOS is indicated by psIn==NULL
    //
    HRESULT Transform16bit(TRANSFORM_STATE nState, LPSHORT psIn, LPDWORD pcsIn, LPSHORT psOut, LPDWORD pcsOut);
   #endif

   #ifdef FLOAT_SAMPLES_SUPPORTED
    //
    //  Called to process a buffer of 32-bit float samples. Return S_FALSE to
    //  cause buffer not to be written (or to indicate no more EOS data).
    //  EOS is indicated by psIn==NULL
    //
    HRESULT TransformFloat(TRANSFORM_STATE nState, LPFLOAT psIn, LPDWORD pcsIn, LPFLOAT psOut, LPDWORD pcsOut);
   #endif

    // ------ Implement ISfPluginProp interface ------------

    //
    //  This macro creates basic get_xxx, set_xxx and bound_xxx routines for
    //  a given property value
    //
    #define GETPUT_SFPROP(tag,typeIn,member,type,min,max)   \
        STDMETHODIMP get_##tag(typeIn * p##tag)             \
        {                                                   \
            *(p##tag) = (typeIn)m_prop.##member;            \
            return NOERROR;                                 \
        };                                                  \
        STDMETHODIMP put_##tag(typeIn x##tag)               \
        {                                                   \
            if ((min) < (max))                              \
            {                                               \
                if (x##tag < (typeIn)(min))                 \
                    x##tag = (typeIn)(min);                 \
                else if (x##tag > (typeIn)(max))            \
                    x##tag = (typeIn)(max);                 \
            }                                               \
            CAutoLock foo(&m_Lock);                         \
            if (m_prop.##member != (type)x##tag)            \
            {                                               \
                m_prop.##member = (type)x##tag;             \
                m_fPropsChanged = TRUE;                     \
                m_fDirty = TRUE;                            \
            }                                               \
            return NOERROR;                                 \
        };                                                  \
        void bound_##tag(void)                              \
        {                                                   \
            if ((min) < (max))                              \
            {                                               \
                if (m_prop.##member < (type)(min))          \
                    m_prop.##member = (type)(min);          \
                else if (m_prop.##member > (type)(max))     \
                    m_prop.##member = (type)(max);          \
            }                                               \
        };

    // declare get/set routines for the individual properties
    //
    GETPUT_SFPROP(Mode,     long, lMode,    long, SFPAN_MODE_PRESERVE_SEPARATION, SFPAN_MODE_MIX_BEFORE_PAN)
    GETPUT_SFPROP(PanLR,    long, lPanLR,   long, SFPAN_FULL_LEFT, SFPAN_FULL_RIGHT)
    GETPUT_SFPROP(OutGain,  long, lOutGain, long, SFPAN_OUT_GAIN_MIN, SFPAN_OUT_GAIN_MAX)

    STDMETHODIMP get_all(TRANSFORM_PROPS * pProp)
    {
        CAutoLock foo(&m_Lock);
        *pProp = m_prop;
        return NOERROR;
    };

    STDMETHODIMP put_all(TRANSFORM_PROPS * pProp)
    {
        CAutoLock foo(&m_Lock);
        m_prop = *pProp;
        m_fPropsChanged = TRUE;
        m_fDirty = TRUE;

        //
        //  don't forget to add any bounding to other parameters here
        //  this keeps things from blowing up if you are handed a
        //  garbage props structure...
        //
        bound_Mode();
        bound_PanLR();
        bound_OutGain();

        return NOERROR;
    };

    #undef GETPUT_PROP

    STDMETHODIMP get_WaveFormat(LPWAVEFORMATEX pwfx, PIN_DIRECTION  dir)
    {
        LPWAVEFORMATEX      pwfxT = NULL;

        if (PINDIR_INPUT == dir)
            pwfxT = m_pwfxIn;
        else if (PINDIR_OUTPUT == dir)
            pwfxT = m_pwfxOut;

        if (NULL == pwfxT)
        {
            DbgBreak("get_WaveFormat called when not connected!!");
            return VFW_E_NOT_CONNECTED;
        }

        *pwfx = *pwfxT;
        return NOERROR;
    };

    STDMETHODIMP put_NotifyWindow(HWND hwnd, UINT id);
    HRESULT TransformDispatchMessage(UINT uMsg);

    // helpers for a parent class implementing IPersistStream
    //
    struct _persist_version
    {
        DWORD           cb;
        CLSID           guid;
    };

    HRESULT TransformIsDirty()
    {
        return m_fDirty ? S_OK : S_FALSE;
    };

    void TransformSetDirty()
    {
        m_fDirty = TRUE;
    };

    HRESULT TransformRead(LPSTREAM pStream);
    HRESULT TransformWrite(LPSTREAM pStream, BOOL fClearDirty);
    DWORD TransformPersistSize(void);
};

#endif // !RC_INVOKED
#endif // __SFPLUGIN__
