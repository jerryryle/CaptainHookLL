/*  ----------------------------------------------------------------------------
    Copyright (c) 2017, Jerry Ryle.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Author(s):  Jerry Ryle <jerryryle@gmail.com>

------------------------------------------------------------------------- */
#include "stdafx.h"
#include "NotificationIcon.h"

CNotificationIcon::CNotificationIcon() :
    m_enabled(FALSE),
    m_nid({ sizeof(m_nid) })
{
}

CNotificationIcon::~CNotificationIcon()
{
    Disable();
    if (m_nid.hIcon) {
        ::DestroyIcon(m_nid.hIcon);
        m_nid.hIcon = NULL;
    }
}

BOOL CNotificationIcon::SetIcon(HICON hIcon)
{
    if (m_nid.hIcon) {
        ::DestroyIcon(m_nid.hIcon);
        m_nid.hIcon = NULL;
    }
    m_nid.hIcon = ::DuplicateIcon(NULL, hIcon);
    m_nid.uFlags |= NIF_ICON;
    return Update(NIM_MODIFY);
}

BOOL CNotificationIcon::SetTooltipText(LPCTSTR pszTooltipText)
{
    static size_t const max_tip_size = sizeof(m_nid.szTip) / sizeof(m_nid.szTip[0]);
    _tcsncpy_s(m_nid.szTip, pszTooltipText, _TRUNCATE);
    m_nid.uFlags |= NIF_TIP;
    return Update(NIM_MODIFY);
}

BOOL CNotificationIcon::SetInfo(LPCTSTR pszInfoTitle, LPCTSTR pszInfo, DWORD dwInfoFlags, HICON hBalloonIcon)
{
    static size_t const max_title_size = sizeof(m_nid.szInfoTitle) / sizeof(m_nid.szInfoTitle[0]);
    static size_t const max_info_size = sizeof(m_nid.szInfo) / sizeof(m_nid.szInfo[0]);
    _tcsncpy_s(m_nid.szInfoTitle, pszInfoTitle, _TRUNCATE);
    _tcsncpy_s(m_nid.szInfo, pszInfo, _TRUNCATE);
    m_nid.dwInfoFlags = dwInfoFlags;
    m_nid.hBalloonIcon = hBalloonIcon;
    return Update(NIM_MODIFY);
}

BOOL CNotificationIcon::SetFocus()
{
    if (!m_enabled) {
        return FALSE;
    }
    return ::Shell_NotifyIcon(NIM_SETFOCUS, &m_nid);
}

BOOL CNotificationIcon::Remove()
{
    return ::Shell_NotifyIcon(NIM_DELETE, &m_nid);
}

BOOL CNotificationIcon::Update(DWORD dwMessage)
{
    if (!m_enabled) {
        return TRUE;
    }
    BOOL success = ::Shell_NotifyIcon(dwMessage, &m_nid);
    if (success) {
        m_nid.uFlags &= ~(NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_INFO | NIF_REALTIME);
    }
    return success;
}

BOOL CNotificationIcon::SetVersion()
{
    if (!m_enabled) {
        return FALSE;
    }
    return ::Shell_NotifyIcon(NIM_SETVERSION, &m_nid);
}

BOOL CNotificationIcon::Enable(HWND hWnd, UINT uCallbackMessage, UINT uID)
{
    if (m_enabled || !m_nid.hIcon || !hWnd) {
        return FALSE;
    }

    m_nid.hWnd = hWnd;
    m_nid.uCallbackMessage = uCallbackMessage;
    m_nid.uFlags |= NIF_MESSAGE;
    m_nid.uID = uID;
    m_nid.uVersion = NOTIFYICON_VERSION_4;
    m_enabled = TRUE;

    return Update(NIM_ADD) && SetVersion();
}

BOOL CNotificationIcon::Disable()
{
    if (!m_enabled) {
        return FALSE;
    }

    m_enabled = FALSE;

    return Remove();
}
