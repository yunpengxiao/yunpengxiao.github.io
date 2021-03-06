//
// Minimal Layered Window Sample
// by BOT Man, 2017
//

#include <windows.h>
#include <gdiplus.h>  // PNG Image Support
#pragma comment (lib, "gdiplus.lib")

#define DEFAULT_FILENAME L"texture.png"
#define CLASSNAME L"BOT Man - Layered Window Sample"

bool CreateLayeredWindow (LPCWSTR imageName)
{
    // - Create window with 'WS_EX_LAYERED' flag

    HWND hwnd = CreateWindowExW (
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASSNAME, CLASSNAME, WS_POPUP, 100, 100, 0, 0,
        nullptr, nullptr, (HINSTANCE) GetCurrentProcess (), nullptr);
    if (!hwnd) return false;

    // - Load image by GDI+

    Gdiplus::Image image (imageName);
    if (GetLastError () != ERROR_SUCCESS) return false;

    // - Create in-memory DC with 32bit-pixel bitmap

    HDC hdcWnd = GetDC (hwnd);
    HDC hdcMem = CreateCompatibleDC (hdcWnd);
    HBITMAP hBitmap = CreateCompatibleBitmap (
        hdcWnd, image.GetWidth (), image.GetHeight ());
    SelectObject (hdcMem, hBitmap);

    BITMAP bitmap = { 0 };
    GetObject (hBitmap, sizeof BITMAP, (void *) &bitmap);

    if (!hdcWnd || !hdcMem || !hBitmap ||
        bitmap.bmBitsPixel < 32)  // PixelFormat32bppARGB
        return false;

    // - Draw image to bitmap by GDI+

    Gdiplus::Graphics graphics (hdcMem);
    if (graphics.DrawImage (&image, 0, 0) != Gdiplus::Ok)
        return false;

    // - Update layered window with in-memory DC and Show

    SIZE size { (LONG) image.GetWidth (), (LONG) image.GetHeight () };
    POINT point = { 0, 0 };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    if (!UpdateLayeredWindow (hwnd, nullptr, nullptr, &size,
                              hdcMem, &point, 0, &blend, ULW_ALPHA))
        return false;
    ShowWindow (hwnd, SW_SHOW);

    // Note: this example is non-resource-safe...
    DeleteObject (hBitmap);
    DeleteDC (hdcMem);
    ReleaseDC (hwnd, hdcWnd);
    return true;
}

int WINAPI wWinMain (HINSTANCE, HINSTANCE, LPWSTR, int)
{
    auto wndProc =
        [] (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
    {
        switch (msg)
        {
        case WM_LBUTTONUP:
            PostQuitMessage (0);
            return 0;
        }
        return DefWindowProc (hwnd, msg, wParam, lParam);
    };

    WNDCLASSW wndClass = { 0 };
    wndClass.hCursor = LoadCursor (nullptr, IDC_HAND);
    wndClass.hInstance = (HINSTANCE) GetCurrentProcess ();
    wndClass.lpfnWndProc = wndProc;
    wndClass.lpszClassName = CLASSNAME;
    RegisterClassW (&wndClass);

    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup (&gdiplusToken, &gdiplusStartupInput, nullptr);

    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW (GetCommandLineW (), &argc);
    if (!CreateLayeredWindow ((argc >= 2) ? argv[1] : DEFAULT_FILENAME)) return 1;

    Gdiplus::GdiplusShutdown (gdiplusToken);

    MSG msg;
    while (GetMessage (&msg, nullptr, 0, 0))
    {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }
    return (int) msg.wParam;
}
