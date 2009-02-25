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
#define SFPLUGIN_NAME           "Sonic Foundry PIDK Invert"


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
//#define NEED_CHANNELS_OUT   (2)     // force mono/stereo out


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
            0xD616F3A0, 0xD622, 0x11CE, 0xAA, 0xC5,
            0x00, 0x20, 0xAF, 0x0B, 0x99, 0xA3);


//
//  GUID for the plug-in's property manipulation interface. This MUST exist
//  if your plug-in has property pages. It is recommended, but not required,
//  to make this interface public (i.e. document it for developers).
//
//  {D616F3BF-D622-11CE-AAC5-0020AF0B99A3}
//
DEFINE_GUID(IID_ISfPlugInProp,
            0xD616F3AF, 0xD622, 0x11CE, 0xAA, 0xC5,
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


// class used to wrap an audio transform and Isolate a programmer
// from the details of ActiveMovie.
//
class CAudioTransform
{
    private:
        LPWAVEFORMATEX  m_pwfx;      // this is the waveformat of the data
                                     // in each buffer.  it is set
                                     // when we start streaming
                                     // and valid as long as we are streaming

    public:
        CAudioTransform(IUnknown * punk, HRESULT * phr);
        ~CAudioTransform();

        // these functions are called when the filters is started
        // and stopped. they are the perfect place to allocate
        // and release temp memory.
        //
        HRESULT TransformBegin (LPWAVEFORMATEX pwfx);
        HRESULT TransformEnd ();

       #if !defined FLOAT_SAMPLES_ONLY
        // called to process a buffer of 16 bit PCM samples,
        // return S_FALSE to cause buffer not to be written.
        // (or to indicate no more EOS data)
        //
        HRESULT Transform16bit(LPSHORT psBuffer, DWORD csBuffer);
       #endif

       #ifdef FLOAT_SAMPLES_SUPPORTED
        // called to process 1 channel of a buffer of float samples
        // return S_FALSE to cause buffer not to be written.
        // (or to indicate no more EOS data)
        //
        HRESULT TransformFloat(LPFLOAT psBuffer, DWORD csBuffer);
       #endif

};

#endif // !RC_INVOKED
#endif // __SFPLUGIN__
