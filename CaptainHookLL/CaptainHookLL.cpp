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
#include "CaptainHookLL.h"
#include "stdio.h"
#include "NotificationIcon.h"

//
// Constants
//

enum wmapp_messages {
    WMAPP_NOTIFYCALLBACK = WM_APP + 1,
};

static UINT const UID_CAPTAINHOOKLL = 1;
static UINT const IDT_KEYSTROKETIMER = 1;

//
// Function declarations
//

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static void RegisterWindowClass(HINSTANCE hInstance, LPCTSTR pszClassName, LPCTSTR pszMenuName, WNDPROC lpfnWndProc, WORD iconId);
static void ShowContextMenu(HWND hWnd);
static HWND CreateApplicationWindow(HINSTANCE hInstance);
static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
static HHOOK RegisterKeyboardHook();
static BOOL UnregisterKeyboardHook(HHOOK hhk);
static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
static BOOL HandleKeyEvent(HWND hWnd, DWORD keycode, BOOL keyIsDown, BOOL isSystemKey);

//
// Global variables
//

static HWND g_hWnd = NULL;
static HINSTANCE g_hInstance = NULL;
static HHOOK g_hLLHook = NULL;
static CNotificationIcon g_NotificationIcon;


int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    g_hInstance = hInstance;
    g_hWnd = CreateApplicationWindow(g_hInstance);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        /* Install the low level hook to trap keyboard input. */
        g_hLLHook = RegisterKeyboardHook();

        /* Configure and enable the notification icon (the app's only UI) */
        g_NotificationIcon.SetIcon(::LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_NOTIFICATIONHOOK)));
        g_NotificationIcon.SetTooltipText(_T("Captain Hook"));
        g_NotificationIcon.Enable(hWnd, WMAPP_NOTIFYCALLBACK, UID_CAPTAINHOOKLL);
        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_TIMER:
        switch (wParam) {
        case IDT_KEYSTROKETIMER:
            ::KillTimer(hWnd, IDT_KEYSTROKETIMER);
            g_NotificationIcon.SetIcon(::LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_NOTIFICATIONHOOK)));
            break;

        default:
            break;
        }
        break;

    case WMAPP_NOTIFYCALLBACK:
        switch (LOWORD(lParam)) {
        case NIN_SELECT:
            break;
        case NIN_BALLOONTIMEOUT:
            break;
        case NIN_BALLOONUSERCLICK:
            break;
        case WM_CONTEXTMENU:
            ShowContextMenu(hWnd);
            break;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        case IDM_ABOUT:
            DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_DESTROY:
    case WM_ENDSESSION:
        /* Remove the low-level keyboard hook */
        UnregisterKeyboardHook(g_hLLHook);

        /* Remove the notification icon */
        g_NotificationIcon.Disable();

        /* Quit the app */
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

static void RegisterWindowClass(HINSTANCE hInstance, LPCTSTR pszClassName, LPCTSTR pszMenuName, WNDPROC lpfnWndProc, WORD iconId)
{
    WNDCLASSEX wcex = { sizeof(wcex) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = lpfnWndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(iconId));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = pszMenuName;
    wcex.lpszClassName = pszClassName;
    RegisterClassEx(&wcex);
}

static void ShowContextMenu(HWND hWnd)
{
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDC_CAPTAINHOOKLL));
    if (hMenu) {
        // We want to display only what's under the "file" menu.
        HMENU hSubMenu = GetSubMenu(hMenu, 0);
        if (hSubMenu) {
            // our window must be in the foreground before calling TrackPopupMenu
            // or the menu will not disappear when the user clicks away
            SetForegroundWindow(hWnd);

            // respect menu drop alignment
            UINT uFlags = TPM_RIGHTBUTTON | TPM_BOTTOMALIGN;
            if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0) {
                uFlags |= TPM_RIGHTALIGN;
            } else {
                uFlags |= TPM_LEFTALIGN;
            }

            TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hWnd, NULL);
        }
        DestroyMenu(hMenu);
    }
}

static HWND CreateApplicationWindow(HINSTANCE hInstance)
{
    RegisterWindowClass(hInstance, _T("__CaptainHook"), NULL, WndProc, IDI_CAPTAINHOOKLL);

    return ::CreateWindowEx(0,
        _T("__CaptainHook"),
        _T("Captain Hook"),
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
        0, 0, 200, 200,
        NULL,
        NULL,
        hInstance,
        0);
}

static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Message handler for about box.
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

static HHOOK RegisterKeyboardHook()
{
    return ::SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, g_hInstance, 0);
}

static BOOL UnregisterKeyboardHook(HHOOK hhk)
{
    if (!hhk) {
        return FALSE;
    }
    return ::UnhookWindowsHookEx(hhk);
}

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION) {
        if ((wParam == WM_KEYDOWN) || (wParam == WM_KEYUP) ||
            (wParam == WM_SYSKEYDOWN) || (wParam == WM_SYSKEYUP)) {
            KBDLLHOOKSTRUCT *kbhook = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
            BOOL keyIsDown = ((wParam == WM_KEYDOWN) || (wParam == WM_SYSKEYDOWN));
            BOOL keyIsSystemKey = ((wParam == WM_SYSKEYDOWN) || (wParam == WM_SYSKEYUP));
            BOOL shouldKeepProcessing = HandleKeyEvent(g_hWnd, kbhook->vkCode, keyIsDown, keyIsSystemKey);
            if (!shouldKeepProcessing) {
                // Prevent this keystroke from making it further in the hook chain or to the application.
                return 1;
            }
        }
    }

    return ::CallNextHookEx(NULL, nCode, wParam, lParam);
}

static BOOL HandleKeyEvent(HWND hWnd, DWORD keycode, BOOL keyIsDown, BOOL keyIsSystemKey)
{
    /* keyIsSystemKey is TRUE if ALT is held. If you don't care about the different between
       KEY vs ALT-KEY, then you don't need this. */
    UNREFERENCED_PARAMETER(keyIsSystemKey);

    static int count = 0;
    switch (keycode) {
    case 'A':
        /* This demonstrates a key event handler that performs one action when a key is pressed
        and another when the key is released. */
        if (keyIsDown) {
            /* Note that this will occurr repeatedly while a key is held. If you care about
                catching only the transition from released to held, you'll need to track that
                state yourself. */
            // Key pressed or held.
            g_NotificationIcon.SetIcon(::LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_NOTIFICATIONHOOKFISH)));
        }
        else {
            // Key released
            g_NotificationIcon.SetIcon(::LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_NOTIFICATIONHOOK)));
        }
        break;

    case 'B':
        /* This demonstrates a key event handler that performs an action when a key is released
        and sets a timer to clear the action a fixed time later. This timer is extended every time
        the key is released. */
        if (!keyIsDown) {
            g_NotificationIcon.SetIcon(::LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_NOTIFICATIONHOOKBAIT)));
            ::SetTimer(hWnd, IDT_KEYSTROKETIMER, 250, NULL);
        }
        break;

    case VK_PRIOR: /* Page Up Key */
        break;

    case VK_NEXT: /* Page Down Key */
        break;
    default:
        // If we're not processing the key, return TRUE to allow it to be passed to the application.
        return TRUE;
    }
    // If we processed the key, return FALSE to prevent it from reaching the application.
    return FALSE;
}