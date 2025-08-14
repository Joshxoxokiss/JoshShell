#include "JoshShell.h"
#include <commctrl.h>
#include <process.h>

#pragma comment(lib, "comctl32.lib")

HWND JoshShellWnd::hwndEdit_ = nullptr;
HWND JoshShellWnd::hwndStatic_ = nullptr;

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nCmdShow)
{
    InitCommonControls();
    return JoshShellWnd::Run(hInst, nCmdShow);
}

int JoshShellWnd::Run(HINSTANCE hInst, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"JoshShellWindow";
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(BG_COLOR);
    RegisterClassW(&wc);

    // Calculate proper window size to account for borders and title bar
    RECT rc = { 0, 0, 800, 500 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, TITLE_TXT,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

void JoshShellWnd::Paint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rc; GetClientRect(hwnd, &rc);
    rc.bottom = 40;   // top banner height
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, TITLE_CLR);

    HFONT hFont = CreateFontW(22, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH, L"Segoe UI");
    HFONT old = (HFONT)SelectObject(hdc, hFont);
    DrawTextW(hdc, TITLE_TXT, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, old);
    DeleteObject(hFont);
    EndPaint(hwnd, &ps);
}

void JoshShellWnd::RunCommand(const std::wstring& cmd)
{
    SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };
    HANDLE hRead, hWrite;
    CreatePipe(&hRead, &hWrite, &sa, 0);
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;

    std::wstring full = L"cmd.exe /c " + cmd;
    if (CreateProcessW(nullptr, const_cast<wchar_t*>(full.c_str()),
        nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
        nullptr, nullptr, &si, &pi))
    {
        CloseHandle(hWrite);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        DWORD read;
        char buf[4096];
        std::wstring output;
        while (ReadFile(hRead, buf, sizeof(buf) - 1, &read, nullptr) && read) {
            buf[read] = 0;
            int wlen = MultiByteToWideChar(CP_OEMCP, 0, buf, read, nullptr, 0);
            std::wstring part(wlen, 0);
            MultiByteToWideChar(CP_OEMCP, 0, buf, read, &part[0], wlen);
            output += part;
        }
        CloseHandle(hRead);

        int len = GetWindowTextLengthW(hwndEdit_);
        SendMessageW(hwndEdit_, EM_SETSEL, len, len);
        SendMessageW(hwndEdit_, EM_REPLACESEL, FALSE, (LPARAM)output.c_str());
    }
}

LRESULT CALLBACK JoshShellWnd::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        const int top = 45;

        // Get actual client area size for proper control positioning
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        int clientWidth = clientRect.right - clientRect.left;
        int clientHeight = clientRect.bottom - clientRect.top;

        hwndEdit_ = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"EDIT", nullptr,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            10, top, clientWidth - 20, clientHeight - top - 40, hwnd, nullptr, GetModuleHandle(nullptr), nullptr);

        hwndStatic_ = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"EDIT", nullptr,
            WS_CHILD | WS_VISIBLE | ES_LEFT,
            10, clientHeight - 30, clientWidth - 100, 25, hwnd, (HMENU)2, GetModuleHandle(nullptr), nullptr);

        CreateWindowExW(
            0, L"BUTTON", L"Run",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            clientWidth - 85, clientHeight - 30, 75, 25,
            hwnd, (HMENU)1, GetModuleHandle(nullptr), nullptr);

        // Set focus to input field
        SetFocus(hwndStatic_);
        return 0;
    }
    case WM_SIZE: {
        // Handle window resizing - reposition controls
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        int clientWidth = clientRect.right - clientRect.left;
        int clientHeight = clientRect.bottom - clientRect.top;
        const int top = 45;

        if (hwndEdit_) {
            SetWindowPos(hwndEdit_, nullptr, 10, top, clientWidth - 20, clientHeight - top - 40,
                SWP_NOZORDER | SWP_NOACTIVATE);
        }
        if (hwndStatic_) {
            SetWindowPos(hwndStatic_, nullptr, 10, clientHeight - 30, clientWidth - 100, 25,
                SWP_NOZORDER | SWP_NOACTIVATE);
        }
        // Reposition Run button
        HWND hButton = GetDlgItem(hwnd, 1);
        if (hButton) {
            SetWindowPos(hButton, nullptr, clientWidth - 85, clientHeight - 30, 75, 25,
                SWP_NOZORDER | SWP_NOACTIVATE);
        }
        return 0;
    }
    case WM_PAINT:
        Paint(hwnd);
        return 0;
    case WM_COMMAND:
        if (LOWORD(wp) == 1 || LOWORD(wp) == 2) { // Run button or Enter key in edit control
            if (LOWORD(wp) == 2 && HIWORD(wp) != EN_CHANGE) {
                // Only process if it's not just a text change notification
                break;
            }
            int len = GetWindowTextLengthW(hwndStatic_);
            if (len > 0) {
                std::wstring cmd(len + 1, 0);
                GetWindowTextW(hwndStatic_, &cmd[0], len + 1);
                cmd.resize(len);
                RunCommand(cmd);
                SetWindowTextW(hwndStatic_, L"");
            }
        }
        return 0;
    case WM_KEYDOWN:
        // Handle Enter key in the input field
        if (wp == VK_RETURN && GetFocus() == hwndStatic_) {
            int len = GetWindowTextLengthW(hwndStatic_);
            if (len > 0) {
                std::wstring cmd(len + 1, 0);
                GetWindowTextW(hwndStatic_, &cmd[0], len + 1);
                cmd.resize(len);
                RunCommand(cmd);
                SetWindowTextW(hwndStatic_, L"");
            }
            return 0;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}