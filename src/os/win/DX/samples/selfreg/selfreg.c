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
//  This code can be used to write an application that calls the self
//  registration routines of DirectX Media Streaming Services (ActiveMovie)
//  plug-ins. This can be used in custom setup programs or whatever... This
//  code is actually pretty close to what Sonic Foundry's Sound Forge program
//  uses when you drag and drop a DirectX plug-in on it.
//
//  Another option for setup programs is to use REGSVR32.EXE...
//
//
//==========================================================================;

#include <windows.h>
#include <windowsx.h>

char g_szProductName[]  = "Self Register Thing";

typedef HRESULT (STDAPICALLTYPE *OLESELFREGPROC)(void);

HRESULT SfSelfRegister_IsSupported
(
    LPCTSTR                 pszFilePath
)
{
    HRESULT             hr;
    DWORD               dw;
    DWORD               cbInfo;
    LPVOID              pvData;
    LPDWORD             pdwInfo;
    char                szVerPath[128];
    BOOL                f;

    // Get the size of the version information.
    cbInfo = GetFileVersionInfoSize((LPTSTR)pszFilePath, &dw);
    if (0 == cbInfo)
    {
        return (E_FAIL);
    }

    hr = E_NOTIMPL;

    pvData = GlobalAllocPtr(GHND, cbInfo);
    while (NULL != pvData)
    {
        // Fill the buffer with the version information.
        f = GetFileVersionInfo((LPTSTR)pszFilePath, 0, cbInfo, pvData);    
        if (!f)
        {
            break;
        }

        // Get the translation information.
        f = VerQueryValue(pvData, "\\VarFileInfo\\Translation",
                            (void**)&pdwInfo, &dw);
        if (!f || (0 == dw))
        {
            break;
        }

        //  Build the path to the key OLESelfRegister using the
        //  translation information.
        wsprintf(szVerPath, "\\StringFileInfo\\%04hX%04hX\\OLESelfRegister",
                    LOWORD(pdwInfo[0]), HIWORD(pdwInfo[0])) ;

        // Search for the key.
        f = VerQueryValue(pvData, szVerPath, (void**)&pdwInfo, &dw);
        if (f)
        {
            hr = NOERROR;
        }

        break;
    }

    if (NULL != pvData)
    {
        GlobalFreePtr(pvData);
    }

    return (hr);
}


HRESULT SfSelfRegister_Register
(
    LPCTSTR                 pszFilePath,
    BOOL                    fUnregister
)
{
    HRESULT             hr;
    HINSTANCE           hinstModule;
    UINT                u;
    OLESELFREGPROC      pfnRegister;
    LPCTSTR             pszFunction;

    hr = SfSelfRegister_IsSupported(pszFilePath);
    if (NOERROR != hr)
    {
        return (hr);
    }

    u = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    hinstModule = LoadLibraryEx(pszFilePath, NULL,
                                LOAD_WITH_ALTERED_SEARCH_PATH);
    SetErrorMode(u);

    if (hinstModule < (HINSTANCE)HINSTANCE_ERROR)
    {
        return (E_FAIL);
    }

    pszFunction = fUnregister ? "DllUnregisterServer" : "DllRegisterServer";

    pfnRegister = (OLESELFREGPROC)GetProcAddress(hinstModule, pszFunction);
    if (NULL != pfnRegister)
    {
        hr = pfnRegister();
    }
    else
    {
        hr = E_NOTIMPL;
    }

    FreeLibrary(hinstModule);

    return (hr);    
}


BOOL SfDoSelfRegistration
(
    HWND                    hwnd,
    LPCTSTR                 pszFilePath
)
{
    TCHAR               ach[512];
    HRESULT             hr;
    BOOL                fr;
    int                 n;

    fr = TRUE;

    hr = SfSelfRegister_IsSupported(pszFilePath);
    switch (hr)
    {
        case NOERROR:
            wsprintf(ach,
                    "The file %s is a DirectX Plug-In that supports "
                    "self registration.\n\n"
                    "Would you like to register it now (click No to "
                    "un-register)?",
                        pszFilePath);
            n = MessageBox(hwnd, ach, g_szProductName, 
                            MB_YESNOCANCEL | MB_ICONQUESTION);
            if (IDCANCEL != n)
            {
                BOOL        fUnregister = (IDNO == n);

                hr = SfSelfRegister_Register(pszFilePath, fUnregister);
                if (NOERROR == hr)
                {
                    wsprintf(ach,
                            "The DirectX Plug-In %s was "
                            "successfully %s.",
                            pszFilePath,
                            fUnregister ? "un-registered" : "registered");
                    MessageBox(hwnd, ach, g_szProductName, 
                                MB_OK | MB_ICONINFORMATION);
                }
                else
                {
                    wsprintf(ach,
                            "The DirectX Plug-In %s failed "
                            "to %s properly. (%.08lXh)",
                            pszFilePath,
                            fUnregister ? "un-register" : "register",
                            hr);

                    MessageBox(hwnd, ach, g_szProductName, 
                                MB_OK | MB_ICONINFORMATION);

                    hr = NOERROR;
                }
            }
            break;

        case E_NOTIMPL:
            wsprintf(ach,
                "The file %s is a binary file that does not support"
                "self registration and is most likely not a DirectX "
                "Plug-In.",
                    pszFilePath);

            MessageBox(hwnd, ach, g_szProductName, 
                        MB_OK | MB_ICONINFORMATION);
            break;

        default:
            //
            //  ok, not even a binary.. 
            //
            fr = FALSE;
            break;
    }

    return (fr);
}

