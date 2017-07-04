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
#pragma once
#include <shellapi.h>

class CNotificationIcon
{
public:
    CNotificationIcon();
    virtual ~CNotificationIcon();

    BOOL SetIcon(HICON hIcon);
    BOOL SetTooltipText(LPCTSTR pszTooltipText);

    enum {
        ICON_NONE = NIIF_NONE,
        ICON_INFO = NIIF_INFO,
        ICON_WARNING = NIIF_WARNING,
        ICON_ERROR = NIIF_ERROR,
        ICON_USER = NIIF_USER,
        ICON_LARGE = NIIF_LARGE_ICON,

        NO_SOUND = NIIF_NOSOUND,
        RESPECT_QUIET_TIME = NIIF_RESPECT_QUIET_TIME
    };

    BOOL SetInfo(
        LPCTSTR pszInfoTitle, LPCTSTR pszInfo,
        DWORD dwInfoFlags = CNotificationIcon::ICON_NONE | CNotificationIcon::NO_SOUND | CNotificationIcon::RESPECT_QUIET_TIME,
        HICON hBalloonIcon = NULL);

    BOOL SetFocus();

    BOOL Enable(HWND hWnd, UINT uCallbackMessage, UINT uID);
    BOOL Disable();

protected:
    BOOL Update(DWORD dwMessage);
    BOOL SetVersion();
    BOOL Remove();

protected:
    BOOL            m_enabled;
    NOTIFYICONDATA  m_nid;
};
