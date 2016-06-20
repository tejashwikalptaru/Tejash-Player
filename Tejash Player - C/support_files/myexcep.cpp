#include <windows.h>
#include <winnt.h>
#include <stdio.h>
#include "resource.h"


int ShowExceptionDlg(EXCEPTION_POINTERS *ep);

EXCEPTION_RECORD CurrExceptionRecord;
CONTEXT CurrContext;

int ShowExceptionDlg(EXCEPTION_POINTERS *ep)
{
	memcpy(&CurrExceptionRecord, ep->ExceptionRecord, sizeof(EXCEPTION_RECORD));
	memcpy(&CurrContext, ep->ContextRecord, sizeof(CONTEXT));

	//return 1 in order to continue to run the program after the exception handling.			
	return 1;
}
