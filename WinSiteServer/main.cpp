//#include <windows.h>
#include <stdlib.h>
#include <windowsx.h>

#include "resource.h"
#include "../HTTPServer/TCPServer.h"

TCPServer* Server;

INT_PTR CALLBACK /*LONG WINAPI*/ DlgProc(HWND, UINT, WPARAM, LPARAM);
void DlgOnCommand(HWND, int, HWND, UINT);
BOOL DlgOnInitDialog(HWND, HWND, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	/*HWND hwnd;
	MSG msg;
	WNDCLASS w;

	memset(&w, 0, sizeof(w));

	w.style = CS_HREDRAW | CS_VREDRAW;
	w.lpfnWndProc = WndProc;
	w.hInstance = hInstance;
	w.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	w.lpszClassName = L"MainWndClass";
	w.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	w.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);

	RegisterClass(&w);



	hwnd = CreateWindow(L"MainWndClass", L"WinSiteServer", WS_OVERLAPPEDWINDOW, 0, 0, 640, 480, NULL, NULL, hInstance, NULL);*/

	

	//Server->SetMainWindowHwnd(hwnd);

	/*CreateWindowEx(WS_EX_CLIENTEDGE, L"edit", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_TABSTOP | ES_READONLY, 
	5, 5, 625, 465, hwnd, (HMENU)10001, hInstance, NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);*/

	

	DialogBoxW(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);


	/*while(GetMessage(&msg, NULL, 0, 0)){
	TranslateMessage(&msg);
	DispatchMessage(&msg);
	}*/

	

	return FALSE;//msg.wParam;
}

INT_PTR CALLBACK /*LONG WINAPI*/ DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
		HANDLE_MSG(hwnd, WM_INITDIALOG, DlgOnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, DlgOnCommand);
	/*case WM_DESTROY:
		Server->Stop();
		delete Server;
		break;*/
	}

	return 0;

}

BOOL DlgOnInitDialog(HWND hwnd, HWND, LPARAM)
{
	//SetWindowTextA(GetDlgItem(hwnd, ID_EDIT1), "Инициализация диалога");
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);

	if(err!=0){
		MessageBox(NULL, L"Error: WSAS", L"Error", NULL);
		return NULL;
	}

	Server = new TCPServer;
	Server->SetMainWindowHwnd(hwnd);
	
	//SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
	return TRUE;
}

void DlgOnCommand(HWND hwnd, int id, HWND, UINT)
{
	if(id==40000){
		if(Server->Start(1111)){
			//MessageBox(hwnd, L"Started...", L"WinSiteServer", NULL);
		}
		else{
			//MessageBox(hwnd, L"Failed to start...", L"WinSiteServer", NULL);
		}
	}
	if(id==40001){
		if(Server->Stop()){
			//MessageBox(hwnd, L"Stoped...", L"WinSiteServer", NULL);
		}
		else{
			//MessageBox(hwnd, L"Failed to stop...", L"WinSiteServer", NULL);
		}
	}
	if(id==40002){
		Server->Stop();
		delete Server;
		//WSACancelBlockingCall();
		WSACleanup();
		//SendMessage(hwnd, WM_DESTROY, NULL, NULL);
		EndDialog(hwnd,0);
	}

}