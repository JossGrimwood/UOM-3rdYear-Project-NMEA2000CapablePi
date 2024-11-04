#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <chrono>
#include <iostream>
#include <cstdlib> // For EXIT_FAILURE
#include <iostream> // For std::cerr
#include <stdexcept>

// Global variables
NOTIFYICONDATA nid;

bool shouldExit = false;

HANDLE hProcess = NULL;

// Function to add an icon to the system tray
void AddNotificationIcon(HWND hWnd);

// Function to create and display the context menu
void ShowContextMenu(HWND hWnd);

// Main window procedure
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void StartExternalProcess() {
    STARTUPINFO si = {};
    PROCESS_INFORMATION pi = {};
    BOOL success = CreateProcess(
        NULL,                   // Module name
        "client.exe",  // Command line
        NULL,                   // Process handle not inheritable
        NULL,                   // Thread handle not inheritable
        FALSE,                  // Set handle inheritance to FALSE
        0,                      // No creation flags
        NULL,                   // Use parent's environment block
        NULL,                   // Use parent's starting directory
        &si,                    // Pointer to STARTUPINFO structure
        &pi                     // Pointer to PROCESS_INFORMATION structure
    );
    if (!success) {
        std::cerr << "Failed to start the external process." << std::endl;
    } else {
        hProcess = pi.hProcess;
        CloseHandle(pi.hThread);
    }
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MyHiddenWindowClass";
    RegisterClass(&wc);

    // Create main window
    HWND hWnd = CreateWindowEx(0, "MyHiddenWindowClass", "HiddenWindow", 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    
    StartExternalProcess();
    // Main message loop
    MSG msg;
    while (!shouldExit) {
        // Process pending messages, if any
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // Check if the process has exited
        if (hProcess != NULL) {
            DWORD exitCode;
            if (GetExitCodeProcess(hProcess, &exitCode) && exitCode != STILL_ACTIVE) {
                CloseHandle(hProcess);
                hProcess = NULL;
                
                // Check if the process exited with EXIT_FAILURE
                if (exitCode == EXIT_FAILURE) {
                    StartExternalProcess();
                }
            }
        }
        Sleep(100);
    }

    return 0;
}


void AddNotificationIcon(HWND hWnd) {
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;

    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName( NULL, buffer, MAX_PATH );

    #ifdef UNICODE
        int size = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
        std::string path(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &path[0], size, nullptr, nullptr);
        path;
    #else
        std::string path;
        path = std::string(buffer);
    #endif
    std::string image = path.substr(0, path.find_last_of("\\"));
    image = image.substr(0, image.find_last_of("\\")) + "\\images\\logo.ico";
     
    //std::cout << image << std::endl;
    nid.hIcon = (HICON)LoadImage(NULL, image.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    if (nid.hIcon == NULL) {
        std::cerr << "Failed to load icon." << std::endl;
        // You may want to use a default icon here if loading fails
        nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    strcpy(nid.szTip,  "NMEA2000client");

    Shell_NotifyIcon(NIM_ADD, &nid);
}

void ShowContextMenu(HWND hWnd) {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, 1, "Exit");

    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
    DestroyMenu(hMenu);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            AddNotificationIcon(hWnd);
            break;
        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
        case WM_USER + 1: // Handle notification icon messages
            switch (LOWORD(lParam)) {
                case WM_RBUTTONUP: // Right-click on the notification icon
                    ShowContextMenu(hWnd);
                    break;
                default:
                    break;
            }
            break;
        case WM_COMMAND: // Handle menu commands
            switch (LOWORD(wParam)) {
            case 1: // Exit command
                // Close the external process
                if (hProcess != NULL) {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                    hProcess = NULL; // Reset hProcess after closing the handle
                }
                // Close the main window
                shouldExit = true;
                DestroyWindow(hWnd);
                break;
            default:
                break;
            }
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

