#include <stdio.h>
#include <conio.h>
#include <Windows.h>

HMODULE dll;
bool(__cdecl* StartEngine)(char* key, char* hash);

void main()
{
	dll = LoadLibrary("Engine.dll");
	if (dll)
	{
		*(FARPROC*)&StartEngine = GetProcAddress(dll, "StartEngine");
	}
	if (StartEngine("TKTWMS", "80F1A5ABA6D4595757B479FB250364D2") == 0)
		printf("true");
	else
		printf("no");
	_getch();
}