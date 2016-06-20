/*

:: -->> Ripple Water Effect <<-- :: 

Ripple_Register(hInstance); //in win main

if(hImage) //in wm-create or wm-init
{
	GetObject(hImage, sizeof(bm), &bm);
	Ripple_Create(0, 0, bm.bmWidth, bm.bmHeight, 101, hWnd, hInst, hImage);
	DeleteObject(hImage);
}

Note :
Ripple Test - by Napalm
You may use all or any part of the following code as long as you agree 
to the Creative Commons Attribution 2.0 UK: England & Wales license.
http://creativecommons.org/licenses/by/2.0/uk/ 

*/

typedef struct _LPRIPPLE {
	BITMAPINFO	bmiFrame;
	DWORD		dwPixelCount;
	DWORD		dwImageSize;
	LPDWORD		lpImageData[2];
	LPINT		nHeightMap[2];
	INT			nWidth;
	INT			nHeight;
	INT			nDensity;
	INT			nMap;
	HWND		hWnd;
	UINT_PTR	uTimerId;
} RIPPLE, *LPRIPPLE;

BOOL Ripple_Register(HINSTANCE hInst);
LPRIPPLE Ripple_Create(INT nX, INT nY, INT nWidth, INT nHeight,
	WORD wID, HWND hWndParent, HINSTANCE hInst, HBITMAP hImage);