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
#include <streams.h>
#include <commctrl.h>
#include <math.h>

#include "context.h"
#include "resource.h"
#include "sfstress.h"
#include "sfppage.h"


//--------------------------------------------------------------------------;
//
//
//
//--------------------------------------------------------------------------;

int SfFormatdB
(
    LPSTR                   psz,
    LONG                    lValue,
    LONG                    lInf
)
{
    int                 nn;

    if (lValue <= lInf)
    {
        lstrcpy(psz, "-Inf. dB");
        return (8);
    }

    if (lValue >= 0)
    {
        nn = 0;
    }
    else
    {
        lValue = abs(lValue);
        psz[0] = '-';
        nn = 1;
    }

    nn += wsprintf(&psz[nn], "%u.%u dB", lValue / 10, lValue % 10);

    return (nn);
}


//
//  This function NON-DESTRUCTIVELY returns a pointer to the first
//  character of the File Name within a File Path.
//
//  Arguments:
//      LPCTSTR pszFilePath: Pointer to a valid File Path.
//
//  Return (LPTSTR):
//      Pointer to first character within the File Path for the File Name.
//
LPTSTR SfGetFileNamePtr
(
    LPCTSTR                 pszFilePath
)
{
    LPCTSTR             pszFileName;

    //
    //  !!! NOTE !!! don't assume you know how to optimize this code
    //  unless you know what you are doing--this works from the FRONT
    //  to the END for a reason! (because DBCS must be parsed in that way)
    //
    for (pszFileName = pszFilePath;
         0 != *pszFilePath;
        #ifdef DBCS
         pszFilePath = AnsiNext(pszFilePath))
        #else
         ++pszFilePath)
        #endif
    {
        if ((':' == *pszFilePath) || ('\\' == *pszFilePath))
        {
            pszFileName = pszFilePath + 1;
        }
    }

    return (LPTSTR)pszFileName;
}

//
// build up a full pathname to the helpfile, assuming that the
// helpfile is in the same directory as the filter dll.
//
void SfGetHelpFilePath(
    LPTSTR pszHelpPath,
    UINT   cch)
{
    extern HINSTANCE    g_hInst;
    LPTSTR              psz;

    GetModuleFileName(g_hInst, pszHelpPath, cch);
    psz = SfGetFileNamePtr(pszHelpPath);
    cch -= (UINT)(psz - pszHelpPath);
    lstrcpyn(psz, SFPLUGIN_HELP_FILE, cch);
}

int SfContextHelp
(
    HWND                    hwnd,
    LPHELPINFO              phi,
    const SFCONTEXT_HELPID *paHelpIDs
)
{
    TCHAR               szHelpPath[MAX_PATH];
    DWORD               dwContext;
    UINT                fuCommand;

    SfGetHelpFilePath(szHelpPath, MAX_PATH);

    if (NULL == phi)
    {
        dwContext = 0;
        fuCommand = HELP_QUIT;
    }
    else
    {
        dwContext = IDH_OVERVIEW;
        fuCommand = HELP_CONTEXT;

        if (HELPINFO_WINDOW == phi->iContextType)
        {
            UINT    ii;

            // can we find a specific context for this window?
            //
            for (ii = 0; 0 != paHelpIDs[ii].iCtrlId; ++ii)
            {
                if (paHelpIDs[ii].iCtrlId == phi->iCtrlId)
                {
                    hwnd      = (HWND)phi->hItemHandle;
                    dwContext = (DWORD)paHelpIDs;
                    fuCommand = HELP_WM_HELP;
                    break;
                }
            }
        }
    }

    // bring up help for that context
    //
    return WinHelp(hwnd, szHelpPath, fuCommand, dwContext);
}


//--------------------------------------------------------------------------;
//
//
//
//--------------------------------------------------------------------------;

//
// CreateInstance
//
// Override CClassFactory method.
// Set punk to point to an IUnknown interface on a new CSfPlugInPropPage object
//
CUnknown *CSfPlugInPropPage::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CUnknown *punkNew = new CSfPlugInPropPage(punk, phr, 0);
    if (NULL == punkNew)
    {
        *phr = E_OUTOFMEMORY;
    }

    return punkNew;
}

//
// CSfPlugInPropPage constructor
//
// initialise a CSfPlugInPropPage object.
//
static const struct
{
    UINT     uIdDlg;
    UINT     uIdName;
}
g_aPlugInPropertyPages[] =
{
    {IDD_PROPPAGE_1,    IDS_PROPPAGE_1},
#ifdef MULTIPLE_PROPERTY_PAGES
    {IDD_PROPPAGE_2,    IDS_PROPPAGE_2},
    {IDD_PROPPAGE_3,    IDS_PROPPAGE_3},
#endif
};

CSfPlugInPropPage::CSfPlugInPropPage(
    LPUNKNOWN punk,
    HRESULT * phr,
    UINT      uPageId)
    :
    CBasePropertyPage(NAME(SFPLUGIN_NAME " Property Page"),
                      punk, 
                      g_aPlugInPropertyPages[uPageId].uIdDlg,
                      g_aPlugInPropertyPages[uPageId].uIdName)
    ,m_pProp(NULL)
{
}


//
// fill in general info for the property page.
//
//
STDMETHODIMP CSfPlugInPropPage::GetPageInfo(
    LPPROPPAGEINFO pPageInfo)
{
    CheckPointer(pPageInfo,E_POINTER);
    WCHAR wszTitle[STR_MAX_LENGTH];
    WideStringFromResource(wszTitle,m_TitleId);
    int Length = (lstrlenW(wszTitle) + 1) * sizeof(WCHAR);

    // Allocate dynamic memory for the property page title

    LPOLESTR pszTitle = (LPOLESTR) CoTaskMemAlloc(Length);
    if (pszTitle == NULL)
    {
        NOTE("No caption memory");
        return E_OUTOFMEMORY;
    }
    CopyMemory(pszTitle,wszTitle,Length);

    // Allocate dynamic memory for the property page help file

    LPOLESTR pszHelp = (LPOLESTR) CoTaskMemAlloc(MAX_PATH * sizeof(WCHAR) * 2);
    if (pszHelp == NULL)
    {
        NOTE("No help filename memory");
        CoTaskMemFree (pszTitle);
        return E_OUTOFMEMORY;
    }

    LPSTR  pszIn = (LPSTR)(pszHelp + MAX_PATH);
    SfGetHelpFilePath (pszIn, MAX_PATH);
    MultiByteToWideChar(CP_ACP, 0, pszIn, -1, pszHelp, MAX_PATH);

    pPageInfo->cb               = sizeof(PROPPAGEINFO);
    pPageInfo->pszTitle         = pszTitle;
    pPageInfo->pszDocString     = NULL;
    pPageInfo->pszHelpFile      = pszHelp;
    pPageInfo->dwHelpContext    = SFPLUGIN_HELP_CONTEXT;

    // Set defaults in case GetDialogSize fails
    pPageInfo->size.cx          = 300;
    pPageInfo->size.cy          = 150;

    GetDialogSize(m_DialogId, (DLGPROC)DialogProc, 0L, &pPageInfo->size);
    return NOERROR;
}


BOOL CSfPlugInPropPage::InitControls (void)
{
    static struct
    {
        int     id;
        int     min;
        int     max;
        int     tick;
        int     page;
    }
    aInit[] =
    {
        {IDC_SFSTRESS_FDR_DRYOUT,  0, SFSTRESS_DRY_GAIN_MAX - SFSTRESS_DRY_GAIN_MIN, 120, 10},
        {IDC_SFSTRESS_FDR_WETOUT,  0, SFSTRESS_WET_GAIN_MAX - SFSTRESS_WET_GAIN_MIN, 120, 10},
        {IDC_SFSTRESS_TRK_DELAY,    SFSTRESS_DELAY_MIN, SFSTRESS_DELAY_MAX, 1000, 100},
        {IDC_SFSTRESS_TRK_BUFFER,   SFSTRESS_BUFFERSIZE_MIN, SFSTRESS_BUFFERSIZE_MAX, 20000, 100},
        {IDC_SFSTRESS_TRK_LOAD,     SFSTRESS_LOAD_MIN, SFSTRESS_LOAD_MAX, 50, 10},
        {IDC_SFSTRESS_TRK_PRESET,   SFSTRESS_PRESETSIZE_MIN, SFSTRESS_PRESETSIZE_MAX, 10000, 100},

    };
    UINT ii;

    for (ii = 0; ii < NUMELMS(aInit); ++ii)
    {
        HWND hctl = GetDlgItem(m_hwnd, aInit[ii].id);
        if (hctl)
        {
            TrackBar_SetRange(hctl, aInit[ii].min, aInit[ii].max);
            TrackBar_SetTicFreq(hctl, aInit[ii].tick);
            TrackBar_SetPageSize(hctl, aInit[ii].page);
        }
    }

    static char szBlurb[] =
    "This plug-in is an example from the Sonic Foundry Plug-In "
    "Development Kit (PIDK). The PIDK is available free of charge "
    "on Sonic Foundry's web site (www.sonicfoundry.com).";

    SetDlgItemText(m_hwnd, IDC_SONIC_FOUNDRY_BLURB, szBlurb);

    return TRUE;
}


BOOL CSfPlugInPropPage::UpdateControls(
    UINT   uId)
{
    TCHAR               sz[40];
    HWND                hctl;
    long                nn;
    BOOL                ff;

    ASSERT(NULL != m_pProp);
    if ((NULL == m_pProp) || (NULL == m_hwnd))
    {
        return FALSE;
    }

    //
    //  IDC_SFPLUGIN_NOTIFY_MESSAGE is a special 'notification only' control
    //  identifier. It is commonly used to determine when a [re-]connection is
    //  made to the filter (either input or output pin). This is often
    //  necessary to know for refreshing controls that are dependent
    //  on the input and output pin formats.
    //
    if (IDC_SFPLUGIN_NOTIFY_MESSAGE == uId)
    {
        uId = 0;
    }

    // Check if a preset read or write failed
    if (0 == uId)
    {
        BOOL fWrite, fRead;
        m_pProp->get_PresetStatus(&fWrite, &fRead);

        if(fWrite)
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_WRITEFAIL, "Write Fail");
        else
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_WRITEFAIL, "Write Pass");


        if(fRead)
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_READFAIL, "Read Fail");
        else
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_READFAIL, "Read Pass");
    }

    // dry level
    //
    if ((0 == uId) || (IDC_SFSTRESS_FDR_DRYOUT == uId))
    {
        hctl = GetDlgItem(m_hwnd, IDC_SFSTRESS_FDR_DRYOUT);
        if (NULL != hctl)
        {
            m_pProp->get_DryLevel(&nn);
            if (0 == uId)
            {
                //
                //  Windows' trackbars are inverted from what a fader
                //  should be, so force the reverse logic here...
                //
                TrackBar_SetPos(hctl, SFSTRESS_DRY_GAIN_MAX - nn);
            }

            SfFormatdB(sz, nn, SFSTRESS_DRY_GAIN_MIN);
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_DRYOUT, sz);
        }
    }

    // wet level
    //
    if ((0 == uId) || (IDC_SFSTRESS_FDR_WETOUT == uId))
    {
        hctl = GetDlgItem(m_hwnd, IDC_SFSTRESS_FDR_WETOUT);
        if (NULL != hctl)
        {
            m_pProp->get_WetLevel(&nn);
            if (0 == uId)
            {
                //
                //  Windows' trackbars are inverted from what a fader
                //  should be, so force the reverse logic here...
                //
                TrackBar_SetPos(hctl, SFSTRESS_WET_GAIN_MAX - nn);
            }

            SfFormatdB(sz, nn, SFSTRESS_WET_GAIN_MIN);
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_WETOUT, sz);
        }
    }

    // delay time in millisec
    //
    if ((0 == uId) || (IDC_SFSTRESS_TRK_DELAY == uId))
    {
        m_pProp->get_DelayMillisec(&nn);
        hctl = GetDlgItem(m_hwnd, IDC_SFSTRESS_TRK_DELAY);
        if (NULL != hctl)
        {
            if (0 == uId)
            {
                TrackBar_SetPos(hctl, nn);
            }

            wsprintf (sz, "%d ms", nn);
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_DELAY, sz);
        }
    }

    // internal processing buffer size
    //
    if ((0 == uId) || (IDC_SFSTRESS_TRK_BUFFER == uId))
    {
        m_pProp->get_BufferSize(&nn);
        hctl = GetDlgItem(m_hwnd, IDC_SFSTRESS_TRK_BUFFER);
        if (NULL != hctl)
        {
            if (0 == uId)
            {
                TrackBar_SetPos(hctl, nn);
            }

            wsprintf (sz, "%d", nn);
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_BUFFER, sz);
        }
    }

    // internal processing load
    //
    if ((0 == uId) || (IDC_SFSTRESS_TRK_LOAD == uId))
    {
        m_pProp->get_Load(&nn);
        hctl = GetDlgItem(m_hwnd, IDC_SFSTRESS_TRK_LOAD);
        if (NULL != hctl)
        {
            if (0 == uId)
            {
                TrackBar_SetPos(hctl, nn);
            }

            wsprintf (sz, "%d", nn);
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_LOAD, sz);
        }
    }

    // size of preset 
    //
    if ((0 == uId) || (IDC_SFSTRESS_TRK_PRESET == uId))
    {
        DWORD   dwTemp;

        m_pProp->get_LargeArraySize(&dwTemp);
        hctl = GetDlgItem(m_hwnd, IDC_SFSTRESS_TRK_PRESET);
        if (NULL != hctl)
        {
            if (0 == uId)
            {
                TrackBar_SetPos(hctl, dwTemp);
            }

            wsprintf (sz, "%d", dwTemp);
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_PRESET, sz);
        }
    }

    // Check controls
    if ((uId == 0) || (IDC_SFSTRESS_CHK_OVERRUN == uId))
    {
        m_pProp->get_Overrun(&ff);       
        if (0 == uId)
            CheckDlgButton(m_hwnd,IDC_SFSTRESS_CHK_OVERRUN, ff); 
    }

    if ((uId == 0) || (IDC_SFSTRESS_CHK_FLOATERR== uId))
    {
        m_pProp->get_Floaterr(&ff);       
        if (0 == uId)
            CheckDlgButton(m_hwnd,IDC_SFSTRESS_CHK_FLOATERR, ff); 
    }

    return TRUE;
}


//
// OnReceiveMessage
//
// Override CBasePropertyPage method.
// Handle windows messages for the dialog of the property sheet.
//
BOOL CSfPlugInPropPage::OnReceiveMessage (
    HWND   hwnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            m_hwnd = hwnd;
            if (NULL != m_pProp)
            {
                m_pProp->put_NotifyWindow(m_hwnd, IDC_SFPLUGIN_NOTIFY_MESSAGE);
            }
            InitControls();
            UpdateControls(0);
            return TRUE;

        case WM_DESTROY:
            if (NULL != m_pProp)
            {
                m_pProp->put_NotifyWindow(m_hwnd, 0);
            }

            SfContextHelp(m_hwnd, NULL, NULL);
            m_hwnd = NULL;
            break;

        case WM_HELP:
        {
            static const SFCONTEXT_HELPID aHelpIDs[] =
            {
                {0,                                 0}
            };

            SfContextHelp(m_hwnd, (LPHELPINFO)lParam, aHelpIDs);
            return TRUE;
        }

        case WM_COMMAND:
            if (NULL != m_pProp)
            {
                UINT uNotify = GET_WM_COMMAND_CMD(wParam,lParam);
                UINT uId     = GET_WM_COMMAND_ID(wParam,lParam);
                HWND hctl    = GET_WM_COMMAND_HWND(wParam,lParam);
                BOOL ff;

                switch (uId)
                {
                    case IDC_SFPLUGIN_NOTIFY_MESSAGE:
                        //
                        //  LOWORD(wParam) -> notify id
                        //  HIWORD(wParam) -> message
                        //

                        switch (HIWORD(wParam))
                        {
                            case PINDIR_INPUT:
                            case PINDIR_OUTPUT:
                                break;

                            case IDC_SFPLUGIN_NOTIFY_FORCE_UPDATE:                                 
                                uId = 0;          
                                break;
                        }
                        break;

                    case IDC_SFSTRESS_CHK_OVERRUN:
                        ff = IsDlgButtonChecked(m_hwnd, uId);
                        m_pProp->put_Overrun(ff);            
                        break;

                    case IDC_SFSTRESS_CHK_FLOATERR:
                        ff = IsDlgButtonChecked(m_hwnd, uId);
                        m_pProp->put_Floaterr(ff);            
                        break;
                }

                UpdateControls(uId);
            }
            return TRUE;

        case WM_VSCROLL:
            if (NULL != m_pProp)
            {
                HWND        hctl = GET_WM_VSCROLL_HWND(wParam, lParam);
                UINT        uId  = GetDlgCtrlID(hctl);
                long        lPos;

                switch (uId)
                {
                    case IDC_SFSTRESS_FDR_DRYOUT:
                        lPos  = SFSTRESS_DRY_GAIN_MAX;
                        lPos -= TrackBar_GetPos(hctl);
                        m_pProp->put_DryLevel(lPos);
                        break;

                    case IDC_SFSTRESS_FDR_WETOUT:
                        lPos  = SFSTRESS_WET_GAIN_MAX;
                        lPos -= TrackBar_GetPos(hctl);
                        m_pProp->put_WetLevel(lPos);
                        break;
                }

                UpdateControls(uId);
            }
            return TRUE;

        case WM_HSCROLL:
            if (NULL != m_pProp)
            {
                HWND    hctl = GET_WM_HSCROLL_HWND(wParam,lParam);
                UINT    uId  = GetDlgCtrlID(hctl);
                LONG    lPos;

                switch (uId)
                {
                    case IDC_SFSTRESS_TRK_DELAY:
                        lPos = TrackBar_GetPos(hctl);
                        m_pProp->put_DelayMillisec(lPos);
                        break;

                    case IDC_SFSTRESS_TRK_BUFFER:
                        lPos = TrackBar_GetPos(hctl);
                        m_pProp->put_BufferSize(lPos);
                        break;

                    case IDC_SFSTRESS_TRK_LOAD:
                        lPos = TrackBar_GetPos(hctl);
                        m_pProp->put_Load(lPos);
                        break;

                    case IDC_SFSTRESS_TRK_PRESET:
                        lPos = TrackBar_GetPos(hctl);
                        m_pProp->put_LargeArraySize(lPos);
                        break;
                
                }

                UpdateControls(uId);
            }
            return TRUE;
    }

    return FALSE;
}

//
// OnConnect
//
// Override CBasePropertyPage method.
// Get the interface to the filter.
// init the dialog controls to reflect the current state of the filter
//
HRESULT CSfPlugInPropPage::OnConnect(IUnknown * punk)
{
    // Get ISfPlugInProp interface
    //
    if (punk == NULL)
    {
        DbgBreak("You can't call me with a NULL pointer!!");
        return E_POINTER;
    }

    // OnConnect can be called to update connected object even if 
    // the property page is already connected. (This could be optimized
    // to be more efficient if we are given the same object that we
    // already have an interface for.  In that case we would just
    // get object parameters and update our UI to reflect the object state).
    //
    if (NULL != m_pProp)
    {
        OnDisconnect();
    }

    HRESULT hr = punk->QueryInterface(IID_ISfPlugInProp, (void **) &m_pProp);
    if (FAILED(hr))
    {
        DbgBreak("Can't get ISfPlugInProp interface.");
        return E_NOINTERFACE;
    }

    ASSERT(NULL != m_pProp);

    // if we already have a window up, we need to update the controls
    // to match params for the new object we have just connected to.
    //
    if (NULL != m_hwnd)
    {
        m_pProp->put_NotifyWindow(m_hwnd, IDC_SFPLUGIN_NOTIFY_MESSAGE);
        UpdateControls(0);
    }

    return NOERROR;
}


//
// OnDisconnect
//
// Override CBasePropertyPage method.
// Release the private interface.
//
HRESULT CSfPlugInPropPage::OnDisconnect()
{
    //
    // Release the interface
    //
    if (NULL == m_pProp)
    {
        return E_UNEXPECTED;
    }

    if (NULL != m_hwnd)
    {
        m_pProp->put_NotifyWindow(m_hwnd, 0);
    }

    m_pProp->Release();
    m_pProp = NULL;

    return NOERROR;
}

//
// OnDeactivate
//
// Destroy the dialog.
//
HRESULT CSfPlugInPropPage::OnDeactivate(void)
{
    return NOERROR;
}


#ifdef MULTIPLE_PROPERTY_PAGES

//--------------------------------------------------------------------------;
//
//  Second Property Page
//
//--------------------------------------------------------------------------;

CUnknown *CSfPlugInPropPage2::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CUnknown *punkNew = new CSfPlugInPropPage2(punk, phr);
    if (NULL == punkNew)
    {
        *phr = E_OUTOFMEMORY;
    }

    return punkNew;
}

CSfPlugInPropPage2::CSfPlugInPropPage2(
    LPUNKNOWN lpunk,
    HRESULT * phr)
    :
    CSfPlugInPropPage(lpunk, phr, 1)
{
}

BOOL CSfPlugInPropPage2::InitControls(void)
{
    static struct
    {
        int     id;
        int     min;
        int     max;
        int     tick;
        int     page;
    }
    aInit[] =
    {
        {IDC_SFSTRESS_FDR_DRYOUT,  0, SFSTRESS_DRY_GAIN_MAX - SFSTRESS_DRY_GAIN_MIN, 120, 10},
        {IDC_SFSTRESS_FDR_WETOUT,  0, SFSTRESS_WET_GAIN_MAX - SFSTRESS_WET_GAIN_MIN, 120, 10},
        {IDC_SFSTRESS_TRK_DELAY,    SFSTRESS_DELAY_MIN, SFSTRESS_DELAY_MAX, 1000, 100},
    };
    UINT ii;

    for (ii = 0; ii < NUMELMS(aInit); ++ii)
    {
        HWND hctl = GetDlgItem(m_hwnd, aInit[ii].id);
        if (hctl)
        {
            TrackBar_SetRange(hctl, aInit[ii].min, aInit[ii].max);
            TrackBar_SetTicFreq(hctl, aInit[ii].tick);
            TrackBar_SetPageSize(hctl, aInit[ii].page);
        }
    }

    static char szBlurb[] =
    "This plug-in is an example from the Sonic Foundry Plug-In "
    "Development Kit (PIDK). The PIDK is available free of charge "
    "on Sonic Foundry's web site (www.sonicfoundry.com).";

    SetDlgItemText(m_hwnd, IDC_SONIC_FOUNDRY_BLURB, szBlurb);

    return TRUE;
}

BOOL CSfPlugInPropPage2::UpdateControls(
    UINT   uId)
{
    TCHAR               sz[40];
    HWND                hctl;
    long                nn;

    ASSERT(NULL != m_pProp);
    if ((NULL == m_pProp) || (NULL == m_hwnd))
    {
        return FALSE;
    }

    //
    //  IDC_SFPLUGIN_NOTIFY_MESSAGE is a special 'notification only' control
    //  identifier. It is commonly used to determine when a [re-]connection is
    //  made to the filter (either input or output pin). This is often
    //  necessary to know for refreshing controls that are dependent
    //  on the input and output pin formats.
    //
    if (IDC_SFPLUGIN_NOTIFY_MESSAGE == uId)
    {
        uId = 0;
    }

    // dry level
    //
    if ((0 == uId) || (IDC_SFSTRESS_FDR_DRYOUT == uId))
    {
        hctl = GetDlgItem(m_hwnd, IDC_SFSTRESS_FDR_DRYOUT);
        if (NULL != hctl)
        {
            m_pProp->get_DryLevel(&nn);
            if (0 == uId)
            {
                //
                //  Windows' trackbars are inverted from what a fader
                //  should be, so force the reverse logic here...
                //
                TrackBar_SetPos(hctl, SFSTRESS_DRY_GAIN_MAX - nn);
            }

            SfFormatdB(sz, nn, SFSTRESS_DRY_GAIN_MIN);
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_DRYOUT, sz);
        }
    }

    // wet level
    //
    if ((0 == uId) || (IDC_SFSTRESS_FDR_WETOUT == uId))
    {
        hctl = GetDlgItem(m_hwnd, IDC_SFSTRESS_FDR_WETOUT);
        if (NULL != hctl)
        {
            m_pProp->get_WetLevel(&nn);
            if (0 == uId)
            {
                //
                //  Windows' trackbars are inverted from what a fader
                //  should be, so force the reverse logic here...
                //
                TrackBar_SetPos(hctl, SFSTRESS_WET_GAIN_MAX - nn);
            }

            SfFormatdB(sz, nn, SFSTRESS_WET_GAIN_MIN);
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_WETOUT, sz);
        }
    }

    // delay time in millisec
    //
    if ((0 == uId) || (IDC_SFSTRESS_TRK_DELAY == uId))
    {
        m_pProp->get_DelayMillisec(&nn);
        hctl = GetDlgItem(m_hwnd, IDC_SFSTRESS_TRK_DELAY);
        if (NULL != hctl)
        {
            if (0 == uId)
            {
                TrackBar_SetPos(hctl, nn);
            }

            wsprintf (sz, "%d ms", nn);
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_DELAY, sz);
        }
    }

    return TRUE;
}

//
// OnReceiveMessage
//
// Override CBasePropertyPage method.
// Handle windows messages for the dialog of the property sheet.
//
BOOL CSfPlugInPropPage2::OnReceiveMessage (
    HWND   hwnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            m_hwnd = hwnd;
            if (NULL != m_pProp)
            {
                m_pProp->put_NotifyWindow(m_hwnd, IDC_SFPLUGIN_NOTIFY_MESSAGE);
            }
            InitControls();
            UpdateControls(0);
            return TRUE;

        case WM_DESTROY:
            if (NULL != m_pProp)
            {
                m_pProp->put_NotifyWindow(m_hwnd, 0);
            }

            SfContextHelp(m_hwnd, NULL, NULL);
            m_hwnd = NULL;
            break;

        case WM_HELP:
        {
            static const SFCONTEXT_HELPID aHelpIDs[] =
            {
                {0,                                 0}
            };

            SfContextHelp(m_hwnd, (LPHELPINFO)lParam, aHelpIDs);
            return TRUE;
        }

        case WM_COMMAND:
            if (NULL != m_pProp)
            {
                UINT uNotify = GET_WM_COMMAND_CMD(wParam,lParam);
                UINT uId     = GET_WM_COMMAND_ID(wParam,lParam);
                HWND hctl    = GET_WM_COMMAND_HWND(wParam,lParam);

                switch (uId)
                {
                    case IDC_SFPLUGIN_NOTIFY_MESSAGE:
                        //
                        //  LOWORD(wParam) -> notify id
                        //  HIWORD(wParam) -> message
                        //

                        switch (HIWORD(wParam))
                        {
                            case PINDIR_INPUT:
                            case PINDIR_OUTPUT:
                                break;

                            case IDC_SFPLUGIN_NOTIFY_FORCE_UPDATE:                                 
                                uId = 0;          
                                break;
                        }
                        break;
                }

                UpdateControls(uId);
            }
            return TRUE;

        case WM_VSCROLL:
            if (NULL != m_pProp)
            {
                HWND        hctl = GET_WM_VSCROLL_HWND(wParam, lParam);
                UINT        uId  = GetDlgCtrlID(hctl);
                long        lPos;

                switch (uId)
                {
                    case IDC_SFSTRESS_FDR_DRYOUT:
                        lPos  = SFSTRESS_DRY_GAIN_MAX;
                        lPos -= TrackBar_GetPos(hctl);
                        m_pProp->put_DryLevel(lPos);
                        break;

                    case IDC_SFSTRESS_FDR_WETOUT:
                        lPos  = SFSTRESS_WET_GAIN_MAX;
                        lPos -= TrackBar_GetPos(hctl);
                        m_pProp->put_WetLevel(lPos);
                        break;
                }

                UpdateControls(uId);
            }
            return TRUE;

        case WM_HSCROLL:
            if (NULL != m_pProp)
            {
                HWND    hctl = GET_WM_HSCROLL_HWND(wParam,lParam);
                UINT    uId  = GetDlgCtrlID(hctl);
                LONG    lPos;

                switch (uId)
                {
                    case IDC_SFSTRESS_TRK_DELAY:
                        lPos = TrackBar_GetPos(hctl);
                        m_pProp->put_DelayMillisec(lPos);
                        break;                
                }

                UpdateControls(uId);
            }
            return TRUE;
    }

    return FALSE;
}

//--------------------------------------------------------------------------;
//
//  Third Property Page
//
//--------------------------------------------------------------------------;

CUnknown *CSfPlugInPropPage3::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CUnknown *punkNew = new CSfPlugInPropPage3(punk, phr);
    if (NULL == punkNew)
    {
        *phr = E_OUTOFMEMORY;
    }

    return punkNew;
}

CSfPlugInPropPage3::CSfPlugInPropPage3(
    LPUNKNOWN lpunk,
    HRESULT * phr)
    :
    CSfPlugInPropPage(lpunk, phr, 2)
{
}

BOOL CSfPlugInPropPage3::InitControls(void)
{
    static struct
    {
        int     id;
        int     min;
        int     max;
        int     tick;
        int     page;
    }
    aInit[] =
    {
        {IDC_SFSTRESS_TRK_BUFFER,   SFSTRESS_BUFFERSIZE_MIN, SFSTRESS_BUFFERSIZE_MAX, 20000, 100},
        {IDC_SFSTRESS_TRK_LOAD,     SFSTRESS_LOAD_MIN, SFSTRESS_LOAD_MAX, 50, 10},
        {IDC_SFSTRESS_TRK_PRESET,   SFSTRESS_PRESETSIZE_MIN, SFSTRESS_PRESETSIZE_MAX, 10000, 100},
    };
    UINT ii;

    for (ii = 0; ii < NUMELMS(aInit); ++ii)
    {
        HWND hctl = GetDlgItem(m_hwnd, aInit[ii].id);
        if (hctl)
        {
            TrackBar_SetRange(hctl, aInit[ii].min, aInit[ii].max);
            TrackBar_SetTicFreq(hctl, aInit[ii].tick);
            TrackBar_SetPageSize(hctl, aInit[ii].page);
        }
    }

    static char szBlurb[] =
    "This plug-in is an example from the Sonic Foundry Plug-In "
    "Development Kit (PIDK). The PIDK is available free of charge "
    "on Sonic Foundry's web site (www.sonicfoundry.com).";

    SetDlgItemText(m_hwnd, IDC_SONIC_FOUNDRY_BLURB, szBlurb);

    return TRUE;
}

BOOL CSfPlugInPropPage3::UpdateControls(
    UINT   uId)
{
     TCHAR               sz[40];
    HWND                hctl;
    long                nn;

    ASSERT(NULL != m_pProp);
    if ((NULL == m_pProp) || (NULL == m_hwnd))
    {
        return FALSE;
    }

    //
    //  IDC_SFPLUGIN_NOTIFY_MESSAGE is a special 'notification only' control
    //  identifier. It is commonly used to determine when a [re-]connection is
    //  made to the filter (either input or output pin). This is often
    //  necessary to know for refreshing controls that are dependent
    //  on the input and output pin formats.
    //
    if (IDC_SFPLUGIN_NOTIFY_MESSAGE == uId)
    {
        uId = 0;
    }

    // Check if a preset read or write failed
    if (0 == uId)
    {
        BOOL fWrite, fRead;
        m_pProp->get_PresetStatus(&fWrite, &fRead);

        if(fWrite)
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_WRITEFAIL, "Write Fail");
        else
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_WRITEFAIL, "Write Pass");


        if(fRead)
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_READFAIL, "Read Fail");
        else
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_READFAIL, "Read Pass");
    }

    // internal processing buffer size
    //
    if ((0 == uId) || (IDC_SFSTRESS_TRK_BUFFER == uId))
    {
        m_pProp->get_BufferSize(&nn);
        hctl = GetDlgItem(m_hwnd, IDC_SFSTRESS_TRK_BUFFER);
        if (NULL != hctl)
        {
            if (0 == uId)
            {
                TrackBar_SetPos(hctl, nn);
            }

            wsprintf (sz, "%d", nn);
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_BUFFER, sz);
        }
    }

    // internal processing load
    //
    if ((0 == uId) || (IDC_SFSTRESS_TRK_LOAD == uId))
    {
        m_pProp->get_Load(&nn);
        hctl = GetDlgItem(m_hwnd, IDC_SFSTRESS_TRK_LOAD);
        if (NULL != hctl)
        {
            if (0 == uId)
            {
                TrackBar_SetPos(hctl, nn);
            }

            wsprintf (sz, "%d", nn);
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_LOAD, sz);
        }
    }

    // size of preset 
    //
    if ((0 == uId) || (IDC_SFSTRESS_TRK_PRESET == uId))
    {
        DWORD   dwTemp;

        m_pProp->get_LargeArraySize(&dwTemp);
        hctl = GetDlgItem(m_hwnd, IDC_SFSTRESS_TRK_PRESET);
        if (NULL != hctl)
        {
            if (0 == uId)
            {
                TrackBar_SetPos(hctl, dwTemp);
            }

            wsprintf (sz, "%d", dwTemp);
            SetDlgItemText(m_hwnd, IDC_SFSTRESS_TXT_PRESET, sz);
        }
    }

    return TRUE;
}

//
// OnReceiveMessage
//
// Override CBasePropertyPage method.
// Handle windows messages for the dialog of the property sheet.
//
BOOL CSfPlugInPropPage3::OnReceiveMessage (
    HWND   hwnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            m_hwnd = hwnd;
            if (NULL != m_pProp)
            {
                m_pProp->put_NotifyWindow(m_hwnd, IDC_SFPLUGIN_NOTIFY_MESSAGE);
            }
            InitControls();
            UpdateControls(0);
            return TRUE;

        case WM_DESTROY:
            if (NULL != m_pProp)
            {
                m_pProp->put_NotifyWindow(m_hwnd, 0);
            }

            SfContextHelp(m_hwnd, NULL, NULL);
            m_hwnd = NULL;
            break;

        case WM_HELP:
        {
            static const SFCONTEXT_HELPID aHelpIDs[] =
            {
                {0,                                 0}
            };

            SfContextHelp(m_hwnd, (LPHELPINFO)lParam, aHelpIDs);
            return TRUE;
        }

        case WM_COMMAND:
            if (NULL != m_pProp)
            {
                UINT uNotify = GET_WM_COMMAND_CMD(wParam,lParam);
                UINT uId     = GET_WM_COMMAND_ID(wParam,lParam);
                HWND hctl    = GET_WM_COMMAND_HWND(wParam,lParam);

                switch (uId)
                {
                    case IDC_SFPLUGIN_NOTIFY_MESSAGE:
                        //
                        //  LOWORD(wParam) -> notify id
                        //  HIWORD(wParam) -> message
                        //

                        switch (HIWORD(wParam))
                        {
                            case PINDIR_INPUT:
                            case PINDIR_OUTPUT:
                                break;

                            case IDC_SFPLUGIN_NOTIFY_FORCE_UPDATE:                                 
                                uId = 0;          
                                break;
                        }
                        break;
                }

                UpdateControls(uId);
            }
            return TRUE;

#if 0
        case WM_VSCROLL:
            if (NULL != m_pProp)
            {
                HWND        hctl = GET_WM_VSCROLL_HWND(wParam, lParam);
                UINT        uId  = GetDlgCtrlID(hctl);
                long        lPos;

                switch (uId)
                {
                    case IDC_???:
                        break;
                }

                UpdateControls(uId);
            }
            return TRUE;
#endif

        case WM_HSCROLL:
            if (NULL != m_pProp)
            {
                HWND    hctl = GET_WM_HSCROLL_HWND(wParam,lParam);
                UINT    uId  = GetDlgCtrlID(hctl);
                LONG    lPos;

                switch (uId)
                {
                    case IDC_SFSTRESS_TRK_BUFFER:
                        lPos = TrackBar_GetPos(hctl);
                        m_pProp->put_BufferSize(lPos);
                        break;

                    case IDC_SFSTRESS_TRK_LOAD:
                        lPos = TrackBar_GetPos(hctl);
                        m_pProp->put_Load(lPos);
                        break;

                    case IDC_SFSTRESS_TRK_PRESET:
                        lPos = TrackBar_GetPos(hctl);
                        m_pProp->put_LargeArraySize(lPos);
                        break;            
                }

                UpdateControls(uId);
            }
            return TRUE;
    }

    return FALSE;
}

#endif // Multiple property pages

//
//  "unreferenced inline function has been removed"
//
#pragma warning(disable: 4514)
