#include "WinInterop.h"
#include "Assets/Resource.h"
#include "ProcessSwitching.h"
#include "Main.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <thread>

#define	WM_USER_SHELLICON WM_USER + 1

HINSTANCE g_instance;
NOTIFYICONDATA g_notifyData;
HMENU g_popMenu;
const char* g_title = "MinerManager";
const char* g_windowClass = "MinerManager";
const char* g_applicationToolTip = "Miner Manager App";
HICON g_icon;
BOOL bDisable = FALSE;							// keep application state

BOOL InitInstance(int32 nCmdShow)
{
    HWND windowHandle;

    windowHandle = CreateWindow(g_windowClass, g_title, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, g_instance, NULL);

    if (!windowHandle)
    {
        CreateErrorWindow("Error: Could not create window handle");
        return FALSE;
    }
	g_icon = LoadIcon(g_instance, MAKEINTRESOURCE(ICON_ID));

    g_notifyData.cbSize = sizeof(NOTIFYICONDATA); // sizeof the struct in bytes 
    g_notifyData.hWnd = (HWND)windowHandle;              //handle of the window which will process this app. messages 
    g_notifyData.uID = ICON_ID;           //ID of the icon that willl appear in the system tray 
    g_notifyData.uFlags = NIF_MESSAGE | NIF_ICON;
    g_notifyData.hIcon = g_icon;
    g_notifyData.uCallbackMessage = WM_USER_SHELLICON;
    Shell_NotifyIcon(NIM_ADD, &g_notifyData);

    return TRUE;
}

#include "shellapi.h"
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int32 wmEvent;
    switch (message)
    {
    case WM_USER_SHELLICON: 
        // systray msg callback 
            //Right Mouse Button Event:
        if (LOWORD(lParam) == WM_RBUTTONDOWN)
        {   
            POINT clickLocation;
            GetCursorPos(&clickLocation);
            g_popMenu = CreatePopupMenu();
            if (g_popMenu == NULL)
            {
                DWORD error = GetLastError();
                CreateErrorWindow(ToString("Could not create popup menu for MinerManger SysTray, error: %i", error).c_str());
            }

            UINT basicFlags = MF_BYPOSITION | MF_STRING;
            UINT StartButtonFlag = basicFlags;
            UINT StopButtonFlag = basicFlags;
            if (g_updating)
            {
                StartButtonFlag |= MF_GRAYED;
            }
            else
            {
                StopButtonFlag |= MF_GRAYED;
            }

            if (g_gameDetected.load() && g_updating)
                InsertMenu(g_popMenu, 0xFFFFFFFF, basicFlags | MF_DISABLED, MENU_STATUS, "Detected");
            else
                InsertMenu(g_popMenu, 0xFFFFFFFF, basicFlags | MF_DISABLED, MENU_STATUS, "Not Detected");
            InsertMenu(g_popMenu, 0xFFFFFFFF, MF_SEPARATOR,     MENU_SEP, "SEP");
            InsertMenu(g_popMenu, 0xFFFFFFFF, StartButtonFlag,  MENU_START,  "Start");
            InsertMenu(g_popMenu, 0xFFFFFFFF, StopButtonFlag,   MENU_STOP,   "Stop");
            InsertMenu(g_popMenu, 0xFFFFFFFF, MF_SEPARATOR,     MENU_SEP,    "SEP");
            InsertMenu(g_popMenu, 0xFFFFFFFF, basicFlags,       MENU_CONFIG, "Settings");
            InsertMenu(g_popMenu, 0xFFFFFFFF, MF_SEPARATOR,     MENU_SEP,    "SEP");
            InsertMenu(g_popMenu, 0xFFFFFFFF, basicFlags,       MENU_EXIT,   "Exit");

            SetForegroundWindow(hWnd);
            TrackPopupMenu(g_popMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, clickLocation.x, clickLocation.y, 0, hWnd, NULL);
            return TRUE;
        }
        else
        {
            break;
        }

        //Application Menu Event
    case WM_COMMAND:
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (LOWORD(wParam))
        {
        case MENU_START:
        {

            g_messageFromMainToProcess |= THREAD_MESSAGE_START;
            break;
        }
        case MENU_STOP:
        {

            g_messageFromMainToProcess |= THREAD_MESSAGE_STOP;
            break;
        }
        case MENU_CONFIG:
        {
            //not sure how to do this
#if 0
            HINSTANCE errorResult = ShellExecute(NULL, "open", "MinerManager.config", NULL, NULL, NULL);
            if ((INT_PTR)errorResult <= 32)
            {
                //ERROR_FILE_NOT_FOUND
                DWORD error = GetLastError();
                CreateErrorWindow(ToString("Could not open MinerManager.config, error: %i", error).c_str());
            }
#else
            system("MinerManager.config");
#endif
            break;
        }
        case MENU_EXIT:
        {

            g_messageFromMainToProcess |= THREAD_MESSAGE_EXIT;
            Shell_NotifyIcon(NIM_DELETE, &g_notifyData);
            DestroyWindow(hWnd);
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_DESTROY:
    {

        g_messageFromMainToProcess |= THREAD_MESSAGE_EXIT;
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int32 nCmdShow)
{
    //Register class
    g_instance = hInstance;
    {
#if 1
        HANDLE iconHandle = LoadImage(g_instance, MAKEINTRESOURCE(ICON_ID), IMAGE_ICON, 16, 16, LR_LOADTRANSPARENT);
#else
        HANDLE iconHandle = LoadImage(g_instance, "MinerManagerIcon.png", IMAGE_ICON, 32, 32, LR_LOADTRANSPARENT);
#endif
        
        if (iconHandle == NULL)
        {
            DWORD error = GetLastError();
            CreateErrorWindow("Could not load icon in _tWinMain");
        }

        WNDCLASSEX wcex;

        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = (HICON)iconHandle;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);//+ 1
        wcex.lpszMenuName = g_windowClass;
        wcex.lpszClassName = g_windowClass;
        wcex.hIconSm = NULL;

        RegisterClassEx(&wcex);
    }


    // Perform application initialization:
    if (!InitInstance(nCmdShow))
    {
        return FALSE;
    }

    //HACCEL accelTable = LoadAccelerators(hInstance, g_windowClass);
    std::thread processThread = std::thread(&ThreadMain, nullptr);
    SetThreadName(processThread.native_handle(), "Process Thread");

    MSG msg;
    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        //if (!TranslateAccelerator(msg.hwnd, accelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    processThread.join();


    return (int) msg.wParam;
}
