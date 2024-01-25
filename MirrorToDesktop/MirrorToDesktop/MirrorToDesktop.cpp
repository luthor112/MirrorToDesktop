#include <iostream>
#include <Windows.h>

#define MIRROR_WINAMP
//#define MIRROR_CLASS NULL
//#define MIRROR_CAPTION NULL
//#define MIRROR_HWND 0xDEADBEEF
#define HIDE_CONSOLE
#define MIRROR_FPS 30

// Defines from wa_ipc.h
#define WM_WA_IPC WM_USER
#define IPC_GETVISWND 612
/* (requires Winamp 5.0+)
** int viswnd=(HWND)SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_GETVISWND);
** This returns a HWND to the visualisation command handler window if set by IPC_SETVISWND.
*/

// https://stackoverflow.com/a/56132585
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    HWND p = FindWindowEx(hwnd, NULL, L"SHELLDLL_DefView", NULL);
    HWND* ret = (HWND*)lParam;

    if (p)
    {
        // Gets the WorkerW Window after the current one.
        *ret = FindWindowEx(NULL, hwnd, L"WorkerW", NULL);
    }
    return true;
}

// https://stackoverflow.com/a/56132585
HWND get_wallpaper_window()
{
    // Fetch the Progman window
    HWND progman = FindWindow(L"ProgMan", NULL);
    // Send 0x052C to Progman. This message directs Progman to spawn a 
    // WorkerW behind the desktop icons. If it is already there, nothing 
    // happens.
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
    // We enumerate all Windows, until we find one, that has the SHELLDLL_DefView 
    // as a child. 
    // If we found that window, we take its next sibling and assign it to workerw.
    HWND wallpaper_hwnd = nullptr;
    EnumWindows(EnumWindowsProc, (LPARAM)&wallpaper_hwnd);
    // Return the handle you're looking for.
    return wallpaper_hwnd;
}

int main()
{
    std::cout << "Getting wallpaper HDC..." << std::endl;
    HWND hwnd_Desktop = get_wallpaper_window();
    HDC hDC_Desktop = GetDC(hwnd_Desktop);

    std::cout << "Clearing wallpaper..." << std::endl;
    RECT desktopRect;
    GetWindowRect(hwnd_Desktop, &desktopRect);
    HBRUSH blueBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hDC_Desktop, &desktopRect, blueBrush);

#ifdef MIRROR_WINAMP
    HWND winampWnd = FindWindowA("Winamp v1.x", NULL);
    HWND copyWnd = (HWND)SendMessage(winampWnd, WM_WA_IPC, 0, IPC_GETVISWND);
#elif defined(MIRROR_CLASS) && defined(MIRROR_CAPTION)
    HWND copyWnd = FindWindowA(MIRROR_CLASS, MIRROR_CAPTION);
#else
    HWND copyWnd = (HWND)MIRROR_HWND;
#endif

    // https://stackoverflow.com/a/14407301
    HDC TargetDC = GetDC(copyWnd);
    RECT rect;
    GetWindowRect(copyWnd, &rect);

    std::cout << "Start mirroring..." << std::endl;
#ifdef HIDE_CONSOLE
    if (!FreeConsole())
    {
        std::cout << "Could not detach from console." << std::endl;
    }
#endif
    while (TRUE) {
        // https://stackoverflow.com/a/14407301
        BitBlt(hDC_Desktop, 0, 0, rect.right - rect.left, rect.bottom - rect.top, TargetDC, 0, 0, SRCCOPY);
        Sleep(1000 / MIRROR_FPS);
    }

    return 0;
}
