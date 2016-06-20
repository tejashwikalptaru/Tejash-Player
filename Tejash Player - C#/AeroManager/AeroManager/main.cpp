#include <Windows.h>
#include "resource.h"

HMODULE hDwmDLL;
BOOL enabled = FALSE;
DWORD special = 0;

typedef struct COLORIZATIONPARAMS
{
	COLORREF         clrColor;		   //ColorizationColor
	COLORREF         clrAftGlow;	   //ColorizationAfterglow
	UINT             nIntensity;	   //ColorizationColorBalance -> 0-100
	UINT             clrAftGlowBal;    //ColorizationAfterglowBalance
	UINT			 clrBlurBal;       //ColorizationBlurBalance
	UINT			 clrGlassReflInt;  //ColorizationGlassReflectionIntensity
	BOOL             fOpaque;
}DWMColor;

DWMColor dwmcolor, backup;

// Needed Function from DWM Library
HRESULT (WINAPI *DwmIsCompositionEnabled)(BOOL *pfEnabled);
HRESULT (WINAPI *DwmSetColorizationParameters) (COLORIZATIONPARAMS *colorparam,UINT unknown);
HRESULT (WINAPI *DwmGetColorizationParameters) (COLORIZATIONPARAMS *colorparam);


BOOL InitAeroManager()
{
	hDwmDLL = LoadLibrary("dwmapi.dll"); // Loads the DWM DLL
	if(!hDwmDLL)
		return FALSE;
	else
	{
		// Everything is fine upto here, we can get function address
		*(FARPROC *)&DwmIsCompositionEnabled = GetProcAddress(hDwmDLL,"DwmIsCompositionEnabled");
		//Below two functions are undocumented thus load from there index
		*(FARPROC *)&DwmGetColorizationParameters = GetProcAddress(hDwmDLL,(LPCSTR)127);
		*(FARPROC *)&DwmSetColorizationParameters = GetProcAddress(hDwmDLL,(LPCSTR)131);
	}
	if(!DwmIsCompositionEnabled && !DwmGetColorizationParameters && !DwmSetColorizationParameters) // The undocumented functions are not there,sorry
		return FALSE;
	else
	{
		DwmIsCompositionEnabled(&enabled);//Need to call DwmIsCompositionEnabled before calling the DwmSetColorizationParameters or it will fail.
		if(!enabled) // If glass composition is not enabled then close up
			return FALSE;
		else
		{
			DwmGetColorizationParameters(&backup); // Get current values
			return TRUE;
		}
	}
}

void SetAeroColor(long level)
{
	DwmGetColorizationParameters(&dwmcolor); // Get current values
	special = (dwmcolor.clrAftGlow * HIWORD(dwmcolor.clrColor));

	level = ((HIWORD(level) + LOWORD(level)) / 2) / 128;
	if(level == 256)
		level = 255;
	if(level < 128)
		dwmcolor.clrColor = special + RGB(0, 128 + level, level * 2);
	else
		dwmcolor.clrColor = special + RGB(0, 256 - (level - 127) * 2, 255);
	dwmcolor.clrAftGlow = dwmcolor.clrColor;
	DwmSetColorizationParameters(&dwmcolor,0);
}

void Restore()
{
	DwmSetColorizationParameters(&backup,0);
}