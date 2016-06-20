#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <windows.h>
#include "support_files\resource.h"
// Functions and other defines
#include "support_files\defines.h"

//#define COMMERCIAL

CLimitSingleInstance g_SingleInstanceObj(TEXT("Global\\{A74DEDDA-BEF5-4dca-9B66-05B0FD461E0E}"));

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
	// Checking and allowing only single instance...
	if(g_SingleInstanceObj.IsAnotherInstanceRunning())
	{
		OldHwnd=FindWindow(NULL,"Tejash Player");
		if(OldHwnd)
		{
			if(strlen(lpCmdLine))
			{
				cds.cbData = strlen(lpCmdLine)+1;
				cds.lpData = lpCmdLine;
				SendMessage(OldHwnd,WM_COPYDATA,(WPARAM)0,(LPARAM)&cds);
			}
			return 0;
		}
		else
		{
			OldHwnd=FindWindow(NULL,"Please wait, searching files...");
			if(OldHwnd)
			{
				if(strlen(lpCmdLine))
				{
					cds.cbData = strlen(lpCmdLine)+1;
					cds.lpData = lpCmdLine;
					SendMessage(OldHwnd,WM_COPYDATA,(WPARAM)0,(LPARAM)&cds);
				}
				return 0;
			}
		}
	}

	

	hIns=hInstance;
	InitCommonControls();

	// enable trackbar support
	INITCOMMONCONTROLSEX cc={sizeof(cc),ICC_BAR_CLASSES};
	InitCommonControlsEx(&cc);

	// Loading skin
	skin.Init();

#if _DEBUG
	MessageBox(0,"Running in debug mode\nDo not distribute this debug version","Warning",MB_ICONERROR);
#endif

#if _WIN64
	MessageBox(0,"You are running a 64 bit Windows\nThis version of Tejash Player is not compatible with 64 bit of Windows","Fatal Error",MB_ICONERROR);
	return 0;
#endif

	lstrcpy(CmdLine,lpCmdLine);
	if(lstrlen(CmdLine))
		PlayFlag=true;

	// Initialize GDI+
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	gpBitmap = NULL;

	// Register ripper for About box ripple effect
	Ripple_Register(hInstance);

	DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_DIALOG1),HWND_DESKTOP,DLGPROC(&DlgProc),NULL);
}

BOOL CALLBACK DlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch(message)
	{
	case WM_INITDIALOG:
		ZeroMemory(localdir,sizeof localdir);
		SHGetSpecialFolderPath(ghwnd,localdir,CSIDL_LOCAL_APPDATA,false);
		SHGetSpecialFolderPath(hWnd,recent,CSIDL_LOCAL_APPDATA,false);
		lstrcat(recent,"\\tejasplayer.ini");
		WritePrivateProfileString("Tejash Player Configuration file","About","Do not modify or edit this file.This file is used to store location of already played files.It will be deleted automatically when program exits",recent);
		ghwnd=hWnd;
		SetWindowText(hWnd,"Tejash Player");

		
		// Initialize default output...
		if(!BASS_Init(-1,44100,0,hWnd,NULL))
		{
			MessageBox(hWnd,"Can not initialize device !","Error",MB_ICONERROR);
			SendMessage(hWnd,WM_CLOSE,NULL,NULL);
		}
		
		if(!BASS_SFX_Init(hIns,hWnd))
		{
			MessageBox(hWnd,"Can not initialize SFX !","Error",MB_ICONERROR);
			SendMessage(hWnd,WM_CLOSE,NULL,NULL);
		}

		// Getting client rect
		GetClientRect(GetDlgItem(hWnd,ID_VIS),&rect);

		// Load all the plugins
		InitDecoders();

		InitDATA(); // Load Startup values and settings...
		
		InitTray();

		SendMessage(hWnd,WM_TIMER,ID_TIMER,0); // Shows the time instantly after loading dialogbox
		SetTimer(hWnd,ID_TIMER,1000,0);

		DragAcceptFiles(hWnd,true);


		// Setting ICON on dialogbox
		SendMessage(hWnd,WM_SETICON,1,(LPARAM)(LoadIcon(hIns,MAKEINTRESOURCE(IDI_ICON1))));

		// Tooltip init for buttons...
		gToolTip::AddTip(hWnd,hIns,"Play",IDC_BUTTON1,true);
		gToolTip::AddTip(hWnd,hIns,"Pause",IDC_BUTTON2,true);
		gToolTip::AddTip(hWnd,hIns,"Stop",IDC_BUTTON3,true);
		gToolTip::AddTip(hWnd,hIns,"Song details",IDC_BUTTON4,true);
		gToolTip::AddTip(hWnd,hIns,"Play previous track",IDC_BUTTON6,true);
		gToolTip::AddTip(hWnd,hIns,"Play next track",IDC_BUTTON7,true);
		gToolTip::AddTip(hWnd,hIns,"Volume bar",IDC_SLIDER1,true);
		gToolTip::AddTip(hWnd,hIns,"Seek bar",IDC_SLIDER2,true);
		gToolTip::AddTip(hWnd,hIns,"Mute sound",IDC_CHECK1,true);
		gToolTip::AddTip(hWnd,hIns,"Repeate playing track",IDC_CHECK2,true);
		gToolTip::AddTip(hWnd,hIns,"Repeate all songs in current playlist",IDC_LOOPALL ,true);
		gToolTip::AddTip(hWnd,hIns,"3D Surround sound (If HD Out is supported)",IDC_CHECK3,true);
		// If song is passed as argument, then play it...
		if(PlayFlag)
		{
			loop:	
			// Adjust CmdLine for next play, fixing a bug :)
			// If we play song via cmdline first run is good, but if we
			// stop song and play again, it will give error because a extra " adds
			// cmd line parameter, thus I am removing it here and fixing it
			char temp;
			int CmdLineLen=lstrlen(CmdLine);
			for(int i=1;i<CmdLineLen;i++)
			{
				temp=CmdLine[i];
				CmdLine[i-1]=temp;
			}
			CmdLine[CmdLineLen-2]='\0';
			if(CanIPlay())
			{
				if(PlayListDB())
					ParsePlayList(CmdLine);
				else
				{
					loop2:
					previousasked=false;
					playtemp=false;
					PlayList=false;
					PlaySong(hWnd,CmdLine);
				}
				//////////////////////////////////////////////
			}
		}

		// Registering Hot keys
		RegisterHotKey(hWnd,IDH_PLAYPAUSE,MOD_ALT,VK_SPACE); // PLAY PAUSE HOT KEY
		RegisterHotKey(hWnd,IDH_NEXT,MOD_ALT,VK_RIGHT); // Next
		RegisterHotKey(hWnd,IDH_PREV,MOD_ALT,VK_LEFT); // Prev
		RegisterHotKey(hWnd,IDH_PLAYLIST,MOD_ALT,VK_UP); // Playlist
		RegisterHotKey(hWnd,IDH_GOTOTARCK,MOD_ALT,0x47); // Goto Track
		RegisterHotKey(hWnd,IDH_SHOWME,MOD_ALT,0x53); // Show hide dialog
		return true;

	case WM_COPYDATA:
		ZeroMemory(CmdLine,sizeof CmdLine);
		rcds=(COPYDATASTRUCT*) lParam;
		lstrcpy(CmdLine,(LPSTR)rcds->lpData);
		goto loop;
		return true;
	case WM_TIMER:
		if(wParam==ID_TIMER)
		{
			// update position of slider (seek bar)
			SendDlgItemMessage(ghwnd,IDC_SLIDER2,TBM_SETPOS,1,(DWORD)BASS_ChannelBytes2Seconds(channel,BASS_ChannelGetPosition(channel,BASS_POS_BYTE)));
			if(!MonitorTrack)
				SetDlgItemText(hWnd,DEF_TIME,"0:00");
		}
		if(wParam==ID_TIMER5)
		{
			if(hSFX > 0)
			{
				HWND hWndVis = GetDlgItem(hWnd, ID_VIS);
				HDC hdc = GetDC(hWndVis);
				BASS_SFX_PluginRender(hSFX,channel, hdc);
				ReleaseDC(hWndVis, hdc);
			}
		}
		return true;

	case WM_DROPFILES:
		{
		ZeroMemory(CmdLine,sizeof CmdLine);
		hdrp=(HDROP)(HANDLE)wParam;
		DragQueryFile(hdrp,0,CmdLine,MAX_PATH);
		DragFinish(hdrp);
		if(CanIPlay())
		{
			if(PlayListDB())
					ParsePlayList(CmdLine);
			else
			{
				goto loop2;
			}
		}
		}
		return true;

	case WM_COMMAND:
		Handler(hWnd,wParam,lParam);
		return true;


	case WM_TRAYICON:
		{
			switch(wParam)
			{
			case ID_TRAY_TIC:
				break;
			}
			if(lParam == WM_LBUTTONUP)
			{
				GoToTray(false);
			}
			else if(lParam == WM_RBUTTONDOWN)
			{
				POINT curPoint ;
				GetCursorPos( &curPoint ) ;
				SetForegroundWindow(hWnd); 
				UINT clicked = TrackPopupMenu(g_menu,TPM_BOTTOMALIGN | TPM_CENTERALIGN |TPM_RETURNCMD ,curPoint.x,curPoint.y,0,ghwnd,NULL);
				if(clicked == ID_TRAY_MAXIMIZE)
					GoToTray(false);
				if(clicked == ID_TRAY_ABOUT)
					SendMessage(hWnd,WM_COMMAND,ABOUT_PLAYER,0);
				if(clicked == ID_TRAY_EXIT)
					SendMessage(hWnd,WM_CLOSE,0,0);
				if(clicked ==  ID_TRAY_PLAYLIST)
					SendMessage(hWnd,WM_COMMAND,VIEW_PLAYLIST,0);
			}
		}
		return true;

	case WM_HOTKEY:
		{
			switch(wParam)
			{
			case IDH_PLAYPAUSE:
				{
					if(pause==false)
					{
						if(stop==true)
						{
							SendMessage(hWnd,WM_COMMAND,IDC_BUTTON1,0);
							stop=false;
							break;
						}
						SendMessage(hWnd,WM_COMMAND,IDC_BUTTON2,0);
						pause=true;
						break;
					}
					if(pause==true)
					{
						SendMessage(hWnd,WM_COMMAND,IDC_BUTTON1,0);
						pause=false;
						break;
					}
				}
				break;
			case IDH_NEXT:
				SendMessage(hWnd,WM_COMMAND,IDC_BUTTON7,0);
				break;
			case IDH_GOTOTARCK:
				SendMessage(hWnd,WM_COMMAND,ID_GOTO_TRACK,0);
				break;
			case IDH_PREV:
				SendMessage(hWnd,WM_COMMAND,IDC_BUTTON6,0);
				break;
			case IDH_PLAYLIST:
				if(alreadyplaylist)
					ShowWindow(playlist_hwnd,SW_SHOWNORMAL);
				if(alreadyplaylist==false)
					SendMessage(hWnd,WM_COMMAND,VIEW_PLAYLIST,0);
				break;
			case IDH_SHOWME:
				{
					if(visible)
						GoToTray(true);
					else
						GoToTray(false);
				}
				break;
			}
		}
		return true;

	case WM_HSCROLL:
		if(GetDlgCtrlID((HWND)lParam)==IDC_SLIDER1)
		{
			if (lParam && LOWORD(wParam)!=SB_THUMBPOSITION && LOWORD(wParam)!=SB_ENDSCROLL)
			{
				p=SendMessage(HWND(lParam),TBM_GETPOS,0,0);
				BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL,float(p/100.f));
				if(IsDlgButtonChecked(hWnd,IDC_CHECK1) == BST_CHECKED)
					CheckDlgButton(hWnd,IDC_CHECK1,BST_UNCHECKED);

				// Ability to save current volume for next start...
				char* vols=new char[500];
				sprintf(vols,"%f",p);
				WritePrivateProfileString("volume","pos",vols,settings_path);
				delete [] vols;
			}
		}
		if(GetDlgCtrlID((HWND)lParam)==IDC_SLIDER2)
		{
			if (lParam && LOWORD(wParam)!=SB_THUMBPOSITION && LOWORD(wParam)!=SB_ENDSCROLL) 
			{	// set the position
				int pos=SendMessage((HWND)lParam,TBM_GETPOS,0,0);
				BASS_ChannelSetPosition(channel,BASS_ChannelSeconds2Bytes(channel,pos),BASS_POS_BYTE);
			}
		}
		return true;

	case WM_CTLCOLORBTN:
		return (BOOL)CreateSolidBrush(RGB(150,251,13));

	case WM_CLOSE:
		DeleteFile(recent);
		DeleteFile(temp_playlist);
		if(!Parsed)
			DeleteFile(buffer);
		BASS_PluginFree(0);
		BASS_Free();
		Shell_NotifyIcon(NIM_DELETE, &notifyicondata);
		DestroyMenu(g_menu);
		KillTimer(hWnd,ID_TIMER);
		mp3fi.Free();
		TerminateThread(thrhandle2,0);
		TerminateThread(thrHandle3,0);
		TerminateThread(thrhandle4,0);
		TerminateThread(thrhandle5,0);
		UnregisterHotKey(hWnd,IDH_PLAYLIST);
		UnregisterHotKey(hWnd,IDH_PLAYPAUSE);
		UnregisterHotKey(hWnd,IDH_NEXT);
		UnregisterHotKey(hWnd,IDH_PREV);
		UnregisterHotKey(hWnd,IDH_GOTOTARCK);
		UnregisterHotKey(hWnd,IDH_SHOWME);
		if(alreadyplaylist)
		{
			alreadyplaylist=false;
			SendMessage(playlist_hwnd,WM_CLOSE,0,0);
		}
		if(no_image)
		{
			if (gpBitmap) delete gpBitmap;
			if(bmp) delete bmp;
		}
		GdiplusShutdown(gdiplusToken);
		EndDialog(hWnd,0);
		return true;
	}
	return false;
}

void Handler(HWND hWnd,WPARAM wParam,LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case ABOUT_PLAYER:
		DialogBoxParam(hIns,MAKEINTRESOURCE(IDD_DIALOG2),hWnd,DLGPROC(&AboutProc),NULL);
		break;

	case EXIT:
		SendMessage(hWnd,WM_CLOSE,NULL,NULL);
		break;

	case OPEN:
		MessageBox(hWnd,"Searching in a folder or drive may take time\nPlease be patient while creating playlist\nI will tell you after creating","Thanks",MB_ICONINFORMATION);
		ZeroMemory(fldr,sizeof fldr);
		GetFolderSelection(hWnd,fldr,"Select a folder or drive containing music files");
		if(lstrlen(fldr))
		{
			SetWindowText(hWnd,"Please wait, searching files...");
			playtemp=true;
			NoPlay=false;
			TerminateThread(thrHandle3,0);
			Parsed=false;
			thrHandle3=CreateThread(0,0,(LPTHREAD_START_ROUTINE)PreparePlayList,0,0,0);
		}
		break;

	case IDC_CHECK1:
		if(IsDlgButtonChecked(hWnd,IDC_CHECK1)==BST_CHECKED)
			BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL,float(0));
		else
			BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL,float(p/100.f));
		break;

	case IDC_BUTTON1:
		if(lastindex ==0)
		{
			if(stop==true)
			{
				previousasked=false;
				PlaySong(hWnd,CmdLine);
				stop=false;
				EnableWindow(GetDlgItem(hWnd,IDC_BUTTON2),true);
			}
			else
			{
				BASS_Start();
				MonitorTrack=true;
			}
		}
		else
			if(pause)
			{
				BASS_Start();
				MonitorTrack=true;
				pause=false;
			}
			else
			{
				PlaySong(hWnd,lastsong);
			}
		break;

	case IDC_BUTTON2:
		BASS_Pause();
		pause=true;
		break;

	case IDC_BUTTON3:
		BASS_Stop();
		MonitorTrack=false;
		TerminateThread(thrhandle4,0);
		SendDlgItemMessage(hWnd,DEF_TIME,WM_SETTEXT,0,(LPARAM)"0:00");
		SendDlgItemMessage(hWnd,TRACK_TIME,WM_SETTEXT,0,(LPARAM)"0:00");
		SetDlgItemText(hWnd,IDC_EDIT1,title);
		stop=true;
		pause=false;
		EnableWindow(GetDlgItem(hWnd,IDC_BUTTON2),false);
		break;
	case IDC_BUTTON4:
		SetPopUpMenuItem(hWnd);
		break;
	case OPEN_FRM_MENU:
		memset(CmdLine,0,sizeof CmdLine);
		Open(hWnd,CmdLine);
		if(lstrlen(CmdLine))
		{
			if(PlayListDB())
			{
				lastindex=0;
				ParsePlayList(CmdLine);
			}
			else
			{
				previousasked=false;
				if(PlayList)
				{
					PlayList=false;
					playtemp=false;
					index=0;
					lastindex=0;
					NoPlay=false;
				}
				PlaySong(hWnd,CmdLine);
			}
		}
		break;
	case ID_TOOLS_MINIMIZETOTRAY:
		GoToTray(true);
		break;
	case IDC_BUTTON6:
		PlayPrevious();
		break;
	case IDC_BUTTON7:
		PlayNext();
		break;
	case ID_FX:
		BASS_Pause();
		DialogBoxParam(hIns,MAKEINTRESOURCE(IDD_DIALOG3),HWND_DESKTOP,DLGPROC(&FxDialog),NULL);
		break;
	case IDC_CHECK2:
		if(IsDlgButtonChecked(hWnd,IDC_CHECK2) == BST_CHECKED)
		{
			BASS_ChannelFlags(channel,BASS_SAMPLE_LOOP,BASS_SAMPLE_LOOP);
			CheckDlgButton(hWnd,IDC_LOOPALL,BST_UNCHECKED);
		}
		else
			BASS_ChannelFlags(channel, 0, BASS_SAMPLE_LOOP);
		break;
	case IDC_LOOPALL:
		if(IsDlgButtonChecked(hWnd,IDC_CHECK2) == BST_CHECKED)
		{
			CheckDlgButton(hWnd,IDC_CHECK2,BST_UNCHECKED);
			BASS_ChannelFlags(channel, 0, BASS_SAMPLE_LOOP);
		}
		break;
	case IDC_CHECK3:
		if(IsDlgButtonChecked(hWnd,IDC_CHECK3) == BST_CHECKED)
		{
			BASS_ChannelFlags(channel,BASS_MUSIC_SURROUND,BASS_MUSIC_SURROUND);
			BASS_ChannelFlags(channel,BASS_MUSIC_3D ,BASS_MUSIC_3D);
		}
		else
		{
			BASS_ChannelFlags(channel, 0, BASS_MUSIC_SURROUND);
			BASS_ChannelFlags(channel, 0, BASS_MUSIC_3D);
		}
		break;
	case ID_CREATE_PLAY:
		ZeroMemory(fldr,sizeof fldr);
		GetFolderSelection(hWnd,fldr,"Select a folder or drive containing music files");
		if(lstrlen(fldr))
		{
			SetWindowText(hWnd,"Please wait, searching files...");
			createplay=true;
			playtemp=false;
			NoPlay=true;
			TerminateThread(thrHandle3,0);
			thrHandle3=CreateThread(0,0,(LPTHREAD_START_ROUTINE)PreparePlayList,0,0,0);
		}
		break;
	case ID_OPEN_PLAY:
		memset(CmdLine,0,sizeof CmdLine);
		OpenPlayList(hWnd,CmdLine);
		if(lstrlen(CmdLine))
		{
			ParsePlayList(CmdLine);
		}
		break;
	case ID_RECORD:
		BASS_Stop();
		BASS_Free();
		ShowWindow(hWnd,SW_HIDE);
		DialogBoxParam(hIns,MAKEINTRESOURCE(IDD_DIALOG4),HWND_DESKTOP,DLGPROC(&RecProc),NULL);
		index--;
		break;
	case PLAY_ONLINE:
		BASS_ChannelPlay(0,0);
		SendMessage(hWnd,WM_COMMAND,IDC_BUTTON3,0);
		ShowWindow(hWnd,SW_HIDE);
		DialogBoxParam(hIns,MAKEINTRESOURCE(IDD_DIALOG5),HWND_DESKTOP,DLGPROC(&NetProc),NULL);
		break;
	case VIEW_PLAYLIST:
		if(alreadyplaylist)
			ShowWindow(playlist_hwnd,SW_SHOWNORMAL);
		if(!alreadyplaylist)
			DialogBoxParam(hIns,MAKEINTRESOURCE(IDD_DIALOG6),HWND_DESKTOP,DLGPROC(&ShowPlaylistWindow),0);
		break;
	case ID_UPDATE_CHECK:
		{
			char update_path[MAX_PATH]={0};
			GetModuleFileName(GetModuleHandle(NULL),update_path,MAX_PATH);
			char *ptr = strrchr(update_path,'\\');
			int m = ptr-update_path;
			int v=0;
			char FN[MAX_PATH]={0};
			for(v=0; v<m; v++)
			{
				FN[v] = update_path[v];
			}
			lstrcat(FN,"\\wyUpdate.exe");
			if(ShellExecute(hWnd,"OPEN",FN,0,0,SW_SHOWNORMAL)<=(HINSTANCE)32)
				MessageBox(hWnd,"Unable to run the update checker.Please reinstall player.","Error",MB_ICONERROR);
		}
		break;
	case ID_GOTO_TRACK:
		if(!PlayList)
		{
			MessageBox(hWnd,"Please load a playlist first...","Info", MB_ICONEXCLAMATION);
			break;
		}
		else
		{
			if(!alreadygoto)
				DialogBoxParam(hIns,MAKEINTRESOURCE(IDD_DIALOG8),HWND_DESKTOP,DLGPROC(&GotoTrackProc),NULL);
			break;
		}
		break;
	case ID_CHANGES:
		{
			DWORD size = 0;
			const char* data = NULL;
			LoadFileInResource(IDR_TEXTFILE1,133, size, data);
			// The text stored in the resource might not be NULL terminated.
			char* buffer = new char[size+1];
			memcpy(buffer, data, size);
			buffer[size] = 0; // NULL terminator
			MessageBox(hWnd,buffer,"What's New",0);
			delete[] buffer;
		}
		break;
	case ID_VIS_OFF:
		DeselectVisTick();
		CheckMenuItem(menu,ID_VIS_OFF,MF_CHECKED);
		if(Vis>0)
		{
			KillTimer(hWnd,ID_TIMER5);
			BASS_SFX_PluginStop(hSFX);
			HANDLE hBitmap;
			hBitmap = LoadImage(hIns,MAKEINTRESOURCE(IDB_BITMAP2), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
			HWND hPic = GetDlgItem(ghwnd,ID_VIS);
			SendMessage(hPic, STM_SETIMAGE, IMAGE_BITMAP, LPARAM(hBitmap)); 
			Vis=0;
			AlbumArt=false;
		}
		if(AlbumArt)
		{
			HANDLE hBitmap;
			hBitmap = LoadImage(hIns,MAKEINTRESOURCE(IDB_BITMAP2), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
			HWND hPic = GetDlgItem(ghwnd,ID_VIS);
			SendMessage(hPic, STM_SETIMAGE, IMAGE_BITMAP, LPARAM(hBitmap)); 
			Vis=0;
			AlbumArt=false;
		}
		break;
	case ID_ALBUM_ART:
		SendMessage(hWnd,WM_COMMAND,ID_VIS_OFF,0);
		DeselectVisTick();
		CheckMenuItem(menu,ID_ALBUM_ART,MF_CHECKED);
		GetAlbumArt(dump);
		AlbumArt=true;
		Vis=4;
		break;
	case ID_VIS_BLAZE:
		DeselectVisTick();
		CheckMenuItem(menu,ID_VIS_BLAZE,MF_CHECKED);
		Vis=1;
		Visualization_Start("vis_plugins\\blaze.dll");
		break;
	case ID_VIS_AROTA:
		DeselectVisTick();
		CheckMenuItem(menu,ID_VIS_AROTA,MF_CHECKED);
		Vis=2;
		Visualization_Start("vis_plugins\\Aorta.svp");
		break;
	case ID_VIS_METERX:
		DeselectVisTick();
		CheckMenuItem(menu,ID_VIS_METERX,MF_CHECKED);
		Vis=3;
		Visualization_Start("vis_plugins\\MetreX.svp");
		break;
	case ID_VIS_FULL:
		{
			int tem=Vis;
			SendMessage(hWnd,WM_COMMAND,ID_VIS_OFF,0);
			Vis=tem;
			ShowWindow(hWnd,SW_HIDE);
			DialogBoxParam(hIns,MAKEINTRESOURCE(IDD_DIALOG7),hWnd,DLGPROC(&FullVisProc),NULL);
		}
		break;
	case ID_SKINS_OFF:
		{
			DeselectSkinTick();
			skin.ClearSkin();
			char temp[50]={0};
			wsprintf(temp,"%d",0);
			WritePrivateProfileString("skins","data",temp,settings_path);
			CheckMenuItem(menu,ID_SKINS_OFF,MF_CHECKED);
		}
		break;
	case ID_SKINS_GLOW:
		SkinMe("skins\\glow.skf",1);
		CheckMenuItem(menu,ID_SKINS_GLOW,MF_CHECKED);
		break;
	case ID_SKINS_INDIGO:
		SkinMe("skins\\indigo.skf",2);
		CheckMenuItem(menu,ID_SKINS_INDIGO,MF_CHECKED);
		break;
	case ID_SKINS_JARILO:
		SkinMe("skins\\jarilo.skf",3);
		CheckMenuItem(menu,ID_SKINS_JARILO,MF_CHECKED);
		break;
	case ID_SKINS_MACOS:
		SkinMe("skins\\macos.skf",4);
		CheckMenuItem(menu,ID_SKINS_MACOS,MF_CHECKED);
		break;
	case ID_SKINS_TRANSILVANIA:
		SkinMe("skins\\transilvania.skf",5);
		CheckMenuItem(menu,ID_SKINS_TRANSILVANIA,MF_CHECKED);
		break;
	case ID_SKINS_V:
		SkinMe("skins\\V-touch.skf",6);
		CheckMenuItem(menu,ID_SKINS_V,MF_CHECKED);
		break;
	case ID_SKINS_ACCENT:
		SkinMe("skins\\Accent_ST.skf",7);
		CheckMenuItem(menu,ID_SKINS_ACCENT,MF_CHECKED);
		break;
	case ID_SKINS_ZIPPO:
		SkinMe("skins\\zippo.skf",8);
		CheckMenuItem(menu,ID_SKINS_ZIPPO,MF_CHECKED);
		break;
	case ID_SKINS_MOOSE:
		SkinMe("skins\\Moose.skf",9);
		CheckMenuItem(menu,ID_SKINS_MOOSE,MF_CHECKED);
		break;
	case ID_SKINS_DEVOIR:
		SkinMe("skins\\devoir.skf",10);
		CheckMenuItem(menu,ID_SKINS_DEVOIR,MF_CHECKED);
		break;
	case ID_OPTIONS_OPENTRACKLOCATION:
		if(MonitorTrack)
		{
			char* open=new char[1024];
			lstrcpy(open," /select, ");
			lstrcat(open,dump);
			ShellExecute(hWnd,"OPEN","explorer.exe",open,NULL,SW_SHOWMAXIMIZED);
			delete [] open;
		}
		else
			MessageBox(hWnd,"No song is loaded, play a song first !","Error",MB_ICONEXCLAMATION);
		break;
	case ID_REMEMBER_VOLUMEREMEMBERON:
		{
			if(GetMenuState(menu,ID_REMEMBER_VOLUMEREMEMBERON,MF_BYCOMMAND)==MF_UNCHECKED)
			{
				CheckMenuItem(menu,ID_REMEMBER_VOLUMEREMEMBERON,MF_CHECKED);
				char* temp=new char[10];
				wsprintf(temp,"%d",enable);
				WritePrivateProfileString("vrem","val",temp,settings_path);
				delete [] temp;

				//Save the current volume...
				char* vols=new char[500];
				sprintf(vols,"%f",p);
				WritePrivateProfileString("volume","pos",vols,settings_path);
				delete [] vols;
			}
			else
			{
				CheckMenuItem(menu,ID_REMEMBER_VOLUMEREMEMBERON,MF_UNCHECKED);
				char* temp=new char[10];
				wsprintf(temp,"%d",disable);
				WritePrivateProfileString("vrem","val",temp,settings_path);
				delete [] temp;
			}
		}
		break;
	case ID_ABOUT_REPORTBUG:
		ShellExecute(hWnd,"OPEN","http://sourceforge.net/p/tejashplayer/discussion/",NULL,NULL,SW_SHOWMAXIMIZED);
		break;
	case ID_REMEMBER_TRACKREMEMBERON:
		{
			if(GetMenuState(menu,ID_REMEMBER_TRACKREMEMBERON,MF_BYCOMMAND)==MF_UNCHECKED)
			{
				CheckMenuItem(menu,ID_REMEMBER_TRACKREMEMBERON,MF_CHECKED);
				char* temp=new char[10];
				wsprintf(temp,"%d",enable);
				WritePrivateProfileString("trem","val",temp,settings_path);
				delete [] temp;
				trackrem=true;
				if(MonitorTrack || lstrlen(dump))
					WritePrivateProfileString("track","data",dump,settings_path);
			}
			else
			{
				CheckMenuItem(menu,ID_REMEMBER_TRACKREMEMBERON,MF_UNCHECKED);
				char* temp=new char[10];
				wsprintf(temp,"%d",disable);
				WritePrivateProfileString("trem","val",temp,settings_path);
				delete [] temp;
				trackrem=false;
			}
		}
		break;
	case ID_SETTINGS_NOTIFYABOUTCURRENTTRACK:
		{
			if(GetMenuState(menu,ID_SETTINGS_NOTIFYABOUTCURRENTTRACK,MF_BYCOMMAND)==MF_UNCHECKED)
			{
				CheckMenuItem(menu,ID_SETTINGS_NOTIFYABOUTCURRENTTRACK,MF_CHECKED);
				char* temp=new char[10];
				wsprintf(temp,"%d",1);
				WritePrivateProfileString("notify","val",temp,settings_path);
				delete [] temp;
				notify=true;
			}
			else
			{
				CheckMenuItem(menu,ID_SETTINGS_NOTIFYABOUTCURRENTTRACK,MF_UNCHECKED);
				char* temp=new char[10];
				wsprintf(temp,"%d",0);
				WritePrivateProfileString("notify","val",temp,settings_path);
				delete [] temp;
				notify=false;
			}
		}
		break;

	}
}

void SkinMe(char path[],int x)
{
	DeselectSkinTick();
	char loc[MAX_PATH];
	int a=0;
	GetModuleFileName(NULL,loc,sizeof loc);
	for(int i=0;i<strlen(loc);i++)
	{
		if(loc[i]=='\\')
			a=i;
	}
	loc[a]='\0';
	lstrcat(loc,"\\");
	lstrcat(loc,path);
	OLECHAR* oledata=new OLECHAR[strlen(loc)+1];
	mbstowcs(oledata,loc,strlen(loc)+1);
	BSTR data=SysAllocString(oledata);
	skin.LoadSkinFromFile(data);
	SysFreeString(data);
	delete oledata;
	skin.ApplySkin();
	char temp[50]={0};
	wsprintf(temp,"%d",x);
	WritePrivateProfileString("skins","data",temp,settings_path);
}

void Visualization_Start(char* path)
{
	if(Vis>0)
	{
		KillTimer(ghwnd,ID_TIMER5);
		Sleep(28);
		BASS_SFX_PluginStop(hSFX);
		BASS_SFX_PluginFree(hSFX);
		InvalidateRect(GetDlgItem(ghwnd,ID_VIS),&rect,true);
	}
	hSFX = BASS_SFX_PluginCreate(path,GetDlgItem(ghwnd,ID_VIS),rect.right -rect.left ,rect.bottom -rect.top ,0);
	BASS_SFX_PluginSetStream(hSFX,channel);
	BASS_SFX_PluginStart(hSFX);
	SetTimer(ghwnd,ID_TIMER5,27,0);
	AlbumArt=false;
}

void OpenPlayList(HWND hWnd,char CmdLine[])
{
	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=hWnd;
	ofn.nMaxFile=MAX_PATH;
	ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
	ofn.lpstrFilter="Tejash Player Playlists\0*.tpp\0\0";
	ofn.lpstrFile = CmdLine;
	GetOpenFileName(&ofn);
}

void Open(HWND hWnd,char CmdLine[])
{
	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=hWnd;
	ofn.nMaxFile=MAX_PATH;
	ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
	ofn.lpstrFilter="Playable files\0*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif;*.mo3;*.it;*.xm;*.s3m;*.mtm;*.mod;*.umx;*.cda;*.fla;*.flac;*.oga;*.ogg;*.wma;*.wv;*.aac;*.m4a;*.m4b;*.mp4;*.ac3;*.adx;*.aix;*.ape;*.mac;*.mpc;*mp+;*.mpp;*.ofr;*.ofs;*.tta\0Tejash Player Playlists\0*.tpp\0\0";
	ofn.lpstrFile = CmdLine;
	GetOpenFileName(&ofn);
}
void PlaySong(HWND hWnd,char Song[])
{
	if(lstrcmp(Song,dump)==0)
		samesong=true;
	else
		samesong=false;
	if(lstrlen(Song))
	{
		if(_access(Song,0)==-1)
			PlayNext();
		else
		{
			if(IsSkip(Song))
				PlayNext();
			else
			{
				lstrcpy(dump,Song);
				ZeroMemory(chantime,sizeof chantime);
				BASS_Stop();
				if(!(channel=BASS_StreamCreateFile(false,Song,0,0,BASS_SAMPLE_FX | BASS_STREAM_AUTOFREE )) && !(channel=BASS_MusicLoad(false,Song,0,0,BASS_MUSIC_PRESCAN | BASS_MUSIC_AUTOFREE | BASS_MUSIC_RAMP | BASS_SAMPLE_FX,0)))
				{
					MonitorTrack=false;
					SetDlgItemText(hWnd,IDC_EDIT1,"Can not play the song");
					BASS_Stop();
				}

				else
				{
					if(!playtemp && ! PlayList)
						EnableWindow(GetDlgItem(ghwnd,IDC_LOOPALL),false);

					if(PlayList)
					{
						char* dmp=new char[100];
						wsprintf(dmp,"%d",enable);
						WritePrivateProfileString("IsPlaylist","val",dmp,settings_path);
						if(trackrem)
						{
							ZeroMemory(dmp,sizeof dmp);
							WritePrivateProfileString("IsPlaylist","path",playlist_path,settings_path);
							ZeroMemory(dmp,sizeof dmp);
							wsprintf(dmp,"%d",index);
							WritePrivateProfileString("IsPlaylist","index",dmp,settings_path);
						}
						delete [] dmp;
					}
					else
					{
						char* dmp=new char[100];
						wsprintf(dmp,"%d",disable);
						WritePrivateProfileString("IsPlaylist","val",dmp,settings_path);
						delete [] dmp;
						if(trackrem)
							WritePrivateProfileString("track","data",dump,settings_path);
					}

					SendDlgItemMessage(hWnd,IDC_SLIDER2,TBM_SETPOS,1,MAKELONG(0,0));
					BASS_Start();
					BASS_ChannelPlay(channel,false);
					if(IsDlgButtonChecked(ghwnd,IDC_CHECK2) == BST_CHECKED)
						BASS_ChannelFlags(channel,BASS_SAMPLE_LOOP,BASS_SAMPLE_LOOP);
					BASS_ChannelRemoveSync(channel,hsync);
					hsync=BASS_ChannelSetSync(channel, BASS_SYNC_END, 0, &MyMetaSyncProc, 0); // set the sync
					TerminateThread(thrhandle4,0);
					thrhandle4=CreateThread(0,0,(LPTHREAD_START_ROUTINE)UpdateTime,0,0,0);
					EnableWindow(GetDlgItem(ghwnd,IDC_CHECK2),true);
					EnableWindow(GetDlgItem(ghwnd,IDC_CHECK3),true);
					
					if(IsDlgButtonChecked(hWnd,IDC_CHECK1)==BST_CHECKED)
						BASS_SetVolume(000.f);
					else
					{
						p=SendMessage(GetDlgItem(ghwnd,IDC_SLIDER1),TBM_GETPOS,0,0);
						BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL,float(p/100.f));
					}

					if(IsDlgButtonChecked(hWnd,IDC_CHECK3) == BST_CHECKED)
					{
						BASS_ChannelFlags(channel,BASS_MUSIC_SURROUND,BASS_MUSIC_SURROUND);
						BASS_ChannelFlags(channel,BASS_MUSIC_3D ,BASS_MUSIC_3D);
					}

					if(IsMod(Song))
					{
						EnableWindow(GetDlgItem(hWnd,IDC_CHECK3),true);
						ZeroMemory(title,sizeof title);
						const char* name=BASS_ChannelGetTags(channel, BASS_TAG_MUSIC_NAME); 
						wsprintf(title,"%.30s",name);
						if(lstrlen(title))
							SetDlgItemText(ghwnd,IDC_EDIT1,title);
						else
						{
							ZeroMemory(bad_song_name,sizeof bad_song_name);
							GetNamefromPath(Song,bad_song_name);
							SetDlgItemText(hWnd,IDC_EDIT1,bad_song_name);
						}
					}
					else
					{
						EnableWindow(GetDlgItem(hWnd,IDC_CHECK3),false);
						EnableWindow(GetDlgItem(hWnd,IDC_CHECK2),true);
						if(AlbumArt)
							GetAlbumArt(Song);
						__try
						{
							GetIDTag(hWnd,Song);
							SngError=false;
							if(lstrlen(title))
								SetDlgItemText(hWnd,IDC_EDIT1,title);
							else
							{
								ZeroMemory(bad_song_name,sizeof bad_song_name);
								GetNamefromPath(Song,bad_song_name);
								SetDlgItemText(hWnd,IDC_EDIT1,bad_song_name);
							}

						}
						__except(ShowExceptionDlg(GetExceptionInformation())) 
						{
							ZeroMemory(bad_song_name,sizeof bad_song_name);
							GetNamefromPath(Song,bad_song_name);
							SetDlgItemText(hWnd,IDC_EDIT1,bad_song_name);
							SngError=true;
						}
					}

					if(once_fx_set)
					{
						// Effect setups
						fx[0]=BASS_ChannelSetFX(channel ,BASS_FX_DX8_PARAMEQ,0);
						fx[1]=BASS_ChannelSetFX(channel ,BASS_FX_DX8_PARAMEQ,0);
						fx[2]=BASS_ChannelSetFX(channel ,BASS_FX_DX8_PARAMEQ,0);
						fx[3]=BASS_ChannelSetFX(channel ,BASS_FX_DX8_REVERB,0);
						pdx.fGain=0;
						pdx.fBandwidth=18;
						pdx.fCenter=125;
						BASS_FXSetParameters(fx[0],&pdx);
						pdx.fCenter=1000;
						BASS_FXSetParameters(fx[1],&pdx);
						pdx.fCenter=8000;
						BASS_FXSetParameters(fx[2],&pdx);


						UpdateFX2(bass,false,0);
						UpdateFX2(vocal,false,1);
						UpdateFX2(trebal,false,2);
						UpdateFX2(reverb,true,3);
					}

					//Saves last
					ZeroMemory(lastsong,sizeof lastsong);
					lstrcpy(lastsong,dump);
					lstrcpy(Song,dump);

					EnableWindow(GetDlgItem(hWnd,IDC_BUTTON2),true);
					// Store playing file location
					if(!samesong)
					{
						samesong=false;
						if(!PlayList)
						{
							if(lstrcmp(lastsongs,dump) != 0)
							{
								if(previousasked==false)
								{
									ZeroMemory(row,sizeof row);
									index++;
									MaxIndex=index;
									lastindex=index;
									wsprintf(row,"%d",index);
									WritePrivateProfileString("songs",row,Song,recent);
								}
							}
						}
					}
					MonitorTrack=true;

					lstrcpy(lastsongs,dump);

					// display the file type and length
					QWORD bytes=BASS_ChannelGetLength(channel,BASS_POS_BYTE);
					DWORD time=BASS_ChannelBytes2Seconds(channel,bytes);
					leng=time;
					char file[20]={0};
					sprintf(file,"%u:%02u",time/60,time%60);
					SendDlgItemMessage(hWnd,TRACK_TIME,WM_SETTEXT,0,(LPARAM)file);
					// Update range of seek bar
					SendDlgItemMessage(hWnd,IDC_SLIDER2,TBM_SETRANGE,1,MAKELONG(0,leng));
				}
			}
		}
	}

	if(notify)
	{
		//Renotify user with playing song details :
		if(!SngError)
			sprintf(notifyicondata.szInfo,"%s",title);
		else
		{
			ZeroMemory(bad_song_name,sizeof bad_song_name);
			GetNamefromPath(dump,bad_song_name);
			sprintf(notifyicondata.szInfo,"%s",bad_song_name);
		}

		sprintf(notifyicondata.szInfoTitle,"%s","Now Playing");
		Shell_NotifyIcon(NIM_MODIFY,&notifyicondata);
	}
}

void CALLBACK MyMetaSyncProc(HSYNC handle, DWORD channel, DWORD data, void *user)
{
	if(MonitorTrack)
	{
		if(BASS_ChannelIsActive(channel)==BASS_ACTIVE_STOPPED)
		{
			TerminateThread(thrhandle4,0); // Terminate thread liable for slider time of track...
			MonitorTrack=false;
			BASS_Stop();
			stop = true;
			if(PlayList)
			{
				BASS_Stop();
				index++;
				if(index <= MaxIndex)
				{
					if(index !=0)
					{
						ZeroMemory(row,sizeof row);
						wsprintf(row,"%d",index);
						ZeroMemory(CmdLine,sizeof CmdLine);
						GetPrivateProfileString("songs",row,NULL,CmdLine,sizeof CmdLine,playlist_path);
						MonitorTrack=true;
						PlaySong(ghwnd,CmdLine);
					}
				}
				else
				{
					if(IsDlgButtonChecked(ghwnd,IDC_LOOPALL) == BST_CHECKED)
					{
						// Loop all enabled.Parse playlist again and play from start....
						ParsePlayList(playlist_path);
					}
				}
			}
			
			if(!PlayList)
			{
				if(previousasked)
				{
					if(precurrentindex < MaxIndex)
					{
						preonce=true;
						PlayNext();
					}
				}
				else
					MonitorTrack =false;
			}
		}
	}
}

bool IsMod(char path[])
{
	char* extension=strrchr(path,'.');
	strlwr(extension);
	if(strcmp(extension,".mo3")==0 || strcmp(extension,".it")==0 ||  strcmp(extension,".xm")==0 ||  strcmp(extension,".s3m")==0 ||  strcmp(extension,".mod")==0 ||   strcmp(extension,".umx")==0)
		return true;
	else
		return false;
}

bool IsSkip(char* path)
{
	char* extension=strrchr(path,'.');
	strlwr(extension);
	if(strcmp(extension,".skip")==0)
		return true;
	else
		return false;
}

void UpdateTime()
{
	while(true)
	{
		ZeroMemory(settings_path,sizeof settings_path);
		lstrcpy(settings_path,settings_path2);
		// display the file length
		QWORD bytes=BASS_ChannelGetPosition(channel,BASS_POS_BYTE);
		DWORD time=0;
		time=BASS_ChannelBytes2Seconds(channel,bytes);
		sprintf(file,"%u:%02u",time/60,time%60);
		SendDlgItemMessage(ghwnd,DEF_TIME,WM_SETTEXT,0,(LPARAM)file);

		//If track remember is on then save current time too
		if(trackrem)
		{
			int pos=SendDlgItemMessage(ghwnd,IDC_SLIDER2,TBM_GETPOS,0,0);
			char* buf=new char[100];
			wsprintf(buf,"%d",pos);
			WritePrivateProfileString("track","time",buf,settings_path);
			delete [] buf;
		}
		
		if(!MonitorTrack)
			ExitThread(0);

		Sleep(100);
	}
}

void GetIDTag(HWND hWnd,char Song[])
{
	ZeroMemory(album,sizeof album);
	ZeroMemory(artist,sizeof artist);
	ZeroMemory(title,sizeof title);
	ZeroMemory(year,sizeof year);
	ZeroMemory(comment,sizeof comment);

	mp3fi.Free();

	if(!mp3fi.Init(Song))
	{
		// If MP3 is not there then use BASS TAG functions....
bassid:
		id3=(TAG_ID3*)BASS_ChannelGetTags(channel, BASS_TAG_ID3); 
		wsprintf(title,id3->title);
		wsprintf(album,"%s %.30s","Album : ",id3->album);
		wsprintf(artist,"%s %.30s","Artist : ",id3->artist);
		wsprintf(year,"%s %.4s","Year : ",id3->year);
		wsprintf(comment,"%s %.30s","Comment : ",id3->comment);
		SetDlgItemText(hWnd,IDC_EDIT1,title);
	}
	else
	{
		// If MP3 is there use ID3Lib ...
		if(mp3fi.bHasV1Tag || mp3fi.bHasV2Tag)
		{
			wsprintf(title,mp3fi.szTitle);
			wsprintf(album,"%s %s","Album : ",mp3fi.szAlbum);
			wsprintf(artist,"%s %s","Artist : ",mp3fi.szArtist);
			wsprintf(year,"%s %s","Year : ",mp3fi.szYear);
			wsprintf(comment,"%s %s","Comment : ",mp3fi.szComment);
			SetDlgItemText(hWnd,IDC_EDIT1,title);
		}
		else
			goto bassid;
	}
}

void SetPopUpMenuItem(HWND hWnd)
{
	HMENU hPopupMenu;
	POINT p;
	hPopupMenu=CreatePopupMenu();
	GetCursorPos(&p);

	char* buffer1=new char[1024];
	char* buffer6=new char[1024];

	lstrcpy(buffer1,"Song : ");
	lstrcpy(buffer6,"Track No : ");
	lstrcat(buffer1,title);

	char bufx[150];
	wsprintf(bufx,"%d",index);
	lstrcat(buffer6,bufx);

	ZeroMemory(bad_song_name,sizeof bad_song_name);
	GetNamefromPath(dump,bad_song_name);

	if(MonitorTrack)
	{
		if(! IsMod(dump))
		{
			if(!SngError)
			{
				if(PlayList)
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, buffer6);
			
				InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, comment);
				InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, year);
				InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, album);
				InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, artist);
				InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, buffer1);
			}
			else
			{
				if(PlayList)
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, buffer6);
				InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, bad_song_name);
			}
		}

		if(IsMod(dump))
		{
			ZeroMemory(title,sizeof title);
			const char* name=BASS_ChannelGetTags(channel, BASS_TAG_MUSIC_NAME); 
			wsprintf(title,"%.30s",name);
			if(PlayList)
			{
				InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, buffer6);
				if(lstrlen(title))
				{
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, title);
				}
				else
				{
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, bad_song_name);
				}
			}
			else
			{
				if(lstrlen(title))
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, title);
				else
				{
					InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, bad_song_name);
				}
			}
		}
	}
	else
	{
		if(trackrem)
			InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, "Play the song first");
		else
			InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, "No song loaded !");
	}

	TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, p.x, p.y+23, 0, hWnd, NULL);

	delete [] buffer1;
	delete [] buffer6;
}

void InitTray()
{
	memset(&notifyicondata,0,sizeof(NOTIFYICONDATA));
	notifyicondata.cbSize=sizeof(NOTIFYICONDATA);
	notifyicondata.hWnd = ghwnd;
	notifyicondata.uID = ID_TRAY_TIC;
	notifyicondata.uFlags = NIF_ICON |NIF_MESSAGE |NIF_TIP | NIF_INFO;     
	notifyicondata.uCallbackMessage = WM_TRAYICON; 
	notifyicondata.hIcon = (HICON)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON,16,16,0);
	sprintf(notifyicondata.szTip,"%s","Tejash Player v4.3");
	sprintf(notifyicondata.szInfo,"%s","Tejash Player is now running in taskbar\nRight Click on icon for options");
	sprintf(notifyicondata.szInfoTitle,"%s","Welcome");
	notifyicondata.hBalloonIcon=(HICON)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON,16,16,0);
	notifyicondata.uTimeout = 1000;

	g_menu = CreatePopupMenu();

	InsertMenu(g_menu,0,MF_BYPOSITION | MF_STRING,ID_TRAY_MAXIMIZE,"Restore");
	InsertMenu(g_menu,1,MF_BYPOSITION | MF_STRING,ID_TRAY_PLAYLIST,"Playlist");
	InsertMenu(g_menu,2,MF_BYPOSITION | MF_STRING,ID_TRAY_ABOUT,"About");
	InsertMenu(g_menu,3,MF_BYPOSITION | MF_STRING,ID_TRAY_EXIT,"Exit");

	Shell_NotifyIcon(NIM_ADD,&notifyicondata);
}


void GoToTray(bool flag)
{
	if(flag)
	{
		visible=false;
		ShowWindow(ghwnd,SW_HIDE);
	}
	if(!flag)
	{
		visible=true;
		ShowWindow(ghwnd, SW_SHOW);
	}
}

int PlayPrevious()
{
	if(PlayList)
	{
		if(index <= N)
		{
			char temp[MAX_PATH];
			index--;
			if(index !=0)
			{
				wsprintf(row,"%d",index);
				GetPrivateProfileString("songs",row,NULL,temp,sizeof temp,playlist_path);
				lastindex=index;
				ZeroMemory(lastsong,sizeof lastsong);
				lstrcpy(lastsong,temp);
				if(IsSkip(temp))
				{
					PlayPrevious();
				}
				else
				{
					if(_access(temp,0)==-1)
						PlayPrevious();
					else
						PlaySong(ghwnd,temp);
				}
			}
			if(index==0)
				index++;
		}
	}
	else
	{
		previousasked=true;
		char temp[MAX_PATH];
		ZeroMemory(temp,sizeof temp);
		ZeroMemory(row,sizeof row);
		if(!preonce)
		{
			currentindex=index-1;
			precurrentindex=currentindex;
			if(precurrentindex>0)
			{
				wsprintf(row,"%d",precurrentindex);
				GetPrivateProfileString("songs",row,NULL,temp,sizeof temp,recent);
				lastindex=precurrentindex;
				ZeroMemory(lastsong,sizeof lastsong);
				lstrcpy(lastsong,temp);
				if(_access(temp,0)==-1)
					PlayPrevious();
				else
					PlaySong(ghwnd,temp);
				preonce=true;
				if(precurrentindex > 1 )
				{
					precurrentindex--;
				}
			}
			return 0;
		}
		if(preonce)
		{
			if(precurrentindex > 0)
			{
				wsprintf(row,"%d",precurrentindex);
				GetPrivateProfileString("songs",row,NULL,temp,sizeof temp,recent);
				if(precurrentindex > 1)
				{
					precurrentindex--;
				}
				lastindex=precurrentindex;
				ZeroMemory(lastsong,sizeof lastsong);
				lstrcpy(lastsong,temp);
				if(_access(temp,0)==-1)
					PlayPrevious();
				else
					PlaySong(ghwnd,temp);
			}
			return 0;
		}
	}
}

int PlayNext()
{
	if(PlayList)
	{
		if(index < N)
		{
			char temp[MAX_PATH]={0};
			index++;
			wsprintf(row,"%d",index);
			GetPrivateProfileString("songs",row,NULL,temp,sizeof temp,playlist_path);
			lastindex=index;
			ZeroMemory(lastsong,sizeof lastsong);
			lstrcpy(lastsong,temp);
			if(_access(temp,0)==-1)
				PlayNext();
			else
				PlaySong(ghwnd,temp);
		}
	}
	else
	{
		previousasked =true;
		int max=index;
		char temp[MAX_PATH]={0};
		ZeroMemory(temp,sizeof temp);
		ZeroMemory(row,sizeof row);
		if(preonce)
		{
			if(precurrentindex > 0)
			{
				if(precurrentindex != max)
				{
					precurrentindex++;
					wsprintf(row,"%d",precurrentindex);
					GetPrivateProfileString("songs",row,NULL,temp,sizeof temp,recent);
					lastindex=precurrentindex;
					ZeroMemory(lastsong,sizeof lastsong);
					lstrcpy(lastsong,temp);
					if(_access(temp,0)==-1)
						PlayNext();
					else
						PlaySong(ghwnd,temp);
				}
			}
		}
	}
	return 0;
}

void GetNamefromPath(char Song[],char title[])
{
	int len=lstrlen(Song);
	int len2=lstrlen(title);
	int index=0;
	int j=0;
	ZeroMemory(tempbuf,sizeof tempbuf);
	for(int i=0;i<len;i++)
	{
		if(Song[i]=='\\')
			index=i;
	}
	if(index==0)
	{
		lstrcpy(title,Song);
		return;
	}
	index++;
	for(int i=index;i<len;i++)
	{
		tempbuf[j]=Song[i];
		j++;
		if(j==len2)
			break;
	}
	// Now removing extention
	int x=lstrlen(tempbuf);
	index=0;
	j=0;
	for(int i=0;i<x;i++)
	{
		if(tempbuf[i]=='.')
			index=i;
	}
	tempbuf[index]='\0';
	lstrcpy(title,tempbuf);
}

// About Box functions....


BOOL CALLBACK AboutProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	LOGFONT lFont;
	RECT Rect;
	char * AboutMsg=new char[1024];

#ifdef COMMERCIAL
	AboutMsg="Tejash Player X12.3 Commercial Version\nDeveloped by Tejashwi Kalp Taru, TechTejash Developers Group\n\nSpecial Thanks To:\nUn4seen Developments Ltd. and  BASS plugins developers\nNapalm for Water Effect and GDI Memory leak Fixes\nChristian Richardt for ID3Lib wrapper class to read ID3v2 Tags\nVidya Sagar,Kundan and Rahul Verma for Ideas\n\nFor commercial use only.\nYou can use this for personal uses too\n\n\nCopyright © 2012 Tejashwi Kalp Taru, All rights reserved.";
#endif
#ifndef COMMERCIAL
	AboutMsg="Tejash Player X12.3 Free Version\nDeveloped by Tejashwi Kalp Taru, TechTejash Developers Group\n\nSpecial Thanks To:\nUn4seen Developments Ltd. and  BASS plugins developers\nNapalm for Water Effect and GDI Memory leak Fixes\nChristian Richardt for ID3Lib wrapper class to read ID3v2 Tags\nVidya Sagar,Kundan and Rahul Verma for Ideas\n\nFor non commercial use only.\nBuy a commercial version for commercial purpose\n\n\nCopyright © 2012 Tejashwi Kalp Taru, All rights reserved.";
#endif

	switch(message)
	{
	case WM_INITDIALOG:
		{
			SetDlgItemText(hWnd,IDC_ABOUT_TEXT,AboutMsg);
			BITMAP bm;
			HBITMAP hImage = (HBITMAP)LoadImage(hIns, MAKEINTRESOURCE(IDB_BITMAP1),IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
			if(hImage)
			{
				GetObject(hImage, sizeof(bm), &bm);
				Ripple_Create(0,0,bm.bmWidth,bm.bmHeight,106, hWnd, hIns, hImage);
				DeleteObject(hImage);
			}

			//Underlines the website's text in about box
			GetObject((HANDLE)SendMessage(hWnd,WM_GETFONT,0,0),sizeof (LOGFONT),&lFont);
			lFont.lfUnderline=true;
			SendMessage(GetDlgItem(hWnd,IDC_WEBSITE),WM_SETFONT,(WPARAM)CreateFontIndirect(&lFont),true);
		}
		return true;
	case WM_CLOSE:
		delete [] AboutMsg;
		EndDialog(hWnd,0);
		return true;
	}
	if(message > WM_MOUSEFIRST-1 && message < WM_MOUSELAST+1)
	{
		GetWindowRect(GetDlgItem(hWnd,IDC_WEBSITE),&Rect);
		MapWindowPoints(NULL,hWnd,(LPPOINT)&Rect,2);
		POINT pt;
		pt.x =LOWORD(lParam);
		pt.y =HIWORD(lParam);
		if(PtInRect(&Rect,pt))
		{
			if(message==WM_LBUTTONUP)
			{
				ShellExecute(hWnd,"OPEN","http://www.tejashwi.com/techtejash/tejashplayer.html",0,0,SW_SHOWMAXIMIZED);
			}
			SetCursor(LoadCursor(NULL,IDC_HAND));
		}
		else
		{
			SetCursor(LoadCursor(NULL,IDC_ARROW));
		}
	}
	return false;
}

BOOL GetFolderSelection(HWND hWnd, LPTSTR szBuf, LPCTSTR szTitle)
{
	LPITEMIDLIST pidl     = NULL;
	BROWSEINFO   bi       = { 0 };
	BOOL         bResult  = FALSE;

	bi.hwndOwner      = hWnd;
	bi.pszDisplayName = szBuf;
	bi.pidlRoot       = NULL;
	bi.lpszTitle      = szTitle;
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

	if ((pidl = SHBrowseForFolder(&bi)) != NULL)
	{   
		bResult = SHGetPathFromIDList(pidl, szBuf); 
		CoTaskMemFree(pidl);
	}

	return bResult;
}

void PreparePlayList()
{
	PrepareList(fldr);
}

int PrepareList(char fldr[])
{
	if(playtemp)
	{
		ZeroMemory(buffer,sizeof buffer);
		ZeroMemory(temp_playlist,sizeof temp_playlist);
		lstrcpy(buffer,localdir);
		lstrcat(buffer,"\\tp_p_list.ini");
		DeleteFile(buffer);

		char* ext1[]={".mp3"};
		char* ext2[]={".mp2"};
		char* ext3[]={".mp1"};
		char* ext4[]={".ogg"};
		char* ext5[]={".wav"};
		char* ext6[]={".aif"};
		char* ext7[]={".mo3"};
		char* ext8[]={".it"};
		char* ext9[]={".xm"};
		char* ext10[]={".s3m"};
		char* ext11[]={".mtm"};
		char* ext12[]={".mod"};
		char* ext13[]={".umx"};
		char* ext14[]={".cda"};
		char* ext15[]={".fla"};
		char* ext16[]={".flac"};
		char* ext17[]={".oga"};
		char* ext18[]={".ogg"};
		char* ext19[]={".wma"};
		char* ext20[]={".wv"};
		char* ext21[]={".aac"};
		char* ext22[]={".m4a"};
		char* ext23[]={".m4b"};
		char* ext24[]={".mp4"};
		char* ext25[]={".ac3"};
		char* ext26[]={".adx"};
		char* ext27[]={".aix"};
		char* ext28[]={".ape"};
		char* ext29[]={".mac"};
		char* ext30[]={".mpc"};
		char* ext31[]={".mp+"};
		char* ext32[]={".mpp"};
		char* ext33[]={".ofr"};
		char* ext34[]={".ofs"};
		char* ext35[]={".tta"};
		
		int t1=GetTickCount();

		ext_search(fldr,*ext1,buffer,true);
		ext_search(fldr,*ext2,buffer,false);
		ext_search(fldr,*ext3,buffer,false);
		ext_search(fldr,*ext4,buffer,false);
		ext_search(fldr,*ext5,buffer,false);
		ext_search(fldr,*ext6,buffer,false);
		ext_search(fldr,*ext7,buffer,false);
		ext_search(fldr,*ext8,buffer,false);
		ext_search(fldr,*ext9,buffer,false);
		ext_search(fldr,*ext10,buffer,false);
		ext_search(fldr,*ext11,buffer,false);
		ext_search(fldr,*ext12,buffer,false);
		ext_search(fldr,*ext13,buffer,false);
		ext_search(fldr,*ext14,buffer,false);
		ext_search(fldr,*ext15,buffer,false);
		ext_search(fldr,*ext16,buffer,false);
		ext_search(fldr,*ext17,buffer,false);
		ext_search(fldr,*ext18,buffer,false);
		ext_search(fldr,*ext19,buffer,false);
		ext_search(fldr,*ext20,buffer,false);
		ext_search(fldr,*ext21,buffer,false);
		ext_search(fldr,*ext22,buffer,false);
		ext_search(fldr,*ext23,buffer,false);
		ext_search(fldr,*ext24,buffer,false);
		ext_search(fldr,*ext25,buffer,false);
		ext_search(fldr,*ext26,buffer,false);
		ext_search(fldr,*ext27,buffer,false);
		ext_search(fldr,*ext28,buffer,false);
		ext_search(fldr,*ext29,buffer,false);
		ext_search(fldr,*ext30,buffer,false);
		ext_search(fldr,*ext31,buffer,false);
		ext_search(fldr,*ext32,buffer,false);
		ext_search(fldr,*ext33,buffer,false);
		ext_search(fldr,*ext34,buffer,false);
		ext_search(fldr,*ext35,buffer,false);

		int t2=GetTickCount();

		t1=t2-t1;
		int timetaken=t1/1000; // Converting milisecond to second

		SetWindowText(ghwnd,"Tejash Player");

		int t=TotalSongs();
		if(t==0)
		{
			MessageBox(ghwnd,"The file is not playable with Tejash Player","Sorry",MB_ICONINFORMATION);
			return 1;
		}

		N=TotalSongs();

		MaxIndex=N;

		ZeroMemory(row,sizeof row);
		wsprintf(row,"%d",N);
		WritePrivateProfileString("Total_Files","Total",row,buffer);

		char msg[100];
		sprintf(msg,"Playlist created with %d songs that found so far\nApprox %d seconds taken to find all songs",N,timetaken);
		MessageBox(ghwnd,msg,"Done",MB_ICONINFORMATION);
		PlayList =true;
		index=0;

		lstrcpy(playlist_path,buffer);
		lstrcpy(temp_playlist,buffer);

		// Enable option to loop all tracks
		EnableWindow(GetDlgItem(ghwnd,IDC_LOOPALL),true);
		
		if(!NoPlay)
		{
			// Play a file now
			BASS_Stop();
			index++;
			ZeroMemory(row,sizeof row);
			wsprintf(row,"%d",index);
			ZeroMemory(CmdLine,sizeof CmdLine);
			GetPrivateProfileString("songs",row,NULL,CmdLine,sizeof CmdLine,buffer);
			PlaySong(ghwnd,CmdLine);
			MonitorTrack=true;
		}

		if(alreadyplaylist)
		{
			alreadyplaylist=false;
			SendMessage(playlist_hwnd,WM_CLOSE,0,0);
		}
		return 1;
	}

	if(createplay)
	{
		ZeroMemory(buffer2,sizeof buffer2);
		char file[MAX_PATH]="";
		OPENFILENAME ofn={0};
		ofn.lStructSize=sizeof(ofn);
		ofn.hwndOwner=ghwnd;
		ofn.nMaxFile=MAX_PATH;
		ofn.lpstrFile=buffer2;
		ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
		ofn.lpstrFilter="Tejash Player Playlists\0*.tpp\0\0";
		ofn.lpstrDefExt="tpp";
		if (!GetSaveFileName(&ofn))
		{
			SetWindowText(ghwnd,"Tejash Player");
			return 1;
		}

		char* ext1[]={".mp3"};
		char* ext2[]={".mp2"};
		char* ext3[]={".mp1"};
		char* ext4[]={".ogg"};
		char* ext5[]={".wav"};
		char* ext6[]={".aif"};
		char* ext7[]={".mo3"};
		char* ext8[]={".it"};
		char* ext9[]={".xm"};
		char* ext10[]={".s3m"};
		char* ext11[]={".mtm"};
		char* ext12[]={".mod"};
		char* ext13[]={".umx"};
		char* ext14[]={".cda"};
		char* ext15[]={".fla"};
		char* ext16[]={".flac"};
		char* ext17[]={".oga"};
		char* ext18[]={".ogg"};
		char* ext19[]={".wma"};
		char* ext20[]={".wv"};
		char* ext21[]={".aac"};
		char* ext22[]={".m4a"};
		char* ext23[]={".m4b"};
		char* ext24[]={".mp4"};
		char* ext25[]={".ac3"};
		char* ext26[]={".adx"};
		char* ext27[]={".aix"};
		char* ext28[]={".ape"};
		char* ext29[]={".mac"};
		char* ext30[]={".mpc"};
		char* ext31[]={".mp+"};
		char* ext32[]={".mpp"};
		char* ext33[]={".ofr"};
		char* ext34[]={".ofs"};
		char* ext35[]={".tta"};

		int t1=GetTickCount();
		
		ext_search(fldr,*ext1,buffer2,true);
		ext_search(fldr,*ext2,buffer2,false);
		ext_search(fldr,*ext3,buffer2,false);
		ext_search(fldr,*ext4,buffer2,false);
		ext_search(fldr,*ext5,buffer2,false);
		ext_search(fldr,*ext6,buffer2,false);
		ext_search(fldr,*ext7,buffer2,false);
		ext_search(fldr,*ext8,buffer2,false);
		ext_search(fldr,*ext9,buffer2,false);
		ext_search(fldr,*ext10,buffer2,false);
		ext_search(fldr,*ext11,buffer2,false);
		ext_search(fldr,*ext12,buffer2,false);
		ext_search(fldr,*ext13,buffer2,false);
		ext_search(fldr,*ext14,buffer2,false);
		ext_search(fldr,*ext15,buffer2,false);
		ext_search(fldr,*ext16,buffer2,false);
		ext_search(fldr,*ext17,buffer2,false);
		ext_search(fldr,*ext18,buffer2,false);
		ext_search(fldr,*ext19,buffer2,false);
		ext_search(fldr,*ext20,buffer2,false);
		ext_search(fldr,*ext21,buffer2,false);
		ext_search(fldr,*ext22,buffer2,false);
		ext_search(fldr,*ext23,buffer2,false);
		ext_search(fldr,*ext24,buffer2,false);
		ext_search(fldr,*ext25,buffer2,false);
		ext_search(fldr,*ext26,buffer2,false);
		ext_search(fldr,*ext27,buffer2,false);
		ext_search(fldr,*ext28,buffer2,false);
		ext_search(fldr,*ext29,buffer2,false);
		ext_search(fldr,*ext30,buffer2,false);
		ext_search(fldr,*ext31,buffer2,false);
		ext_search(fldr,*ext32,buffer2,false);
		ext_search(fldr,*ext33,buffer2,false);
		ext_search(fldr,*ext34,buffer2,false);
		ext_search(fldr,*ext35,buffer2,false);

		int t2=GetTickCount();
		t1=t2-t1;

		t1=t1/1000;

		SetWindowText(ghwnd,"Tejash Player");

		Nm=TotalSongs();

		ZeroMemory(row,sizeof row);
		wsprintf(row,"%d",Nm);
		WritePrivateProfileString("Total_Files","Total",row,buffer2);
		char msg2[512]={0};
		sprintf(msg2,"Playlist saving completed...\nApprox %d seconds taken to find all songs\nDo you want to play from playlist now ?",t1);

		if(MessageBox(ghwnd,msg2,"Done",MB_ICONEXCLAMATION | MB_YESNO)==IDYES)
		{
			ParsePlayList(buffer2);
		}

		return 1;
	}
}

void ParsePlayList(char Path[])
{
	ZeroMemory(buffer,sizeof buffer);
	lstrcpy(buffer,Path);
	lstrcpy(playlist_path,buffer);
	ZeroMemory(row,sizeof row);
	MaxIndex=0;
	index=0;
	GetPrivateProfileString("Total_Files","Total",NULL,row,sizeof row,buffer);
	MaxIndex=atoi(row);
	N=MaxIndex;
	PlayList=true;
	//Enable option to loop all tracks in playlist...
	EnableWindow(GetDlgItem(ghwnd,IDC_LOOPALL),true);

	// Play a file now
	BASS_Stop();
	index++;
	ZeroMemory(row,sizeof row);
	wsprintf(row,"%d",index);
	ZeroMemory(CmdLine,sizeof CmdLine);
	GetPrivateProfileString("songs",row,NULL,CmdLine,sizeof CmdLine,buffer);
	PlaySong(ghwnd,CmdLine);
	MonitorTrack=true;
	Parsed=true;

	if(alreadyplaylist)
	{
		alreadyplaylist=false;
		SendMessage(playlist_hwnd,WM_CLOSE,0,0);
	}

}

bool PlayListDB()
{
	char* extension=PathFindExtension(CmdLine);
	strlwr(extension);
	if(lstrcmp(extension,".tpp")==0)
	{
		return true;
	}
	else
	{
		alreadyplaylist=false;
		if(playtemp || PlayList)
			index=0;
	//	SendMessage(playlist_hwnd,WM_CLOSE,0,0);
		return false;
	}
}

// Sound FX Settings:
BOOL CALLBACK FxDialog(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	BASS_INFO bi={sizeof(bi)};
	fxhwnd=hWnd;
	switch(message)
	{
		case WM_INITDIALOG:

			// Init tooltips
			gToolTip::AddTip(hWnd,hIns,"Bass settings",20,true);
			gToolTip::AddTip(hWnd,hIns,"Vocal settings",21,true);
			gToolTip::AddTip(hWnd,hIns,"Treble settings",22,true);
			gToolTip::AddTip(hWnd,hIns,"Echo settings",23,true);
			gToolTip::AddTip(hWnd,hIns,"Reset all user effects and set to defaults",IDC_RESET,true);
			// Check that weather settings is already opened or not
			if(settings_on)
			{
				BASS_Start();
				EndDialog(hWnd,0);
				return true;
			}

			// check that DX8 features are available
			BASS_GetInfo(&bi);
			if (bi.dsver<8)
			{
				MessageBox(hWnd,"DirectX 8 is not installed","Error",MB_ICONERROR);
				EnableWindow(GetDlgItem(hWnd,20),false);
				EnableWindow(GetDlgItem(hWnd,21),false);
				EnableWindow(GetDlgItem(hWnd,22),false);
				EnableWindow(GetDlgItem(hWnd,23),false);
			}
			else
			{
				// initialize eq/reverb sliders
				SendDlgItemMessage(hWnd,20,TBM_SETRANGE,false,MAKELONG(0,20));
				SendDlgItemMessage(hWnd,21,TBM_SETRANGE,false,MAKELONG(0,20));
				SendDlgItemMessage(hWnd,22,TBM_SETRANGE,false,MAKELONG(0,20));
				SendDlgItemMessage(hWnd,23,TBM_SETRANGE,false,MAKELONG(0,20));


				if(!once_fx_set)
				{
					SendDlgItemMessage(hWnd,20,TBM_SETPOS,true,10);
					SendDlgItemMessage(hWnd,21,TBM_SETPOS,true,10);
					SendDlgItemMessage(hWnd,22,TBM_SETPOS,true,10);
					SendDlgItemMessage(hWnd,23,TBM_SETPOS,true,10);
				}
				else
				{
					SendDlgItemMessage(hWnd,20,TBM_SETPOS,true,bass);
					SendDlgItemMessage(hWnd,21,TBM_SETPOS,true,vocal);
					SendDlgItemMessage(hWnd,22,TBM_SETPOS,true,trebal);
					SendDlgItemMessage(hWnd,23,TBM_SETPOS,true,reverb);

					UpdateFX(0,hWnd);
					UpdateFX(1,hWnd);
					UpdateFX(2,hWnd);
					UpdateFX(3,hWnd);
				}
			}

			if(!once_fx_set)
			{
				// Effect setups
				fx[0]=BASS_ChannelSetFX(channel ,BASS_FX_DX8_PARAMEQ,0);
				fx[1]=BASS_ChannelSetFX(channel ,BASS_FX_DX8_PARAMEQ,0);
				fx[2]=BASS_ChannelSetFX(channel ,BASS_FX_DX8_PARAMEQ,0);
				fx[3]=BASS_ChannelSetFX(channel ,BASS_FX_DX8_REVERB,0);
				pdx.fGain=0;
				pdx.fBandwidth=18;
				pdx.fCenter=125;
				BASS_FXSetParameters(fx[0],&pdx);
				pdx.fCenter=1000;
				BASS_FXSetParameters(fx[1],&pdx);
				pdx.fCenter=8000;
				BASS_FXSetParameters(fx[2],&pdx);
				UpdateFX(0,hWnd);
				UpdateFX(1,hWnd);
				UpdateFX(2,hWnd);
				UpdateFX(3,hWnd);
			}
			BASS_Start();
			settings_on=true;
			return true;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_RESET:
				{
					SendDlgItemMessage(hWnd,20,TBM_SETPOS,true,10);
					SendDlgItemMessage(hWnd,21,TBM_SETPOS,true,10);
					SendDlgItemMessage(hWnd,22,TBM_SETPOS,true,10);
					SendDlgItemMessage(hWnd,23,TBM_SETPOS,true,10);

					UpdateFX(0,hWnd);
					UpdateFX(1,hWnd);
					UpdateFX(2,hWnd);
					UpdateFX(3,hWnd);
				}
				break;
			}
			return true;
		case WM_VSCROLL:
			if(lParam)
			{
				once_fx_set=true;
				UpdateFX(GetDlgCtrlID((HWND)lParam)-20,hWnd);
			}
			return true;
		case WM_CLOSE:
			settings_on=false;
			EndDialog(hWnd,0);
			return true;
	}
	return false;
}

void UpdateFX2(int v,bool rev,int b)
{
	if(rev)
	{
		BASS_DX8_REVERB p;
		BASS_FXGetParameters(fx[3],&p);
		p.fReverbMix=(v<20?log(1-v/20.0)*20:-96);
		BASS_FXSetParameters(fx[3],&p);
	}
	if(!rev)
	{
		BASS_DX8_PARAMEQ p;
		BASS_FXGetParameters(fx[b],&p);
		p.fGain=10.0-v;
		BASS_FXSetParameters(fx[b],&p);
	}
}

void UpdateFX(int b,HWND hWnd)
{
	int v=SendDlgItemMessage(hWnd,20+b,TBM_GETPOS,0,0);
	if (b<3) 
	{
		BASS_DX8_PARAMEQ p;
		BASS_FXGetParameters(fx[b],&p);
		p.fGain=10.0-v;
		BASS_FXSetParameters(fx[b],&p);
	}
	else
	{
		BASS_DX8_REVERB p;
		BASS_FXGetParameters(fx[3],&p);
		p.fReverbMix=(v<20?log(1-v/20.0)*20:-96);
		BASS_FXSetParameters(fx[3],&p);
	}
	if(b==0)
		bass=SendDlgItemMessage(hWnd,20,TBM_GETPOS,0,0);
	if(b==1)
		vocal=SendDlgItemMessage(hWnd,21,TBM_GETPOS,0,0);
	if(b==2)
		trebal=SendDlgItemMessage(hWnd,22,TBM_GETPOS,0,0);
	if(b==3)
		reverb=SendDlgItemMessage(hWnd,23,TBM_GETPOS,0,0);

}

BOOL CALLBACK RecProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	const char *i;
	rechwnd=hWnd;
	char text[30]="";
	switch(message)
	{
	case WM_INITDIALOG:
		// setup recording device (using default device)
		if (!BASS_RecordInit(-1)) 
		{
			MessageBox(hWnd,"Can't initialize recording device","Error",MB_ICONINFORMATION);
			BASS_RecordFree();
			BASS_Free();
			ShowWindow(ghwnd,SW_RESTORE);
			ReInitBass();
			SetFocus(ghwnd);
			KillTimer(hWnd,ID_TIMER2);
			EndDialog(hWnd,0);
			return true;
		} 
		else
		{
			SendDlgItemMessage(hWnd,14,TBM_SETRANGE,false,MAKELONG(0,100));
			for(int c=0;i=BASS_RecordGetInputName(c);c++)
			{
				SendDlgItemMessage(hWnd,13,CB_ADDSTRING,0,(LPARAM)i);
				if (!(BASS_RecordGetInput(c,NULL)&BASS_INPUT_OFF))
				{
					input=c;
					SendDlgItemMessage(hWnd,13,CB_SETCURSEL,input,0);
					UpdateInputInfo(hWnd);
				}
			}

			// Init tooltips
			gToolTip::AddTip(hWnd,hIns,"Select input device",13,true);
			gToolTip::AddTip(hWnd,hIns,"Recording volume control",14,true);
			gToolTip::AddTip(hWnd,hIns,"Start/Stop recording",10,true);
			gToolTip::AddTip(hWnd,hIns,"Play recorded sound",11,true);
			gToolTip::AddTip(hWnd,hIns,"Save recorded sound",12,true);
		}
		SetFocus(hWnd);
		SetTimer(hWnd,ID_TIMER2,200,0);
		return true;
	case WM_HSCROLL:
		if (lParam) 
		{ // set input source level
			float level=SendMessage((HWND)lParam,TBM_GETPOS,0,0)/100.f;
			if (!BASS_RecordSetInput(input,0,level)) // failed to set input level
				BASS_RecordSetInput(-1,0,level); // try master level instead
		}
		return true;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case 13:
			if (HIWORD(wParam)==CBN_SELCHANGE) 
			{ // input selection changed
				int i;
				input=SendDlgItemMessage(hWnd,13,CB_GETCURSEL,0,0);// get the selection
				// enable the selected input
				for (i=0;BASS_RecordSetInput(i,BASS_INPUT_OFF,-1);i++) ; // 1st disable all inputs, then...
				BASS_RecordSetInput(input,BASS_INPUT_ON,-1); // enable the selected
				UpdateInputInfo(hWnd); // update info
			}
			break;

			case 11:
			BASS_ChannelPlay(chan,TRUE); // play the recorded data
			break;

			case 10:
			if (!rchan)
				StartRecording(hWnd);
			else
				StopRecording(hWnd);
			break;

			case 12:
			SaveRecord(hWnd);
			break;
		}
		return true;
	case WM_TIMER:
		// update the recording/playback counter
		if (rchan) // recording
			sprintf(text,"%I64d",BASS_ChannelGetPosition(rchan,BASS_POS_BYTE));
		else if (chan) 
		{
			if (BASS_ChannelIsActive(chan)) // playing
				sprintf(text,"%I64d / %I64d",BASS_ChannelGetPosition(chan,BASS_POS_BYTE),BASS_ChannelGetLength(chan,BASS_POS_BYTE));
			else
				sprintf(text,"%I64d",BASS_ChannelGetLength(chan,BASS_POS_BYTE));
		}
		SendDlgItemMessage(hWnd,20,WM_SETTEXT,0,(LPARAM)text);
		return true;
	case WM_CLOSE:
		BASS_RecordFree();
		BASS_Free();
		ShowWindow(ghwnd,SW_RESTORE);
		ReInitBass();
		SetFocus(ghwnd);
		KillTimer(hWnd,ID_TIMER2);
		SendMessage(ghwnd,WM_COMMAND,IDC_BUTTON1,0);
		EndDialog(hWnd,0);
		return true;
	}
	return false;
}

void UpdateInputInfo(HWND hWnd)
{
	char *type;
	float level;
	int it=BASS_RecordGetInput(input,&level); // get info on the input
	if (it==-1 || level<0) 
	{ // failed to get level
		BASS_RecordGetInput(-1,&level); // try master input instead
		if (level<0) 
		{ // that failed too
			level=1; // just display 100%
			EnableWindow(GetDlgItem(hWnd,14),false);
		}
		else
			EnableWindow(GetDlgItem(hWnd,14),true);
	}
	else
		EnableWindow(GetDlgItem(hWnd,14),true);

	SendDlgItemMessage(hWnd,14,TBM_SETPOS,true,level*100);
	switch (it&BASS_INPUT_TYPE_MASK) 
	{
		case BASS_INPUT_TYPE_DIGITAL:
			type="digital";
			break;
		case BASS_INPUT_TYPE_LINE:
			type="line-in";
			break;
		case BASS_INPUT_TYPE_MIC:
			type="microphone";
			break;
		case BASS_INPUT_TYPE_SYNTH:
			type="midi synth";
			break;
		case BASS_INPUT_TYPE_CD:
			type="analog cd";
			break;
		case BASS_INPUT_TYPE_PHONE:
			type="telephone";
			break;
		case BASS_INPUT_TYPE_SPEAKER:
			type="pc speaker";
			break;
		case BASS_INPUT_TYPE_WAVE:
			type="wave/pcm";
			break;
		case BASS_INPUT_TYPE_AUX:
			type="aux";
			break;
		case BASS_INPUT_TYPE_ANALOG:
			type="analog";
			break;
		default:
			type="undefined";
	}
	SendDlgItemMessage(hWnd,15,WM_SETTEXT,0,(LPARAM)type);
}

void StopRecording(HWND hWnd)
{
	BASS_ChannelStop(rchan);
	rchan=0;
	SendDlgItemMessage(hWnd,10,WM_SETTEXT,0,(LPARAM)"Record");
	// complete the WAVE header
	*(DWORD*)(recbuf+4)=reclen-8;
	*(DWORD*)(recbuf+40)=reclen-44;
	// enable "save" button
	EnableWindow(GetDlgItem(hWnd,12),true);
	// setup output device (using default device)
	if (!BASS_Init(-1,44100,0,hWnd,NULL)) {
		MessageBox(hWnd,"Can't initialize output device","Error",MB_ICONERROR);
		return;
	}
	// create a stream from the recording
	if (chan=BASS_StreamCreateFile(TRUE,recbuf,0,reclen,0))
		EnableWindow(GetDlgItem(hWnd,11),true); // enable "play" button
	else 
		BASS_Free();
}

void StartRecording(HWND hWnd)
{
	WAVEFORMATEX *wf;
	if (recbuf) 
	{ // free old recording
		BASS_StreamFree(chan);
		chan=0;
		free(recbuf);
		recbuf=NULL;
		EnableWindow(GetDlgItem(hWnd,11),false);
		EnableWindow(GetDlgItem(hWnd,12),false);
		// close output device before recording incase of half-duplex device
		BASS_Free();
	}
	// allocate initial buffer and make space for WAVE header
	recbuf=(char*)malloc(BUFSTEP);
	reclen=44;
	// fill the WAVE header
	memcpy(recbuf,"RIFF\0\0\0\0WAVEfmt \20\0\0\0",20);
	memcpy(recbuf+36,"data\0\0\0\0",8);
	wf=(WAVEFORMATEX*)(recbuf+20);
	wf->wFormatTag=1;
	wf->nChannels=2;
	wf->wBitsPerSample=16;
	wf->nSamplesPerSec=44100;
	wf->nBlockAlign=wf->nChannels*wf->wBitsPerSample/8;
	wf->nAvgBytesPerSec=wf->nSamplesPerSec*wf->nBlockAlign;
	// start recording @ 44100hz 16-bit stereo
	if (!(rchan=BASS_RecordStart(44100,2,0,&RecordingCallback,0)))
	{
		MessageBox(hWnd,"Couldn't start recording","Error",MB_ICONERROR);
		free(recbuf);
		recbuf=0;
		return;
	}
	SendDlgItemMessage(hWnd,10,WM_SETTEXT,0,(LPARAM)"Stop");
}

// buffer the recorded data
BOOL CALLBACK RecordingCallback(HRECORD handle, const void *buffer, DWORD length, void *user)
{
	// increase buffer size if needed
	if ((reclen%BUFSTEP)+length>=BUFSTEP) 
	{
		recbuf=(char*)realloc(recbuf,((reclen+length)/BUFSTEP+1)*BUFSTEP);
		if (!recbuf) 
		{
			rchan=0;
			MessageBox(rechwnd,"Out of memory!","Error",MB_ICONERROR);
			SendDlgItemMessage(rechwnd,10,WM_SETTEXT,0,(LPARAM)"Record");
			return FALSE; // stop recording
		}
	}
	// buffer the data
	memcpy(recbuf+reclen,buffer,length);
	reclen+=length;
	return TRUE; // continue recording
}

void SaveRecord(HWND hWnd)
{
	FILE *fp;
	char file[MAX_PATH]="";
	OPENFILENAME ofn={0};
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=hWnd;
	ofn.nMaxFile=MAX_PATH;
	ofn.lpstrFile=file;
	ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
	ofn.lpstrFilter="WAV files\0*.wav\0All files\0*.*\0\0";
	ofn.lpstrDefExt="wav";
	if (!GetSaveFileName(&ofn)) return;
	if (!(fp=fopen(file,"wb"))) {
		MessageBox(hWnd,"Can't create the file","Error",MB_ICONERROR);
		return;
	}
	fwrite(recbuf,reclen,1,fp);
	fclose(fp);
}

void ReInitBass()
{
	// Initialize default output...
	if(!BASS_Init(-1,44100,0,ghwnd,NULL))
	{
		MessageBox(ghwnd,"Can not re-initialize playback device","Error",MB_ICONERROR);
		SendMessage(ghwnd,WM_CLOSE,NULL,NULL);
	}
	BASS_PluginFree(0);
	InitDecoders();
}

BOOL CALLBACK NetProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		ShowWindow(hWnd,SW_SHOW);
		nethwnd=hWnd;
		BASS_Start();
		BASS_ChannelStop(channel);
		BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST,1); // enable playlist processing
		BASS_SetConfig(BASS_CONFIG_NET_PREBUF,0); // minimize automatic pre-buffering, so we can do it (and display it) instead
		BASS_SetConfigPtr(BASS_CONFIG_NET_PROXY,proxy); // setup proxy server location

		//Setting the slider of volume control
		SendDlgItemMessage(hWnd,IDC_SLIDER1,TBM_SETRANGE,1,MAKELONG(0,100));
		SendDlgItemMessage(hWnd,IDC_SLIDER1,TBM_SETPOS,1,100);
		return true;
	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
		case RADIO_STOP:
			TerminateThread(thrhandle2,0);
			thrhandle2=0;
			break;
			case 41:
				if (SendDlgItemMessage(hWnd,41,BM_GETCHECK,0,0))
					BASS_SetConfigPtr(BASS_CONFIG_NET_PROXY,NULL); // disable proxy
				else
					BASS_SetConfigPtr(BASS_CONFIG_NET_PROXY,proxy); // enable proxy
				break;
			default:
				if ((LOWORD(wParam)>=10 && LOWORD(wParam)<20) || LOWORD(wParam)==21) 
				{
					// already connecting, then close it and reload new request
					SendMessage(hWnd,WM_COMMAND,RADIO_STOP,0);

					GetDlgItemText(hWnd,40,proxy,sizeof(proxy)-1); // get proxy server
					if (LOWORD(wParam)==21) 
					{ // custom stream URL
						char temp[200];
						SendDlgItemMessage(hWnd,20,WM_GETTEXT,sizeof(temp),(LPARAM)temp);
						url=strdup(temp);
					} 
					else // preset
						url=strdup(urls[LOWORD(wParam)-10]);
					// open URL in a new thread (so that main thread is free)
					thrhandle2=CreateThread(0,0,(LPTHREAD_START_ROUTINE)OpenURL,0,0,0);
				}
		}
		return true;
	case WM_TIMER:
		{ // monitor prebuffering progress
			DWORD progress=BASS_StreamGetFilePosition(chan,BASS_FILEPOS_BUFFER)
				*100/BASS_StreamGetFilePosition(chan,BASS_FILEPOS_END); // percentage of buffer filled
			if (progress>75 || !BASS_StreamGetFilePosition(chan,BASS_FILEPOS_CONNECTED)) { // over 75% full (or end of download)
				KillTimer(hWnd,ID_TIMER3); // finished prebuffering, stop monitoring
				{ // get the broadcast name and URL
					const char *icy=BASS_ChannelGetTags(chan,BASS_TAG_ICY);
					if (!icy) icy=BASS_ChannelGetTags(chan,BASS_TAG_HTTP); // no ICY tags, try HTTP
					if (icy) {
						for (;*icy;icy+=strlen(icy)+1) {
							if (!strnicmp(icy,"icy-name:",9))
								SendDlgItemMessage(hWnd,31,WM_SETTEXT,0,(LPARAM)icy+9);
							if (!strnicmp(icy,"icy-url:",8))
								SendDlgItemMessage(hWnd,32,WM_SETTEXT,0,(LPARAM)icy+8);
						}
					} else
						SendDlgItemMessage(hWnd,31,WM_SETTEXT,0,(LPARAM)"");
				}
				// get the stream title and set sync for subsequent titles
				DoMeta();
				BASS_ChannelSetSync(chan,BASS_SYNC_META,0,&MetaSync,0); // Shoutcast
				BASS_ChannelSetSync(chan,BASS_SYNC_OGG_CHANGE,0,&MetaSync,0); // Icecast/OGG
				// set sync for end of stream
				BASS_ChannelSetSync(chan,BASS_SYNC_END,0,&EndSync,0);
				// play it!
				BASS_ChannelPlay(chan,FALSE);
			} else {
				char text[20];
				sprintf(text,"buffering... %d%%",progress);
				SendDlgItemMessage(hWnd,31,WM_SETTEXT,0,(LPARAM)text);
			}
		}
		return true;
	case WM_HSCROLL:
		if(GetDlgCtrlID((HWND)lParam)==IDC_SLIDER1)
		{
			if (lParam && LOWORD(wParam)!=SB_THUMBPOSITION && LOWORD(wParam)!=SB_ENDSCROLL)
			{
				p=SendMessage(HWND(lParam),TBM_GETPOS,0,0);
				BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL,float(p/100.f));
			}
		}
		return true;
	case WM_CLOSE:
		SendMessage(hWnd,WM_COMMAND,IDC_BUTTON1,0);
		BASS_Free();
		TerminateThread(thrhandle2,0);
		ShowWindow(ghwnd,SW_RESTORE);
		ReInitBass();
		SetFocus(ghwnd);
		KillTimer(hWnd,ID_TIMER3);
		EndDialog(hWnd,0);
		return true;
	}
	return false;
}

void __cdecl OpenURL()
{
	KillTimer(nethwnd,ID_TIMER3); // stop prebuffer monitoring
	BASS_StreamFree(chan); // close old stream
	SendDlgItemMessage(nethwnd,31,WM_SETTEXT,0,(LPARAM)"connecting...");
	SendDlgItemMessage(nethwnd,30,WM_SETTEXT,0,(LPARAM)"");
	SendDlgItemMessage(nethwnd,32,WM_SETTEXT,0,(LPARAM)"");
	chan=BASS_StreamCreateURL(url,0,BASS_STREAM_BLOCK|BASS_STREAM_STATUS|BASS_STREAM_AUTOFREE,StatusProc,0); // open URL
	free(url); // free temp URL buffer
	if (!chan) 
	{ // failed to open
		SendDlgItemMessage(nethwnd,31,WM_SETTEXT,0,(LPARAM)"Can not load the radio stream,try again");
		SendMessage(nethwnd,WM_COMMAND,RADIO_STOP,0);
		thrhandle2=0;
	} 
	else
		SetTimer(nethwnd,ID_TIMER3,50,0); // start prebuffer monitoring
	cthread=0;
}

void CALLBACK StatusProc(const void *buffer, DWORD length, void *user)
{
	if (buffer && !length)
		SendDlgItemMessage(nethwnd,32,WM_SETTEXT,0,(LPARAM)buffer); // display connection status
}

void CALLBACK EndSync(HSYNC handle, DWORD channel, DWORD data, void *user)
{
	SendDlgItemMessage(nethwnd,31,WM_SETTEXT,0,(LPARAM)"not playing");
	SendDlgItemMessage(nethwnd,30,WM_SETTEXT,0,(LPARAM)"");
	SendDlgItemMessage(nethwnd,32,WM_SETTEXT,0,(LPARAM)"");
}

void CALLBACK MetaSync(HSYNC handle, DWORD channel, DWORD data, void *user)
{
	DoMeta();
}

// update stream title from metadata
void DoMeta()
{
	const char *meta=BASS_ChannelGetTags(chan,BASS_TAG_META);
	if (meta) 
	{ // got Shoutcast metadata
		char *p=(char*)strstr(meta,"StreamTitle='");
		if (p) 
		{
			p=strdup(p+13);
			strchr(p,';')[-1]=0;
			SendDlgItemMessage(nethwnd,30,WM_SETTEXT,0,(LPARAM)p);
			free(p);
		}
	} 
	else 
	{
		meta=BASS_ChannelGetTags(chan,BASS_TAG_OGG);
		if (meta) 
		{ // got Icecast/OGG tags
			const char *artist=NULL,*title=NULL,*p=meta;
			for (;*p;p+=strlen(p)+1)
			{
				if (!strnicmp(p,"artist=",7)) // found the artist
					artist=p+7;
				if (!strnicmp(p,"title=",6)) // found the title
					title=p+6;
			}
			if (artist) 
			{
				char text[100];
				_snprintf(text,sizeof(text),"%s - %s",artist,title);
				SendDlgItemMessage(nethwnd,30,WM_SETTEXT,0,(LPARAM)text);
			} 
			else if (title)
				SendDlgItemMessage(nethwnd,30,WM_SETTEXT,0,(LPARAM)title);
		}
    }
}

BOOL CALLBACK ShowPlaylistWindow(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	HMENU hPopupMenu;
	POINT p;
	int iSelect;
	char data[200]={0};
	char search[100]={0};
	pl=hWnd;
	switch(message)
	{
	case WM_INITDIALOG:
		if(playtemp || PlayList)
		{
			playlist_hwnd=hWnd;
			proc_list=GetDlgItem(hWnd,IDC_LIST1);
			ListView_SetExtendedListViewStyle(proc_list,LVS_REPORT);
			ListView_SetExtendedListViewStyle(proc_list,LVS_SINGLESEL);
			ListView_SetExtendedListViewStyle(proc_list, LVS_EX_FULLROWSELECT);
			AddColumn(proc_list,"Track", 0,50, LVCFMT_LEFT);
			AddColumn(proc_list,"Name", 1,540, LVCFMT_LEFT);
			alreadyplaylist=true;
			CreateThread(0,0,(LPTHREAD_START_ROUTINE)ShowPlaylist,0,0,0);
		}
		else
		{
			MessageBox(hWnd,"No playlist item is loaded or not playing from a folder/drive","Error",MB_ICONEXCLAMATION);
			SendMessage(hWnd,WM_CLOSE,0,0);
		}
		return true;
	case WM_NOTIFY:
        if(((LPNMHDR)lParam)->hwndFrom == proc_list && ((LPNMHDR)lParam)->code == NM_RCLICK)
        {                       
           iSelect = SendMessage(proc_list, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);
           ListView_GetItemText(proc_list, iSelect, 0, data, sizeof(data))
		   //  MessageBox(hWnd,data,0,0); // data contents track number, using this we can remove track

           if(iSelect >= 0)
           {  
              GetCursorPos(&p);
              hPopupMenu = CreatePopupMenu();
			  InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 2, "Goto Song location");
              InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, "Remove track");
              int choice = TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, p.x, p.y+23, 0, hWnd, NULL);
              
              switch(choice)
              {
                 case 1:
					 RemoveTrack(data);
                      break;
				 case 2:
					 {
						char temp[MAX_PATH]={0};
						char* open=new char[1024];
						lstrcpy(open," /select, ");
						GetPrivateProfileString("songs",data,NULL,temp,sizeof temp,playlist_path);
						lstrcat(open,temp);
						ShellExecute(hWnd,"OPEN","explorer.exe",open,NULL,SW_SHOWMAXIMIZED);
						delete [] open;
					 }
					 break;
              }
              
              DestroyMenu(hPopupMenu);
           }
		   
		   ListView_SetItemState(proc_list, iSelect, FALSE, LVIS_FOCUSED);
		}
		if(((LPNMHDR)lParam)->hwndFrom == proc_list && ((LPNMHDR)lParam)->code == NM_DBLCLK)
        {                       
           iSelect = SendMessage(proc_list, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);

           ListView_GetItemText(proc_list, iSelect, 0, data, sizeof(data))


           if(iSelect >= 0)
           {  
			   index=atoi(data);
			   PlayByIndex(index);
           }
           
           ListView_SetItemState(proc_list, iSelect, FALSE, LVIS_FOCUSED);
        }
        return false;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON1:
			AddaSong(hWnd);
			break;
		case IDC_BUTTON2:
			{
				ZeroMemory(fldr,sizeof fldr);
				GetFolderSelection(hWnd,fldr,"Select a folder or drive containing music files");
				if(lstrlen(fldr))
				{
					SetWindowText(hWnd,"Please wait, searching files...");
					TerminateThread(thrhandle5,0);
					thrhandle5=CreateThread(0,0,(LPTHREAD_START_ROUTINE)AddaFolder,0,0,0);
				}
			}
			break;
		case IDC_BUTTON3:
			{
				int select=0;
				if(!lastfirst)
				{
					select=index-1;
					// Shows and highlight the current playing song
					ListView_SetItemState(proc_list,-1,0,LVIS_SELECTED); //Deselect all items in list
					SendMessage(proc_list,LVM_ENSUREVISIBLE ,(WPARAM)select,FALSE); // if item is far, scroll to it
					ListView_SetItemState(proc_list,select,LVIS_FOCUSED ,LVIS_FOCUSED); // set focus
					ListView_SetItemState(proc_list,select,LVIS_SELECTED ,LVIS_SELECTED); // select item
				}
				if(lastfirst)
				{
					select=MaxIndex-index;
					// Shows and highlight the current playing song
					ListView_SetItemState(proc_list,-1,0,LVIS_SELECTED); //Deselect all items in list
					SendMessage(proc_list,LVM_ENSUREVISIBLE ,(WPARAM)select,FALSE); // if item is far, scroll to it
					ListView_SetItemState(proc_list,select,LVIS_FOCUSED ,LVIS_FOCUSED); // set focus
					ListView_SetItemState(proc_list,select,LVIS_SELECTED ,LVIS_SELECTED); // select item
				}
			}
			break;
		case IDC_CHECK1:
			if(IsDlgButtonChecked(hWnd,IDC_CHECK1)==BST_CHECKED)
			{
				lastfirst=true;
				CreateThread(0,0,(LPTHREAD_START_ROUTINE)ShowPlaylist,0,0,0);
			}
			else
			{
				lastfirst=false;
				CreateThread(0,0,(LPTHREAD_START_ROUTINE)ShowPlaylist,0,0,0);
			}
			break;
		}
		return false;
	case WM_CLOSE:
		if(alreadyplaylist)
			ShowWindow(hWnd,SW_HIDE);
		else
		{
			alreadyplaylist=false;
			EndDialog(hWnd,0);
			TerminateThread(thrhandle5,0);
		}
		return true;
	}
	return false;
}

void RemoveTrack(char index[])
{
	char temp[MAX_PATH]; ZeroMemory(temp,sizeof temp);
	GetPrivateProfileString("songs",index,NULL,temp,sizeof temp,buffer);
	lstrcat(temp,".skip");
	WritePrivateProfileString("songs",index,temp,buffer);
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)ShowPlaylist,0,0,0);
}

void AddaSong(HWND hWnd)
{
	int max;
	char dump[MAX_PATH],row[10];
	ZeroMemory(dump,sizeof dump);
	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=hWnd;
	ofn.nMaxFile=MAX_PATH;
	ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
	ofn.lpstrFilter="Playable files\0*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif;*.mo3;*.it;*.xm;*.s3m;*.mtm;*.mod;*.umx;*.cda;*.fla;*.flac;*.oga;*.ogg;*.wma;*.wv;*.aac;*.m4a;*.m4b;*.mp4;*.ac3;*.adx;*.aix;*.ape;*.mac;*.mpc;*mp+;*.mpp;*.ofr;*.ofs;*.tta\0\0";
	ofn.lpstrFile = dump;
	GetOpenFileName(&ofn);
	if(lstrlen(dump))
	{
		ZeroMemory(row,sizeof row);
		GetPrivateProfileString("Total_Files","Total",NULL,row,sizeof row,buffer);
		max=atoi(row);
		max++;
		ZeroMemory(row,sizeof row);
		wsprintf(row,"%d",max);
		WritePrivateProfileString("songs",row,dump,buffer);
		WritePrivateProfileString("Total_Files","Total",row,buffer);
		MessageBox(hWnd,"Song successfully added in playlist","Done",MB_ICONINFORMATION);
		CreateThread(0,0,(LPTHREAD_START_ROUTINE)ShowPlaylist,0,0,0);
		MaxIndex=max;
		N=max;
	}

}

void PlayByIndex(int index)
{
	char row[10]={};
	ZeroMemory(row,sizeof row);
	wsprintf(row,"%d",index);
	ZeroMemory(CmdLine,sizeof CmdLine);
	GetPrivateProfileString("songs",row,NULL,CmdLine,sizeof CmdLine,playlist_path);
	PlaySong(ghwnd,CmdLine);
	MonitorTrack=true;
	Parsed=true;
}

void AddColumn(HWND hView,char  text[], int Col, int Width, DWORD dStyle)
{
     LVCOLUMN lvc = { 0 };
     memset(&lvc, 0, sizeof(LVCOLUMN));
     lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH  | LVCF_FMT;
     lvc.fmt  = dStyle;
     
     lvc.iSubItem = Col;
     lvc.cx       = Width;
     lvc.pszText  = text;
     
     ListView_InsertColumn(hView, Col, &lvc);
}

void AddIndex(HWND hView, int Index)
{
     LVITEM lv;
     memset(&lv, 0, sizeof(LVITEM));
     
     lv.mask = LVIF_TEXT;
     lv.iSubItem = 0; 
     lv.pszText = LPSTR_TEXTCALLBACK;
     lv.iItem = Index; 
     
     ListView_InsertItem(proc_list, &lv);
}

void AddRow(HWND hView,char text[], int Index, int Col)
{
     ListView_SetItemText(hView, Index, Col, text);
}

void ShowPlaylist()
{
	char row[10]={0};
	char path[256],Song[256];
	ListView_DeleteAllItems(proc_list);
	GetPrivateProfileString("Total_Files","Total",NULL,row,sizeof row,playlist_path);
	int x=atoi(row);
	SetWindowText(playlist_hwnd,"Please wait... processing");
	if(lastfirst)
	{
		CheckDlgButton(pl,IDC_CHECK1,BST_CHECKED);
		for(int i=1;i<=x;i++)
		{
			ZeroMemory(path,sizeof path);
			wsprintf(row,"%d",i);
			GetPrivateProfileString("songs",row,NULL,path,sizeof path,playlist_path);
			ZeroMemory(Song,sizeof Song);
			if(!IsSkip(path)) // Check and exclude songs that are skipped/deleted from playlists
			{
				lstrcpy(Song,GetName(path));
				AddIndex(proc_list, 0);
				AddRow(proc_list, row, 0, 0);
				AddRow(proc_list, Song, 0, 1);
			}
		}
	}
	else
	{
		CheckDlgButton(pl,IDC_CHECK1,BST_UNCHECKED);
		for(int i=x;i>=1;i--) // decremental loop shows the playlist correctly, first song in list in first position in list control and last in last
		{
			ZeroMemory(path,sizeof path);
			wsprintf(row,"%d",i);
			GetPrivateProfileString("songs",row,NULL,path,sizeof path,playlist_path);
			ZeroMemory(Song,sizeof Song);
			if(!IsSkip(path)) // Check and exclude songs that are skipped/deleted from playlists
			{
				lstrcpy(Song,GetName(path));
				AddIndex(proc_list, 0);
				AddRow(proc_list, row, 0, 0);
				AddRow(proc_list, Song, 0, 1);
			}
		}
	}

	SetWindowText(playlist_hwnd,"Playlist [ Double click to play, Right click to remove track ]");
	ExitThread(0);
}

char* GetName(char path[])
{
	char *name=strrchr(path,'\\');
	char temp;
	int CmdLineLen=lstrlen(name);
	for(int i=1;i<CmdLineLen;i++)
	{
		temp=name[i];
		name[i-1]=temp;
	}
	name[CmdLineLen-1]='\0';
	return name;
}

void AddaFolder()
{
	GetPrivateProfileString("Total_Files","Total",NULL,row,sizeof row,buffer);
	int x=atoi(row);

	char* ext1[]={".mp3"};
	char* ext2[]={".mp2"};
	char* ext3[]={".mp1"};
	char* ext4[]={".ogg"};
	char* ext5[]={".wav"};
	char* ext6[]={".aif"};
	char* ext7[]={".mo3"};
	char* ext8[]={".it"};
	char* ext9[]={".xm"};
	char* ext10[]={".s3m"};
	char* ext11[]={".mtm"};
	char* ext12[]={".mod"};
	char* ext13[]={".umx"};
	char* ext14[]={".cda"};
	char* ext15[]={".fla"};
	char* ext16[]={".flac"};
	char* ext17[]={".oga"};
	char* ext18[]={".ogg"};
	char* ext19[]={".wma"};
	char* ext20[]={".wv"};
	char* ext21[]={".aac"};
	char* ext22[]={".m4a"};
	char* ext23[]={".m4b"};
	char* ext24[]={".mp4"};
	char* ext25[]={".ac3"};
	char* ext26[]={".adx"};
	char* ext27[]={".aix"};
	char* ext28[]={".ape"};
	char* ext29[]={".mac"};
	char* ext30[]={".mpc"};
	char* ext31[]={".mp+"};
	char* ext32[]={".mpp"};
	char* ext33[]={".ofr"};
	char* ext34[]={".ofs"};
	char* ext35[]={".tta"};
	
	SetNumSongs(x);
	ext_search(fldr,*ext1,buffer,false);
	ext_search(fldr,*ext2,buffer,false);
	ext_search(fldr,*ext3,buffer,false);
	ext_search(fldr,*ext4,buffer,false);
	ext_search(fldr,*ext5,buffer,false);
	ext_search(fldr,*ext6,buffer,false);
	ext_search(fldr,*ext7,buffer,false);
	ext_search(fldr,*ext8,buffer,false);
	ext_search(fldr,*ext9,buffer,false);
	ext_search(fldr,*ext10,buffer,false);
	ext_search(fldr,*ext11,buffer,false);
	ext_search(fldr,*ext12,buffer,false);
	ext_search(fldr,*ext13,buffer,false);
	ext_search(fldr,*ext14,buffer,false);
	ext_search(fldr,*ext15,buffer,false);
	ext_search(fldr,*ext16,buffer,false);
	ext_search(fldr,*ext17,buffer,false);
	ext_search(fldr,*ext18,buffer,false);
	ext_search(fldr,*ext19,buffer,false);
	ext_search(fldr,*ext20,buffer,false);
	ext_search(fldr,*ext21,buffer,false);
	ext_search(fldr,*ext22,buffer,false);
	ext_search(fldr,*ext23,buffer,false);
	ext_search(fldr,*ext24,buffer,false);
	ext_search(fldr,*ext25,buffer,false);
	ext_search(fldr,*ext26,buffer,false);
	ext_search(fldr,*ext27,buffer,false);
	ext_search(fldr,*ext28,buffer,false);
	ext_search(fldr,*ext29,buffer,false);
	ext_search(fldr,*ext30,buffer,false);
	ext_search(fldr,*ext31,buffer,false);
	ext_search(fldr,*ext32,buffer,false);
	ext_search(fldr,*ext33,buffer,false);
	ext_search(fldr,*ext34,buffer,false);
	ext_search(fldr,*ext35,buffer,false);

	SetWindowText(pl,"Playlist [ Double click to play, Right click to remove track ]");

	Nm=TotalSongs();

	ZeroMemory(row,sizeof row);
	wsprintf(row,"%d",Nm);
	WritePrivateProfileString("Total_Files","Total",row,buffer);

	MessageBox(pl,"New songs added successfully in current playlist","Done",MB_ICONINFORMATION);

	ShowPlaylist();

	MaxIndex=Nm;
	N=Nm;
}

bool CanIPlay()
{
	char *extn=PathFindExtension(CmdLine);
	if(extn)
	{
		strlwr(extn);
		if(strcmp(extn,".mp2")==0 || strcmp(extn,".mp1")==0 || strcmp(extn,".ogg")==0 || strcmp(extn,".wav")==0 || strcmp(extn,".aif")==0 || strcmp(extn,".mo3")==0 || strcmp(extn,".it")==0 || strcmp(extn,".xm")==0 || strcmp(extn,".s3m")==0 || strcmp(extn,".mtm")==0 || strcmp(extn,".mod")==0 || strcmp(extn,".umx")==0 || strcmp(extn,".mp3")==0 || strcmp(extn,".tpp")==0 || strcmp(extn,".cda")==0|| strcmp(extn,".fla")==0|| strcmp(extn,".flac")==0|| strcmp(extn,".oga")==0|| strcmp(extn,".ogg")==0|| strcmp(extn,".wma")==0|| strcmp(extn,".wv")==0|| strcmp(extn,".aac")==0|| strcmp(extn,".m4a")==0|| strcmp(extn,".m4b")==0|| strcmp(extn,".mp4")==0|| strcmp(extn,".ac3")==0|| strcmp(extn,".adx")==0|| strcmp(extn,".aix")==0|| strcmp(extn,".ape")==0|| strcmp(extn,".mac")==0|| strcmp(extn,".mpc")==0|| strcmp(extn,".mp+")==0|| strcmp(extn,".mpp")==0|| strcmp(extn,".ofr")==0|| strcmp(extn,".ofs")==0|| strcmp(extn,".tta")==0)
		{
			return true;
		}
		else
		{
			lstrcpy(fldr,CmdLine);
			SetWindowText(ghwnd,"Please wait, searching files...");
			playtemp=true;
			NoPlay=false;
			TerminateThread(thrHandle3,0);
			Parsed=false;
			thrHandle3=CreateThread(0,0,(LPTHREAD_START_ROUTINE)PreparePlayList,0,0,0);
		}
		return false;
	}
	else
		return false;
}

void InitDecoders()
{
	BASS_PluginLoad("decoders\\bass_aac.dll",0);
	BASS_PluginLoad("decoders\\bass_ac3.dll",0);
	BASS_PluginLoad("decoders\\bass_adx.dll",0);
	BASS_PluginLoad("decoders\\bass_aix.dll",0);
	BASS_PluginLoad("decoders\\bass_alac.dll",0);
	BASS_PluginLoad("decoders\\bass_ape.dll",0);
	BASS_PluginLoad("decoders\\bass_mpc.dll",0);
	BASS_PluginLoad("decoders\\bass_ofr.dll",0);
	BASS_PluginLoad("decoders\\bass_tta.dll",0);
	BASS_PluginLoad("decoders\\basscd.dll",0);
	BASS_PluginLoad("decoders\\bassflac.dll",0);
	BASS_PluginLoad("decoders\\basswma.dll",0);
	BASS_PluginLoad("decoders\\basswv.dll",0);
}

BOOL CALLBACK FullVisProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		MessageBox(hWnd,"To exit Full Screen mode, press Escape [ESC] Key !","Note !",MB_ICONINFORMATION);
		ShowWindow(hWnd,SW_MAXIMIZE);
		SetWindowPos(hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
		GetClientRect(hWnd,&rc);
		ShowCursor(FALSE);
		RedrawSFXVisuals(hWnd,"vis_plugins\\rabbithole11.svp");
		return true;
	case WM_RBUTTONDOWN:
		if(MonitorTrack)
		{
			if(count==0)
			{
				RedrawSFXVisuals(hWnd,"vis_plugins\\20.svp");
				count=1;
				return true;
			}
			if(count==1)
			{
				RedrawSFXVisuals(hWnd,"vis_plugins\\03.svp");
				count=2;
				return true;
			}
			if(count==2)
			{
				RedrawSFXVisuals(hWnd,"vis_plugins\\06.svp");
				count=3;
				return true;
			}
			if(count==3)
			{
				RedrawSFXVisuals(hWnd,"vis_plugins\\07.svp");
				count=4;
				return true;
			}
			if(count==4)
			{
				RedrawSFXVisuals(hWnd,"vis_plugins\\08.svp");
				count=6;
				return true;
			}
			if(count==6)
			{
				RedrawSFXVisuals(hWnd,"vis_plugins\\17.svp");
				count=7;
				return true;
			}
			if(count==7)
			{
				RedrawSFXVisuals(hWnd,"vis_plugins\\corona.svp");
				count=8;
				return true;
			}
			if(count==8)
			{
				RedrawSFXVisuals(hWnd,"vis_plugins\\rabbithole11.svp");
				count=0;
				return true;
			}
		}
		return true;
	case WM_CTLCOLORDLG:
		return (LONG)CreateSolidBrush(RGB(0, 0, 0));
	case WM_TIMER:
		if(wParam==ID_TIMER4)
		{
			if(hsfx > 0)
			{
				HDC hdc = GetDC(hWnd);
				BASS_SFX_PluginRender(hsfx, channel, hdc);
				ReleaseDC(hWnd, hdc);
			}
		}
		return true;
	case WM_DESTROY:
	case WM_CLOSE:
		ShowCursor(TRUE);
		KillTimer(hWnd,ID_TIMER4);
		BASS_SFX_PluginStop(hsfx);
		BASS_SFX_PluginFree(hsfx);
		if(Vis==1)
			SendMessage(ghwnd,WM_COMMAND,ID_VIS_BLAZE,0);
		if(Vis==2)
			SendMessage(ghwnd,WM_COMMAND,ID_VIS_AROTA,0);
		if(Vis==3)
			SendMessage(ghwnd,WM_COMMAND,ID_VIS_METERX,0);
		if(Vis==4)
			SendMessage(ghwnd,WM_COMMAND,ID_ALBUM_ART,0);
		EndDialog(hWnd,0);
		ShowWindow(ghwnd,SW_MINIMIZE);
		return true;
	}
	if(GetAsyncKeyState(VK_ESCAPE))
		SendMessage(hWnd,WM_CLOSE,0,0);
	return false;
}

void RedrawSFXVisuals(HWND hWnd,char* path)
{
	KillTimer(hWnd,ID_TIMER4);
	BASS_SFX_PluginStop(hsfx);
	BASS_SFX_PluginFree(hsfx);
	hsfx = BASS_SFX_PluginCreate(path,hWnd,rc.right -rc.left ,rc.bottom -rc.top ,BASS_SFX_SONIQUE_OPENGL_DOUBLEBUFFER);
	BASS_SFX_PluginSetStream(hsfx,channel);
	BASS_SFX_PluginStart(hsfx);
	SetTimer(hWnd,ID_TIMER4,27,NULL);
}

void InitDATA()
{
	char temp2[MAX_PATH];
	char temp[3];
	SHGetSpecialFolderPath(ghwnd,settings_path,CSIDL_LOCAL_APPDATA,false);
	lstrcat(settings_path,"\\tp_settings.dll");
	lstrcpy(settings_path2,settings_path);
	OFSTRUCT info;
	menu=GetMenu(ghwnd);
	CheckMenuItem(menu,ID_REMEMBER_TRACKREMEMBERON,MF_UNCHECKED);
	CheckMenuItem(menu,ID_REMEMBER_VOLUMEREMEMBERON,MF_UNCHECKED);
	CheckMenuItem(menu,ID_VIS_OFF,MF_CHECKED);

	if(OpenFile(settings_path,&info,OF_EXIST)==HFILE_ERROR)
	{
		MessageBox(ghwnd,"Welcome to Tejash Player\nThe free and light music player\nThis is your first run,please read the read me and changes log for more informations.","Welcome",MB_ICONEXCLAMATION);
		SendMessage(ghwnd,WM_COMMAND,ID_SKINS_MOOSE,0);

		// Setting the range and position of Slider
		SendDlgItemMessage(ghwnd,IDC_SLIDER1,TBM_SETRANGE,1,MAKELONG(0,100));
		SendDlgItemMessage(ghwnd,IDC_SLIDER1,TBM_SETPOS,1,100);

		ZeroMemory(temp,sizeof temp);
		wsprintf(temp,"%d",disable);

		WritePrivateProfileString("vrem","val",temp,settings_path);
		WritePrivateProfileString("trem","val",temp,settings_path);

		ZeroMemory(temp,sizeof temp);
		wsprintf(temp,"%d",1);
		WritePrivateProfileString("notify","val",temp,settings_path);
	}
	else
	{
		GetPrivateProfileString("skins","data",NULL,temp2,sizeof temp2,settings_path);
		int x=atoi(temp2);
		InvokeSkin(x);

		ZeroMemory(temp2,sizeof temp2);
		GetPrivateProfileString("vrem","val",NULL,temp2,sizeof temp2,settings_path);
		x=atoi(temp2);
		if(x==1) // Volume rem on
		{
			ZeroMemory(temp2,sizeof temp2);
			GetPrivateProfileString("volume","pos",NULL,temp2,sizeof temp2,settings_path);
			p=atof(temp2);
			SendDlgItemMessage(ghwnd,IDC_SLIDER1,TBM_SETRANGE,1,MAKELONG(0,100));
			SendDlgItemMessage(ghwnd,IDC_SLIDER1,TBM_SETPOS,1,(LPARAM)p);
			CheckMenuItem(menu,ID_REMEMBER_VOLUMEREMEMBERON,MF_CHECKED);
		}
		else
		{
			// Setting the range and position of Slider
			SendDlgItemMessage(ghwnd,IDC_SLIDER1,TBM_SETRANGE,1,MAKELONG(0,100));
			SendDlgItemMessage(ghwnd,IDC_SLIDER1,TBM_SETPOS,1,100);
		}

		ZeroMemory(temp2,sizeof temp2);
		GetPrivateProfileString("trem","val",NULL,temp2,sizeof temp2,settings_path);
		x=atoi(temp2);
		if(x==1) //track is remembered....
		{
			trackrem=true; // enable switch to remember songs...
			CheckMenuItem(menu,ID_REMEMBER_TRACKREMEMBERON,MF_CHECKED);

			// Check if last song is played from playlist...
			ZeroMemory(temp2,sizeof temp2);
			GetPrivateProfileString("IsPlaylist","val",NULL,temp2,sizeof temp2,settings_path);
			x=atoi(temp2);
			if(!PlayFlag) // if a song is passed to play then ignore last one...
			{
				if(x==1)
				{
					// play songs from playlist
					ZeroMemory(temp2,sizeof temp2);
					GetPrivateProfileString("IsPlaylist","index",NULL,temp2,sizeof temp2,settings_path);
					x=atoi(temp2);

					ZeroMemory(temp2,sizeof temp2);
					GetPrivateProfileString("IsPlaylist","path",NULL,temp2,sizeof temp2,settings_path);
					char* ext=PathFindExtension(temp2);
					if(lstrcmp(ext,".ini") != 0)
						ParsePlayList(temp2);
					else
					{
						MonitorTrack=false;
						return;
					}

					char temp1[2048]={0};
					//x--;
					index=x;
					wsprintf(row,"%d",x);
					GetPrivateProfileString("songs",row,NULL,temp1,sizeof temp1,playlist_path);
					lastindex=index;
					ZeroMemory(lastsong,sizeof lastsong);
					lstrcpy(lastsong,temp1);
					PlaySong(ghwnd,temp1);

					// update last time too
					ZeroMemory(temp2,sizeof temp2);
					GetPrivateProfileString("track","time",NULL,temp2,sizeof temp2,settings_path);
					x=atoi(temp2);
					BASS_ChannelSetPosition(channel,BASS_ChannelSeconds2Bytes(channel,x),BASS_POS_BYTE);
					// update position of slider (seek bar)
					SendDlgItemMessage(ghwnd,IDC_SLIDER2,TBM_SETPOS,1,(DWORD)BASS_ChannelBytes2Seconds(channel,BASS_ChannelGetPosition(channel,BASS_POS_BYTE)));
				}
				else
				{
					ZeroMemory(temp2,sizeof temp2);
					GetPrivateProfileString("track","data",NULL,temp2,sizeof temp2,settings_path);
					PlaySong(ghwnd,temp2);

					// update last time too
					ZeroMemory(temp2,sizeof temp2);
					GetPrivateProfileString("track","time",NULL,temp2,sizeof temp2,settings_path);
					x=atoi(temp2);
					BASS_ChannelSetPosition(channel,BASS_ChannelSeconds2Bytes(channel,x),BASS_POS_BYTE);
					// update position of slider (seek bar)
					SendDlgItemMessage(ghwnd,IDC_SLIDER2,TBM_SETPOS,1,(DWORD)BASS_ChannelBytes2Seconds(channel,BASS_ChannelGetPosition(channel,BASS_POS_BYTE)));
				}
			}
		}
		ZeroMemory(temp2,sizeof temp2);
		GetPrivateProfileString("notify","val",NULL,temp2,sizeof temp2,settings_path);
		x=atoi(temp2);
		if(x==0)
		{
			notify=false;
			CheckMenuItem(menu,ID_SETTINGS_NOTIFYABOUTCURRENTTRACK,MF_UNCHECKED);
		}
		if(x==1)
		{
			notify=true;
			CheckMenuItem(menu,ID_SETTINGS_NOTIFYABOUTCURRENTTRACK,MF_CHECKED);
		}
	}
}

void DeselectSkinTick()
{
	CheckMenuItem(menu,ID_SKINS_OFF,MF_UNCHECKED);
	CheckMenuItem(menu,ID_SKINS_GLOW,MF_UNCHECKED);
	CheckMenuItem(menu,ID_SKINS_ZIPPO,MF_UNCHECKED);
	CheckMenuItem(menu,ID_SKINS_INDIGO,MF_UNCHECKED);
	CheckMenuItem(menu,ID_SKINS_JARILO,MF_UNCHECKED);
	CheckMenuItem(menu,ID_SKINS_MACOS,MF_UNCHECKED);
	CheckMenuItem(menu,ID_SKINS_TRANSILVANIA,MF_UNCHECKED);
	CheckMenuItem(menu,ID_SKINS_V,MF_UNCHECKED);
	CheckMenuItem(menu,ID_SKINS_ACCENT,MF_UNCHECKED);
	CheckMenuItem(menu,ID_SKINS_MOOSE,MF_UNCHECKED);
	CheckMenuItem(menu,ID_SKINS_DEVOIR,MF_UNCHECKED);
}

void InvokeSkin(int x)
{
	if(x==0)
	{
		DeselectSkinTick();
		SendMessage(ghwnd,WM_COMMAND,ID_SKINS_OFF,0);
		CheckMenuItem(menu,ID_SKINS_OFF,MF_CHECKED);
	}
	if(x==1)
	{
		DeselectSkinTick();
		SendMessage(ghwnd,WM_COMMAND,ID_SKINS_GLOW,0);
	}
	if(x==2)
	{
		DeselectSkinTick();
		SendMessage(ghwnd,WM_COMMAND,ID_SKINS_INDIGO,0);
	}
	if(x==3)
	{
		DeselectSkinTick();
		SendMessage(ghwnd,WM_COMMAND,ID_SKINS_JARILO,0);
	}
	if(x==4)
	{
		DeselectSkinTick();
		SendMessage(ghwnd,WM_COMMAND,ID_SKINS_MACOS,0);
	}
	if(x==5)
	{
		DeselectSkinTick();
		SendMessage(ghwnd,WM_COMMAND,ID_SKINS_TRANSILVANIA,0);
	}
	if(x==6)
	{
		DeselectSkinTick();
		SendMessage(ghwnd,WM_COMMAND,ID_SKINS_V,0);
	}
	if(x==7)
	{
		DeselectSkinTick();
		SendMessage(ghwnd,WM_COMMAND,ID_SKINS_ACCENT,0);
	}
	if(x==8)
	{
		DeselectSkinTick();
		SendMessage(ghwnd,WM_COMMAND,ID_SKINS_ZIPPO,0);
	}
	if(x==9)
	{
		DeselectSkinTick();
		SendMessage(ghwnd,WM_COMMAND,ID_SKINS_MOOSE,0);
	}
	if(x==10)
	{
		DeselectSkinTick();
		SendMessage(ghwnd,WM_COMMAND,ID_SKINS_DEVOIR,0);
	}
}

Gdiplus::Bitmap* ResizeClone(Bitmap *bmp, INT width, INT height,bool PreserveAspectRatio)
{
    UINT o_height = bmp->GetHeight();
    UINT o_width = bmp->GetWidth();
    INT n_width = width;
    INT n_height = height;
    double ratio = ((double)o_width) / ((double)o_height);
	if(PreserveAspectRatio)
	{
		if (o_width > o_height) {
			// Resize down by width
			n_height = static_cast<UINT>(((double)n_width) / ratio);
		} else {
			n_width = static_cast<UINT>(n_height * ratio);
		}
	}
    Gdiplus::Bitmap* newBitmap = new Gdiplus::Bitmap(n_width, n_height, bmp->GetPixelFormat());
    Gdiplus::Graphics graphics(newBitmap);
    graphics.DrawImage(bmp, 0, 0, n_width, n_height);
    return newBitmap;
}

void GetAlbumArt(char* song)
{
	char backup[MAX_PATH];
	lstrcpy(backup,song);
	char localfolder[MAX_PATH];
	SHGetSpecialFolderPath(ghwnd,localfolder,CSIDL_LOCAL_APPDATA,false);
	lstrcat(localfolder,"\\image.jpg");

	// Removing image...
	HWND hPic = GetDlgItem(ghwnd,ID_VIS);
	SendMessage(hPic, STM_SETIMAGE, IMAGE_BITMAP,0); 

	if(no_image)
	{
		if (gpBitmap) delete gpBitmap;
		if(bmp) delete bmp;
	}
	// Restart GDI+
	GdiplusShutdown(gdiplusToken);
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	gpBitmap = NULL;

	ID3_Tag tag(backup);
	DeleteFile(localfolder);
	const ID3_Frame* frame = tag.Find(ID3FID_PICTURE);
	if (frame && frame->Contains(ID3FN_DATA))
	{
		frame->Field(ID3FN_DATA).ToFile(localfolder);
		no_image=true;
		
		if(gpBitmap) delete gpBitmap;
		const WCHAR* wdata;
		int nChars = MultiByteToWideChar(CP_ACP, 0, localfolder, -1, NULL, 0);
		wdata = new WCHAR[nChars];
		MultiByteToWideChar(CP_ACP, 0, localfolder, -1, (LPWSTR)wdata, nChars);
		gpBitmap = new Bitmap(wdata);
		delete [] wdata;

		bmp=ResizeClone(gpBitmap,rect.right - rect.left,rect.bottom - rect.top,false);
		// We need to force the window to redraw itself
		InvalidateRect(GetDlgItem(ghwnd,ID_VIS), NULL, TRUE);

		// Getting HBITMAP from Bitmap
		HBITMAP hbmp;
		bmp->GetHBITMAP(0,&hbmp);
		SendMessage(GetDlgItem(ghwnd,ID_VIS),STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)hbmp);
	}
	else
	{
		no_image=false;
		HANDLE hBitmap;
		hBitmap = LoadImage(hIns,MAKEINTRESOURCE(IDB_BITMAP3), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
		HWND hPic = GetDlgItem(ghwnd,ID_VIS);
		SendMessage(hPic, STM_SETIMAGE, IMAGE_BITMAP, LPARAM(hBitmap)); 
	}
}

BOOL CALLBACK GotoTrackProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	int track=0;
	char err1[100]="Maximum song is this playlist is ";
	char x[150];
	wsprintf(x,"%d",MaxIndex);
	lstrcat(err1,x);
	BOOL read=false;
	switch(message)
	{
	case WM_INITDIALOG:
		if(alreadygoto)
			SendMessage(hWnd,WM_CLOSE,0,0);
		else
			alreadygoto=true;

		SetFocus(hWnd);
		SetForegroundWindow(hWnd);
		return true;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case ID_TRACK_GO:
				track=GetDlgItemInt(hWnd,IDC_TRACK,&read,false);
				if(read)
				{
					if(track>MaxIndex)
						MessageBox(hWnd,err1,"Error",MB_ICONERROR);
					else if(track==0)
						MessageBox(hWnd,"Track number must be greater than Zero (0)","Error",MB_ICONERROR);
					else
					{
						char temp[MAX_PATH]={0};
						index=track;
						wsprintf(row,"%d",track);
						GetPrivateProfileString("songs",row,NULL,temp,sizeof temp,playlist_path);
						lastindex=index;
						ZeroMemory(lastsong,sizeof lastsong);
						lstrcpy(lastsong,temp);
						PlaySong(ghwnd,temp);
					}
				}
				else
					MessageBox(hWnd,"Unable to read track number.\nEither it's too big number or Invalid value","Error",MB_ICONERROR);
				break;
			case ID_TRACK_GO2:
				track=GetDlgItemInt(hWnd,IDC_TRACK,&read,false);
				if(read)
				{
					if(track>MaxIndex)
						MessageBox(hWnd,err1,"Error",MB_ICONERROR);
					else if(track==0)
						MessageBox(hWnd,"Track number must be greater than Zero (0)","Error",MB_ICONERROR);
					else
					{
						wsprintf(row,"%d",track);
						char temp[MAX_PATH]={0};
						char* open=new char[1024];
						lstrcpy(open," /select, ");
						GetPrivateProfileString("songs",row,NULL,temp,sizeof temp,playlist_path);
						lstrcat(open,temp);
						ShellExecute(hWnd,"OPEN","explorer.exe",open,NULL,SW_SHOWMAXIMIZED);
						delete [] open;
					}
				}
				else
					MessageBox(hWnd,"Unable to read track number.\nEither it's too big number or Invalid value","Error",MB_ICONERROR);
				break;
			}
		}
		return true;
	case WM_CLOSE:
		alreadygoto=false;
		EndDialog(hWnd,0);
		return true;
	}
	return false;
}

void LoadFileInResource(int name, int type, DWORD& size, const char*& data)
{
    HMODULE handle = GetModuleHandle(NULL);
    HRSRC rc = FindResource(handle, MAKEINTRESOURCE(name),MAKEINTRESOURCE(type));
    HGLOBAL rcData = LoadResource(handle, rc);
    size = SizeofResource(handle, rc);
    data = static_cast<const char*>(LockResource(rcData));
}

void DeselectVisTick()
{
	CheckMenuItem(menu,ID_VIS_OFF,MF_UNCHECKED);
	CheckMenuItem(menu,ID_ALBUM_ART,MF_UNCHECKED);
	CheckMenuItem(menu,ID_VIS_AROTA,MF_UNCHECKED);
	CheckMenuItem(menu,ID_VIS_BLAZE,MF_UNCHECKED);
	CheckMenuItem(menu,ID_VIS_METERX,MF_UNCHECKED);
}