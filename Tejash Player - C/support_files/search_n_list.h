#pragma comment(lib, "shlwapi.lib")

#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

int NumOfSongs=0;
char row_n[100]={0};
char temp2[MAX_PATH]={0};
char OutputFile[MAX_PATH]={0};

typedef BOOL (WINAPI *RECURSIVEFILEPROC)(LPTSTR lpszFile, LPVOID lpParam);
ULONGLONG RecursiveDirectory(LPTSTR lpszPath, RECURSIVEFILEPROC lpCallback, LPVOID lpParam) // by Napalm
{
	WIN32_FIND_DATA WFD;
	ULONGLONG qwCount = 0;
	TCHAR szFileSpec[MAX_PATH + 1];

	PathCombine(szFileSpec, lpszPath, TEXT("*.*"));
	HANDLE hSearch = FindFirstFile(szFileSpec, &WFD);
	if(hSearch == INVALID_HANDLE_VALUE) return 0;
	do{
		if(_tcscmp(WFD.cFileName, TEXT("..")) && _tcscmp(WFD.cFileName, TEXT("."))){
			if(WFD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && !(WFD.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)){
				PathCombine(szFileSpec, lpszPath, WFD.cFileName);
				qwCount += RecursiveDirectory(szFileSpec, lpCallback, lpParam);
			}else{
				if(lpCallback != NULL){
					PathCombine(szFileSpec, lpszPath, WFD.cFileName);
					if(!lpCallback(szFileSpec, lpParam)) break;
				}
				qwCount++;
			}
		}
	}while(FindNextFile(hSearch, &WFD));
	FindClose(hSearch);
	return qwCount;
}

BOOL WINAPI rd_cb(TCHAR *file_path, VOID *param)
{
	TCHAR *exts;
	exts = (TCHAR*)param;
	while (*exts) {
		TCHAR *cur_ext;
		cur_ext = PathFindExtension(file_path);
		if (cur_ext) 
		{
			strlwr(cur_ext);
			if (_tcscmp(cur_ext, exts) == 0) 
			{
				/* do what you want here with the file */
				/* printf as an example here */
				//_tprintf(TEXT("%s\n"), file_path); 
				NumOfSongs++;
				ZeroMemory(row_n,sizeof row_n);
				wsprintf(row_n,"%d",NumOfSongs);
				ZeroMemory(temp2,sizeof temp2);
				lstrcat(temp2,file_path);
				WritePrivateProfileString("songs",row_n,temp2,OutputFile);
			}
		}
		++exts;
	}

	return TRUE;
}

void ext_search(TCHAR *root_dir, char *exts,char DirForIni[],bool reset)
{
	if(reset)
		NumOfSongs=0;
	ZeroMemory(OutputFile,sizeof OutputFile);
	lstrcpy(OutputFile,DirForIni);
	RecursiveDirectory(root_dir, rd_cb, (void *)exts);
}

int TotalSongs()
{
	return NumOfSongs;
}

void SetNumSongs(int Number)
{
	NumOfSongs=Number;
}