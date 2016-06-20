//Disabling warnings for a clean build output
#pragma warning(disable : 4996)
#pragma warning(disable : 4005)
#pragma warning(disable : 4244)
#pragma warning(disable : 4715)
#pragma warning(disable : 4805)
#pragma warning(disable : 4101)
#pragma warning(disable : 4018)


#include <commctrl.h>
#include <stdlib.h>
#include <assert.h>
#include <shlobj.h>
#include <objbase.h>
#include <shellapi.h>
#include <math.h>
#include <io.h>
#include "search_n_list.h"
#pragma comment(lib,"comctl32.lib")

// Function to allow single instance of this app.
#include "LimitSingleInstance.h"

// Bass Module
#include <bass.h>
#pragma comment(lib,"bass.lib")



// Napalm's Ripple effect function
#include "Ripple.cpp"

//Exception handling
#include "myexcep.cpp"

// Tool tip class
#include "gToolTip.cpp"

#include <bass_sfx.h>
#pragma comment(lib,"bass_sfx.lib")

// SkinCrafter
#include "CSCSkin.cpp"
CSCSkin skin;

#include "MP3FileInfo.cpp"
MP3FileInfo mp3fi;

// _ftol2 error fix
#if NDEBUG 
extern "C" long _ftol(double);
extern "C" long _ftol2(double x) { return _ftol(x); } 
#endif


// GDI+
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib,"GdiPlus.lib")
Bitmap *gpBitmap=0;	// The bitmap for displaying an image
Bitmap *bmp=0;
ULONG_PTR gdiplusToken=0;
bool no_image=false;

// ID3Lib
#define ID3LIB_LINKOPTION 3
#include "id3.h"
#include "id3/globals.h"
#include "id3/tag.h"
#pragma comment(lib,"id3lib.lib")
bool AlbumArt=false;


// Some defines for Tray
#define ID_TRAY_TIC                    9001
#define ID_TRAY_MAXIMIZE               9002
#define ID_TRAY_ABOUT                  9003
#define ID_TRAY_EXIT				   9004
#define ID_TRAY_PLAYLIST			   9008
#define WM_TRAYICON                   ( WM_USER + 1 )
// Some other define

#define ID_TIMER	9005
#define ID_TIMER2   9006
#define ID_TIMER3	9007
#define ID_TIMER4	9010
#define ID_TIMER5	9011
#define RADIO_STOP	9009

//HOT keys
#define IDH_PLAYPAUSE 9012
#define IDH_NEXT 9013
#define IDH_PREV 9014
#define IDH_PLAYLIST 9015
#define IDH_GOTOTARCK 9016
#define IDH_SHOWME	9017

char CmdLine[1024]={0};
BOOL PlayFlag=false;
OPENFILENAME ofn={0};
float p=100.f;
int enable=1,disable=0;
bool trackrem=false;
bool visible=true;
HMENU menu=0;
BOOL stop=0;
HMENU hMenu=0;
HWND ghwnd=0;
HWND rechwnd=0;
HWND nethwnd=0;
HWND proc_list=0;
HWND pl=0;
HWND playlist_hwnd=0;
HWND fxhwnd=0;
HANDLE thrhandle2=0;
HANDLE thrHandle3=0;
HANDLE thrhandle4=0;
HANDLE thrhandle5=0;
HINSTANCE hIns=0;
HMENU g_menu=0;
HDROP hdrp=0;
NOTIFYICONDATA notifyicondata;
char timebuff[50]={0};
char recent[100]={0};
int index=0;
int currentindex=0;
char row[10]={0};
bool preonce=false;
bool samesong=false;
int precurrentindex=0;
HWND OldHwnd=0;
COPYDATASTRUCT cds;
COPYDATASTRUCT* rcds;
bool previousasked=false;
bool notify=true;
char fldr[MAX_PATH]={0};
char lastsong[MAX_PATH]={0};
int temp=0;
DWORD channel=0;
char chantime[50]={0};
bool MonitorTrack=false;
bool PlayList=false;
char localdir[MAX_PATH]={0};
bool SngError=false;
bool playtemp=false;
bool createplay=false;
char buffer[MAX_PATH]={0};
char buffer2[MAX_PATH]={0};
int N=0,Nm=0;
char tempbuf[1024]={0};
int MaxIndex={0};
bool NoPlay=false;
bool Parsed=false;
HFX fx[4];	// 3 eq bands + reverb
BASS_DX8_PARAMEQ pdx;
bool settings_on=false;
char lastsongs[MAX_PATH]={0};
int lastindex=1;
bool pause=false;
DWORD leng=0;
bool lastfirst=false;
OFSTRUCT info={0};
char bad_song_name[30]={0};
int wait=0;
bool next=false;
char file[20]={0};
RECT rect,rc;
HSFX hSFX=0,hsfx=0;
int Vis=0,count=0;
bool alreadyplaylist=false;
HSYNC hsync=0;
bool once_fx_set=false;
int bass=0,vocal=0,trebal=0,reverb=0;
char settings_path[MAX_PATH]={0};
char settings_path2[MAX_PATH]={0};
char playlist_path[MAX_PATH]={0};
char dump[MAX_PATH]={0};
bool alreadygoto=false;
bool newplay=true;
char temp_playlist[MAX_PATH]={0};

//Tags...
TAG_ID3 *id3;
char title[1024]={0};
char artist[1024]={0};
char album[1024]={0};
char year[100]={0};
char comment[1024]={0};



//Record defines
#define BUFSTEP 200000	// memory allocation unit
int input=0;				// current input source
char *recbuf=NULL;		// recording buffer
DWORD reclen=0;			// recording length
HRECORD rchan=0;		// recording channel
HSTREAM chan=0;			// playback channel

// Net radio
DWORD cthread=0;

const char *urls[10]={ // preset stream URLs
	"http://www.radioparadise.com/musiclinks/rp_32.m3u","http://www.radioteentaal.com/masala128.m3u",
	"http://desimusicmix.com:8000/HQ","http://208.115.222.206:9998","http://76.73.126.218:80","http://205.188.215.230:8024"};

char proxy[100]=""; // proxy server
char *url=0;

BOOL CALLBACK DlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
void Handler(HWND hWnd,WPARAM wParam,LPARAM lParam);
void PlaySong(HWND hWnd,char Song[]);
void Open(HWND hWnd,char CmdLine[]);
void GetIDTag(HWND hWnd,char Song[]);
void SetPopUpMenuItem(HWND hWnd);
BOOL CALLBACK AboutProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
void GoToTray(bool flag);
void InitTray();
int PlayPrevious();
int PlayNext();
void GetNamefromPath(char Song[],char title[]);
BOOL GetFolderSelection(HWND hWnd, LPTSTR szBuf, LPCTSTR szTitle);
int PrepareList(char fldr[]);
void PreparePlayList();
BOOL CALLBACK FxDialog(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
void OpenPlayList(HWND hWnd,char CmdLine[]);
void ParsePlayList(char Path[]);
void UpdateFX(int b,HWND hWnd);
bool PlayListDB();
BOOL CALLBACK RecProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
void UpdateInputInfo(HWND hWnd);
void ReInitBass();
void StartRecording(HWND hWnd);
void StopRecording(HWND hWnd);
BOOL CALLBACK RecordingCallback(HRECORD handle, const void *buffer, DWORD length, void *user);
void SaveRecord(HWND hWnd);
BOOL CALLBACK NetProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
void __cdecl OpenURL();
void CALLBACK StatusProc(const void *buffer, DWORD length, void *user);
void CALLBACK EndSync(HSYNC handle, DWORD channel, DWORD data, void *user);
void CALLBACK MetaSync(HSYNC handle, DWORD channel, DWORD data, void *user);
void DoMeta();
void UpdateTime();
bool IsSkip(char* path);
bool IsMod(char path[]);
BOOL CALLBACK ShowPlaylistWindow(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
void AddColumn(HWND hView,char text[], int Col, int Width, DWORD dStyle);
void AddIndex(HWND hView, int Index);
void AddRow(HWND hView,char text[], int Index, int Col);
void ShowPlaylist();
char* GetName(char path[]);
void PlayByIndex(int index);
void AddaSong(HWND hWnd);
void RemoveTrack(char index[]);
void AddaFolder();
bool CanIPlay();
void InitDecoders();
void Visualization_Start(char* path);
BOOL CALLBACK FullVisProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
void RedrawSFXVisuals(HWND hWnd,char* path);
void CALLBACK MyMetaSyncProc(HSYNC handle, DWORD channel, DWORD data, void *user);
void SkinMe(char path[],int x);
void InitDATA();
void InvokeSkin(int x);
Gdiplus::Bitmap* ResizeClone(Bitmap *bmp, INT width, INT height,bool PreserveAspectRatio);
void GetAlbumArt(char* song);
BOOL CALLBACK GotoTrackProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
void LoadFileInResource(int name, int type, DWORD& size, const char*& data);
void UpdateFX2(int v,bool rev,int b);
void DeselectSkinTick();
void DeselectVisTick();