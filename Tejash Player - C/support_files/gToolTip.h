#define _WIN32_IE 0x0500
#include <windows.h>
#include "commctrl.h"
#include "TCHAR.h"


class gToolTip
{
public:
	static BOOL AddTip( HWND,HINSTANCE,TCHAR*,UINT,BOOL = FALSE);
	static void PutInTaskBar(HWND,HINSTANCE,HICON,UINT=INFINITE);
};
