#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

constexpr COLORREF BG_COLOR = 0x00040E56;   // #560E04  (BGR order)
constexpr COLORREF TITLE_CLR = 0x00D7B017;   // #17B0D7  (BGR order)
constexpr LPCWSTR TITLE_TXT = L"Josh's PowerShell Copy";

class JoshShellWnd {
public:
    static int Run(HINSTANCE hInst, int nCmdShow);
private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    static void  Paint(HWND);
    static void  RunCommand(const std::wstring& cmd);
    static HWND  hwndEdit_, hwndStatic_;
};