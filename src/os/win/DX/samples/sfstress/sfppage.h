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
//  sfppage.h
//
//  This file is entirely concerned with the implementation of the
//  properties page. It uses the property page base class to minimise
//  the implementation effort.
//
//==========================================================================;

#ifndef __SFPPAGE__
#define __SFPPAGE__

//
//  message notification id
//
#define IDC_SFPLUGIN_NOTIFY_MESSAGE         32700

#define IDC_SFPLUGIN_NOTIFY_FORCE_UPDATE      100

//
//
//
typedef struct tSFCONTEXT_HELPID
{
    int         iCtrlId;
    DWORD       dwContext;

} SFCONTEXT_HELPID;


//
//  some helper macros for dealing with Windows' trackbars...
//
#ifndef TrackBar_SetRange
#define TrackBar_SetRange(hctl, nMin, nMax) \
   SendMessage (hctl, TBM_SETRANGEMIN, FALSE, (LPARAM)(nMin)), SendMessage (hctl, TBM_SETRANGEMAX, TRUE, (LPARAM)(nMax))
#define TrackBar_GetRange(hctl, pnMin, pnMax) \
   (*(pnMin) = SendMessage(hctl, TBM_GETRANGEMIN,0,0)), (*(pnMax) = SendMessage(hctl, TBM_GETRANGEMAX,0,0))
#define TrackBar_SetPos(hctl,nPos) SendMessage(hctl, TBM_SETPOS,TRUE,(LPARAM)(nPos))
#define TrackBar_GetPos(hctl) SendMessage(hctl, TBM_GETPOS,0,0)
#define TrackBar_SetPageSize(hctl,nPage) SendMessage(hctl, TBM_SETPAGESIZE, 0, (LPARAM)(nPage));
#define TrackBar_SetTicFreq(hctl,nTick) SendMessage(hctl, TBM_SETTICFREQ, (WPARAM)(nTick), 0);
#endif


class CSfPlugInPropPage : public CBasePropertyPage
{
public:
    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    // Overrides from CBasePropertyPage
    HRESULT OnConnect(IUnknown * punk);
    HRESULT OnDisconnect(void);

    HRESULT OnDeactivate(void);

    STDMETHODIMP GetPageInfo(LPPROPPAGEINFO pPageInfo);

    CSfPlugInPropPage(LPUNKNOWN lpunk, HRESULT *phr, UINT uPageId);

protected:
    BOOL OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual BOOL InitControls ();
    virtual BOOL UpdateControls (UINT uId);

    ISfPlugInProp *m_pProp; // pointer to the ISfPlugInProp
                            // interface (set up in OnConnect)
};


#ifdef MULTIPLE_PROPERTY_PAGES

class CSfPlugInPropPage2 : public CSfPlugInPropPage
{
public:
    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    CSfPlugInPropPage2(LPUNKNOWN lpunk, HRESULT *phr);

private:
    BOOL OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL InitControls ();
    BOOL UpdateControls (UINT uId);
};


class CSfPlugInPropPage3 : public CSfPlugInPropPage
{
public:
    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    CSfPlugInPropPage3(LPUNKNOWN lpunk, HRESULT *phr);

private:
    BOOL OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL InitControls ();
    BOOL UpdateControls (UINT uId);
};

#endif

#endif // __SFPPAGE__
