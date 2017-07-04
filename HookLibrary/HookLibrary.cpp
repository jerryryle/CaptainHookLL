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
#include "HookLibrary.h"

HINSTANCE g_hInstance = NULL;
LRESULT CALLBACK KeyboardProc(int, WPARAM, LPARAM);

#pragma data_seg(".SData")
HHOOK g_hMsgHook = NULL;
UINT g_KBoardMessage = NULL;
HWND g_hParentWnd = NULL;
#pragma data_seg( )

// Create a shared section with RWS attributes
#pragma comment(linker,"/SECTION:.SData,RWS")

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        g_hInstance = (HINSTANCE)hModule;
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C" {

    HOOKLIBRARY_API int InstallHook(HWND hWnd, UINT UpdateMsg)
    {
        if (hWnd == NULL) {
            return -1;
        }

        g_hParentWnd = hWnd;
        g_KBoardMessage = UpdateMsg;

        g_hMsgHook = ::SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, g_hInstance, 0);

        if (g_hMsgHook == NULL) {
            return -1;
        }
        return 0;
    };

    HOOKLIBRARY_API int UninstallHook()
    {
        UnhookWindowsHookEx(g_hMsgHook);
        g_hMsgHook = NULL;
        return 0;
    };

}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION) {
        PostMessage(g_hParentWnd, g_KBoardMessage, wParam, lParam);
    }

    return CallNextHookEx(g_hMsgHook, nCode, wParam, lParam);
};
