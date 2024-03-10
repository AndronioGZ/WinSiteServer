#include "stdafx.h"
#include "TCPServer.h"

DWORD WINAPI ListenThreadFunction(LPVOID lpParam){
	TCPServer* psrv;
	psrv = (TCPServer*)lpParam;

	if(bind(psrv->m_Listener, (struct sockaddr *)&psrv->m_Addr, sizeof(psrv->m_Addr)) < 0){
		MessageBox(NULL, L"Error: Bind", L"Error", NULL);
		return 0;
	}

	listen(psrv->m_Listener, 1);

	struct sockaddr_in addr;
	int* addr_size = new int;
	*addr_size = sizeof(addr);

	SOCKET ClientSock;
	SOCKET prevSock = 0;
	bool noEqualSockets = true;

	while(psrv->m_isOn){

		if(psrv->m_ClientDatas.size()>0){

			WaitForSingleObject(psrv->m_ghMutex, INFINITE);

			vector<CLDATA>::iterator it = psrv->m_ClientDatas.begin();
			bool delFl = false;

			while(it!=psrv->m_ClientDatas.end()){
				if(it->toClose){
					closesocket(it->ClientSock);
					WaitForSingleObject(it->ClientThread, INFINITE);
					CloseHandle(it->ClientThread);
					it = psrv->m_ClientDatas.erase(it);
					delFl = true;
				}
				if(!delFl){
					++it;
				}
				else{
					delFl = false;
				}
			}

			ReleaseMutex(psrv->m_ghMutex);

		}

		do{
			ClientSock = accept(psrv->m_Listener, (sockaddr*)&addr, addr_size);

			if(!psrv->m_isOn){
				delete addr_size;
				return 0;
			}

			if(ClientSock==prevSock){
				noEqualSockets = false;
				closesocket(ClientSock);
				ClientSock = 0;
				ZeroMemory(&addr, sizeof(addr));
			}

		}while(noEqualSockets==false && psrv->m_isOn);

		prevSock = ClientSock;


		if(ClientSock<0){
			MessageBox(NULL, L"Error: Accept", L"Error", NULL);
			closesocket(psrv-> m_Listener);
			delete addr_size;
			return 0;
		}

		char Addr[16];
		strcpy_s(Addr, 16, inet_ntoa(addr.sin_addr));

		psrv->AcceptClient(ClientSock, Addr);	

	}

	delete addr_size;

	return 0;
}

DWORD WINAPI ClientThreadFunction(LPVOID lpParam){
	PCLFDATA pCliFuncData;
	pCliFuncData = (PCLFDATA)lpParam;

	bool isConnected = true;
	int bytesRead = 0;

	SOCKET ClientSock = pCliFuncData->ClientSock;

	char* recvBuf;
	recvBuf = new char[MAX_BYTES_RECIEVED];

	bool isUpload = false;
	bool bigFile = false;
	string tempFileName = "";
	string fileName = "";
	string Boundary = "";

	while(isConnected && ((TCPServer*)pCliFuncData->psrv)->m_isOn){
		//memset(recvBuf, 0, MAX_BYTES_RECIEVED);

		HTTPParser *Parser = new HTTPParser;
		PHNDLDATA pHandleData;
		pHandleData = new HNDLDATA;

		do{
			bytesRead = recv(ClientSock, recvBuf, MAX_BYTES_RECIEVED, 0);
			if(bytesRead>0){
				Parser->SetThreadID(pCliFuncData->threadID);
				Parser->ParseReqst(recvBuf, bytesRead, bigFile, fileName, tempFileName, Boundary, isUpload);
				if(isUpload){
					pHandleData->tempFileName = tempFileName;
					pHandleData->fileName = fileName;
					pHandleData->recvdFile = true;
				}
			}
		}while(bigFile);

		if(Parser->HaveHeaders()) pHandleData->Headers = Parser->GetHeaders();
		if(Parser->HavePOSTData()) pHandleData->POSTData = Parser->GetPOSTData();
		if(Parser->HaveURIData()) pHandleData->URIData = Parser->GetURIData();

		pHandleData->Method = Parser->GetMethod();
		pHandleData->URI = Parser->GetURI();
		pHandleData->Version = Parser->GetVersion();
		strcpy_s(pHandleData->ClientAddr, 16, pCliFuncData->ClientAddr);

		if(bytesRead>0){
			PRESPDATA pRespData;

			pRespData = new RESPDATA;

			((TCPServer*)pCliFuncData->psrv)->HandleRequest(recvBuf, bytesRead, pRespData, pHandleData);

			send(ClientSock, pRespData->RespBuf, pRespData->RespSize, 0);

			isConnected = pRespData->Connected;

			if(!pRespData->NoMessageBody) delete[] pRespData->RespBuf;

			delete pRespData;
		}
		else{
			if(!((TCPServer*)pCliFuncData->psrv)->m_isOn){
				delete pCliFuncData;
				pCliFuncData = NULL;

				delete[] recvBuf;

				return 0;
			}
			else{
				isConnected = false;
			}
		}

		fileName = "";
		tempFileName = "";

		delete pHandleData;
		delete Parser;
	}

	//WaitForSingleObject(((TCPServer*)pCliFuncData->psrv)->m_ghMutex, INFINITE);

	for(vector<CLDATA>::iterator it = ((TCPServer*)pCliFuncData->psrv)->m_ClientDatas.begin(); it != ((TCPServer*)pCliFuncData->psrv)->m_ClientDatas.end(); ++it){
		if(it->threadID == pCliFuncData->threadID){
			it->toClose = true;
			break;
		}
	}

	//ReleaseMutex(((TCPServer*)pCliFuncData->psrv)->m_ghMutex);

	delete pCliFuncData;
	pCliFuncData = NULL;

	delete[] recvBuf;

	return 0;
}

void TCPServer::AcceptClient(SOCKET clientSock, char ClientAddr[15]){
	CLDATA CliData;
	CliData.ClientSock = clientSock;
	strcpy_s(CliData.ClientAddr, 16, ClientAddr);
	CliData.toClose = false;

	PCLFDATA pCliFuncData;

	pCliFuncData = new CLFDATA;

	pCliFuncData->psrv = this;
	pCliFuncData->ClientSock = clientSock;
	strcpy_s(pCliFuncData->ClientAddr, 16, ClientAddr);

	CliData.ClientThread = CreateThread(NULL, MAX_BYTES_RECIEVED + 10*1024, ClientThreadFunction, pCliFuncData, STACK_SIZE_PARAM_IS_A_RESERVATION, &(pCliFuncData->threadID));

	if(CliData.ClientThread==NULL){
		MessageBox(m_hWndMain, L"Error: Client Thread", L"Error", NULL);
		delete pCliFuncData;
		return;
	}

	CliData.threadID = pCliFuncData->threadID;

	WaitForSingleObject(m_ghMutex, INFINITE);

	m_ClientDatas.push_back(CliData);

	ReleaseMutex(m_ghMutex);
}

void TCPServer::HandleRequest(char buf[MAX_BYTES_RECIEVED], int bytesRead, PRESPDATA pRespData, PHNDLDATA pHandleData){

	HWND hwndReqst;
	/*string str1;
	str1.append(buf, bytesRead);

	str1+=pHandleData->Method;
	str1+="\r\n";
	str1+=pHandleData->URI;
	str1+="\r\n";
	str1+=pHandleData->Version;
	str1+="\r\n";

	for(int i=0;i<pHandleData->Headers.size();i++){
		str1+=pHandleData->Headers[i].Header;
		str1+=" ";
		str1+=pHandleData->Headers[i].Value;
		str1+="\r\n";
	}*/

	m_logStr.append(pHandleData->ClientAddr);
	m_logStr += " ";
	m_logStr += pHandleData->Method;
	m_logStr += " ";
	m_logStr += pHandleData->URI;
	m_logStr += "\r\n";
	

	hwndReqst = GetDlgItem(m_hWndMain, 1000);
	SetWindowTextA(hwndReqst, (LPCSTR)m_logStr.c_str());

	HTTPHandler* Handler = new HTTPHandler;
	Handler->AcceptRequest(pHandleData);
	*pRespData = *(Handler->GetRespData());

	delete Handler;
}

void TCPServer::SetMainWindowHwnd(HWND hwnd){
	m_hWndMain = hwnd;
}


bool TCPServer::Start(int port){
	if(m_isOn){
		//MessageBox(m_hWndMain, L"Already Runnin\'...", L"", NULL);
		return false;
	}

	DWORD   dwThreadId;

	memset(&m_Addr, 0, sizeof(m_Addr));

	m_Listener = socket(AF_INET, SOCK_STREAM, 0);

	if(m_Listener<0){
		MessageBox(NULL, L"Error: Socket", L"Error", NULL);
		return false;
	}

	m_Addr.sin_family = AF_INET;
	m_Addr.sin_port = htons(port);
	m_Addr.sin_addr.s_addr = INADDR_ANY;

	m_ListenThread = CreateThread(NULL, 1024*1024, ListenThreadFunction, this, STACK_SIZE_PARAM_IS_A_RESERVATION, &dwThreadId);

	if(m_ListenThread==NULL){
		return false;
	}
	else{
		m_isOn = true;
		return true;
	}
}

bool TCPServer::Stop(){
	if(!m_isOn){
		//MessageBox(m_hWndMain, L"Not started yet!..", L"", NULL);
		return false;
	}

	m_isOn = false;

	shutdown(m_Listener, 2);
	closesocket(m_Listener);

	WaitForSingleObject(m_ListenThread, INFINITE);

	CloseHandle(m_ListenThread);

	if(m_ClientDatas.size()>0){

		for(vector<CLDATA>::iterator it = m_ClientDatas.begin();it != m_ClientDatas.end(); ++it){
			shutdown(it->ClientSock, 2);
			closesocket(it->ClientSock);
		}

		HANDLE* hThreadArray;
		hThreadArray = new HANDLE[m_ClientDatas.size()];

		for(int i=0;i<m_ClientDatas.size();i++){
			hThreadArray[i] = m_ClientDatas[i].ClientThread;
		}

		WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS, hThreadArray, TRUE, INFINITE);

		for(int i=0; i < (int)m_ClientDatas.size(); i++){
			CloseHandle(hThreadArray[i]);
			//delete (PCLFDATA) (m_ClientDatas[i].pClientFuncData);
			//m_ClientDatas[i].pClientFuncData = NULL;
		}

		m_ClientDatas.clear();

		delete[] hThreadArray;

	}

	return true;
}


TCPServer::TCPServer(void)
{
	m_isOn = false;
	m_ClientDatas.clear();
	m_ghMutex = CreateMutex(NULL, FALSE, NULL);
	m_logStr = "";
}


TCPServer::~TCPServer(void)
{
	if(m_isOn) Stop();	
}
