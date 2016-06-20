
/*

Ripple Test - by Napalm
You may use all or any part of the following code as long as you agree
to the Creative Commons Attribution 2.0 UK: England & Wales license.
http://creativecommons.org/licenses/by/2.0/uk/

*/

/*
:: -->> Ripple Water Effect <<-- ::

Ripple_Register(hInstance); //in win main

if(hImage) //in wm-create or wm-init
{
	GetObject(hImage, sizeof(bm), &bm);
	Ripple_Create(0, 0, bm.bmWidth, bm.bmHeight, 101, hWnd, hInst, hImage);
	DeleteObject(hImage);
}
*/

#define _WIN32_WINNT            0x0501
#define _WIN32_IE               0x0700

#include <windows.h>
#include <commctrl.h>
#include <assert.h>
#include "ripple.h"

static LPTSTR lpszRippleClassName =  TEXT("RippleImage");
LRESULT CALLBACK Ripple_Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID  Ripple_Drop  (LPRIPPLE lpInstance, INT nX, INT nY, INT nRadius, INT nNewHeight);
DWORD Ripple_Pixel (DWORD dwColor, INT nShift);
VOID  Ripple_Update(LPRIPPLE lpInstance);
VOID  Ripple_Clear (LPRIPPLE lpInstance);
VOID  Ripple_Free  (LPRIPPLE lpInstance);


BOOL Ripple_Register(HINSTANCE hInst) // by Napalm
{
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(WNDCLASSEX));
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_PARENTDC;
    wcex.hInstance     = hInst;
    wcex.lpszClassName = lpszRippleClassName;
    wcex.lpfnWndProc   = Ripple_Proc;
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.cbWndExtra    = sizeof(LONG);
    if(!RegisterClassEx(&wcex))
        return FALSE;
    return TRUE;
}

LPRIPPLE Ripple_Create(int nX, int nY, int nWidth, int nHeight,
                       WORD wID, HWND hWndParent, HINSTANCE hInst, HBITMAP hImage) // by Napalm
{
    HANDLE hHeap;
    LPRIPPLE lpInstance;


    hHeap = GetProcessHeap();
    lpInstance = (LPRIPPLE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(RIPPLE));
    if(!lpInstance)
        return NULL;

    lpInstance->dwPixelCount   = nWidth * nHeight;
    lpInstance->dwImageSize    = lpInstance->dwPixelCount * sizeof(DWORD);
    lpInstance->nHeightMap[0]  = (LPINT)  HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lpInstance->dwImageSize);
    lpInstance->nHeightMap[1]  = (LPINT)  HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lpInstance->dwImageSize);
    lpInstance->lpImageData[0] = (LPDWORD)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lpInstance->dwImageSize);
    lpInstance->lpImageData[1] = (LPDWORD)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lpInstance->dwImageSize);
    if( !lpInstance->nHeightMap[0]  || !lpInstance->nHeightMap[1] ||
            !lpInstance->lpImageData[0] || !lpInstance->lpImageData[1])
    {
        Ripple_Free(lpInstance);
        return NULL;
    }

    lpInstance->bmiFrame.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    lpInstance->bmiFrame.bmiHeader.biWidth       = nWidth;
    lpInstance->bmiFrame.bmiHeader.biHeight      = -nHeight;
    lpInstance->bmiFrame.bmiHeader.biPlanes      = 1;
    lpInstance->bmiFrame.bmiHeader.biBitCount    = 32;
    lpInstance->bmiFrame.bmiHeader.biCompression = BI_RGB;
    lpInstance->nWidth   = nWidth;
    lpInstance->nHeight  = nHeight;
    lpInstance->nDensity = 5; // <<
    lpInstance->nMap     = 0;

    if(hImage != NULL)
    {
        BITMAP bm;
        HDC hdc = CreateCompatibleDC(NULL);
        GetObject(hImage, sizeof(BITMAP), &bm);
        GetDIBits(hdc, hImage, 0, bm.bmHeight, lpInstance->lpImageData[0],
                  &lpInstance->bmiFrame, DIB_RGB_COLORS);
        DeleteDC(hdc);
    }

    lpInstance->hWnd = CreateWindowEx(0, lpszRippleClassName, NULL, WS_VISIBLE | WS_CHILD,
                                      nX, nY, nWidth, nHeight, hWndParent, (HMENU)wID, hInst, (LPVOID)lpInstance);
    if(lpInstance->hWnd == NULL)
    {
        Ripple_Free(lpInstance);
        return NULL;
    }

    return lpInstance;
}

LRESULT CALLBACK Ripple_Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) // by Napalm
{
    LPRIPPLE lpInstance = (LPRIPPLE)GetWindowLong(hWnd, 0);
    if(lpInstance || uMsg == WM_NCCREATE)
    {
        switch(uMsg)
        {
        case WM_NCCREATE:
            lpInstance = (LPRIPPLE)((LPCREATESTRUCT)lParam)->lpCreateParams;
            if(lpInstance == NULL)
                return FALSE;
            SetWindowLong(hWnd, 0, (LONG)lpInstance);
            Ripple_Update(lpInstance);
            lpInstance->uTimerId = SetTimer(hWnd, 1, 20, NULL);
            return TRUE;

        case WM_PAINT:
        {
            HDC hdc;
            if(wParam == 0)
                hdc = GetDC(hWnd);
            else hdc = (HDC)wParam;

            SetDIBitsToDevice(hdc, 0, 0, lpInstance->nWidth, lpInstance->nHeight,
                              0, 0, 0, lpInstance->nHeight, lpInstance->lpImageData[1],
                              &lpInstance->bmiFrame, DIB_RGB_COLORS);

            if(wParam == 0)
                ReleaseDC(hWnd, hdc);
            ValidateRect(hWnd, NULL);
        }
        return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_TIMER:
            Ripple_Update(lpInstance);
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;

        case WM_MOUSEMOVE:
            Ripple_Drop(lpInstance, (INT)LOWORD(lParam), (INT)HIWORD(lParam), 5, 50);
            return 0;

        case WM_LBUTTONDOWN:
            Ripple_Drop(lpInstance, (INT)LOWORD(lParam), (INT)HIWORD(lParam), 25, 500);
            return 0;

        case WM_NCDESTROY:
            KillTimer(hWnd, lpInstance->uTimerId);
            Ripple_Free(lpInstance);
            return 0;
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

VOID Ripple_Drop(LPRIPPLE lpInstance, INT nX, INT nY, INT nRadius, INT nNewHeight) // by Napalm
{
    int nRadiusQuad, nCX, nCY, nCYQuad;
    LPINT lpnNew = lpInstance->nHeightMap[!lpInstance->nMap];
    RECT rc = { -nRadius, -nRadius, nRadius, nRadius };

    if(nX - nRadius < 1)
        rc.left   -= (nX - nRadius - 1);
    if(nY - nRadius < 1)
        rc.top    -= (nY - nRadius - 1);
    if(nX + nRadius > lpInstance->nWidth  - 1)
        rc.right  -= (nX + nRadius - lpInstance->nWidth + 1);
    if(nY + nRadius > lpInstance->nHeight - 1)
        rc.bottom -= (nY + nRadius - lpInstance->nHeight + 1);

    nRadiusQuad = nRadius * nRadius;
    for(nCY = rc.top; nCY < rc.bottom; nCY++)
    {
        nCYQuad = nCY * nCY;
        for(nCX = rc.left; nCX < rc.right; nCX++)
        {
            if(nCX * nCX + nCYQuad < nRadiusQuad)
                lpnNew[lpInstance->nWidth * (nY + nCY) + (nX + nCX)] += nNewHeight;
        }
    }
}

inline DWORD Ripple_Pixel(DWORD dwColor, INT nShift) // by Napalm
{
    INT nR = ((dwColor      ) & 0xFF) - nShift;
    INT nG = ((dwColor >>  8) & 0xFF) - nShift;
    INT nB = ((dwColor >> 16) & 0xFF) - nShift;
    return RGB(
               ((nR < 0x00) ? 0x00 : (nR > 0xFF) ? 0xFF : nR),
               ((nG < 0x00) ? 0x00 : (nG > 0xFF) ? 0xFF : nG),
               ((nB < 0x00) ? 0x00 : (nB > 0xFF) ? 0xFF : nB));
}

VOID Ripple_Update(LPRIPPLE lpInstance) // by Napalm
{
    INT nX, nY, nDX, nDY, nNewHeight, nOffset, nSource, nCount;
    LPDWORD lpSrcImage = lpInstance->lpImageData[0];
    LPDWORD lpDstImage = lpInstance->lpImageData[1];
    LPINT   lpnNew     = lpInstance->nHeightMap[ lpInstance->nMap];
    LPINT   lpnOld     = lpInstance->nHeightMap[!lpInstance->nMap];
    INT     nWidth     = lpInstance->nWidth;
    INT     nHeight    = lpInstance->nHeight;

    nOffset = lpInstance->nWidth + 1;
    nCount  = lpInstance->dwPixelCount;

    for(nY = (nHeight - 1) * nWidth; nOffset < nY; nOffset += 2)
    {
        for(nX = nOffset + nWidth - 2; nOffset < nX; nOffset++)
        {
            nDX = lpnNew[nOffset] - lpnNew[nOffset + 1];
            nDY = lpnNew[nOffset] - lpnNew[nOffset + nWidth];
            nSource = nOffset + nWidth * (nDY >> 3) + (nDX >> 3);
            if(nSource < nCount && nSource > 0)
                lpDstImage[nOffset] = Ripple_Pixel(lpSrcImage[nSource], nDX);

            nOffset++;

            nDX = lpnNew[nOffset] - lpnNew[nOffset + 1];
            nDY = lpnNew[nOffset] - lpnNew[nOffset + lpInstance->nWidth];
            nSource = nOffset + nWidth * (nDY >> 3) + (nDX >> 3);
            if(nSource < nCount && nSource > 0)
                lpDstImage[nOffset] = Ripple_Pixel(lpSrcImage[nSource], nDX);
        }
    }

    nOffset = lpInstance->nWidth + 1;

    for(nY = (nHeight - 1) * nWidth; nOffset < nY; nOffset += 2)
    {
        for(nX = nOffset + nWidth - 2; nOffset < nX; nOffset++)
        {
            nNewHeight = (
                             ( lpnOld[nOffset + nWidth]
                               + lpnOld[nOffset - nWidth]
                               + lpnOld[nOffset + 1]
                               + lpnOld[nOffset - 1]
                               + lpnOld[nOffset - nWidth - 1]
                               + lpnOld[nOffset - nWidth + 1]
                               + lpnOld[nOffset + nWidth - 1]
                               + lpnOld[nOffset + nWidth + 1]
                             ) >> 2) - lpnNew[nOffset];
            lpnNew[nOffset] = nNewHeight - (nNewHeight >> lpInstance->nDensity);
        }
    }

    lpInstance->nMap = !lpInstance->nMap;
}

VOID Ripple_Clear(LPRIPPLE lpInstance) // by Napalm
{
    ZeroMemory(lpInstance->nHeightMap[0], lpInstance->dwImageSize);
    ZeroMemory(lpInstance->nHeightMap[1], lpInstance->dwImageSize);
}

VOID Ripple_Free(LPRIPPLE lpInstance) // by Napalm
{
    HANDLE hHeap = GetProcessHeap();
    if(!lpInstance->nHeightMap[0])
        HeapFree(hHeap, 0, lpInstance->nHeightMap[0]);
    if(!lpInstance->nHeightMap[1])
        HeapFree(hHeap, 0, lpInstance->nHeightMap[1]);
    if(!lpInstance->lpImageData[0])
        HeapFree(hHeap, 0, lpInstance->lpImageData[0]);
    if(!lpInstance->lpImageData[1])
        HeapFree(hHeap, 0, lpInstance->lpImageData[1]);
    HeapFree(hHeap, 0, lpInstance);
}

