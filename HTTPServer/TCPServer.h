#include <windows.h>
#include <sys/types.h>
#include <winsock.h>
#include <vector>
#include "HTTPParser.h"

using namespace std;

#ifndef MAX_BYTES_RECIEVED
#define MAX_BYTES_RECIEVED 1024*50
#endif

typedef struct __declspec(dllexport) ClientData {
    SOCKET ClientSock;
    HANDLE ClientThread;
	char ClientAddr[16];
	DWORD threadID;
	bool toClose;
} CLDATA, *PCLDATA;

typedef struct __declspec(dllexport) ClientFunctionData {
	void * psrv;
	SOCKET ClientSock;
	DWORD threadID;
	char ClientAddr[16];
} CLFDATA, *PCLFDATA;

#pragma once
class __declspec(dllexport) TCPServer
{
	SOCKET m_Listener;
	vector<CLDATA> m_ClientDatas;
	struct sockaddr_in m_Addr;
	bool m_isOn;
	HANDLE m_ListenThread;
	HWND m_hWndMain;
	string m_logStr;

	HANDLE m_ghMutex;


	friend DWORD WINAPI ListenThreadFunction(LPVOID lpParam);
	friend DWORD WINAPI ClientThreadFunction(LPVOID lpParam);

	void HandleRequest(char buf[MAX_BYTES_RECIEVED], int bytesRead, PRESPDATA pRespData, PHNDLDATA pHandleData);
	void AcceptClient(SOCKET clientSock, char ClientAddr[15]);

public:

	void SetMainWindowHwnd(HWND hwnd);

	bool Start(int port);
	bool Stop();

	TCPServer(void);
	~TCPServer(void);
};