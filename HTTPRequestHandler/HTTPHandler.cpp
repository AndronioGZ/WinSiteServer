#include "stdafx.h"
#include "HTTPHandler.h"

void HTTPHandler::AcceptRequest(PHNDLDATA pHandleData){
	string str, str2;

	if(pHandleData->URI == "/"){
		/*str2 = "<html><h3><br>";

		str2.append(pHandleData->ClientAddr);
		str2+="<br>";

		for(int i=0;i<pHandleData->Headers.size();i++){
		str2+=pHandleData->Headers[i].Header;
		str2+="&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
		str2+=pHandleData->Headers[i].Value;
		str2+="<br>";
		}
		str2+="<img src=\"Chrysanthemum.jpg\">";
		str2+="</h3></html>";

		SetStatusLine();
		AddHeader("Content-Type", "text/html; charset=UTF-8");
		AddHeader("Connection", "Close");
		AddMessageBodyStr(str2);*/
		if(GetFileAttributesA("Data\\index.html") != DWORD(-1)){
			char* fBuf = NULL;
			string fStr;
			int fLen;
						
			fLen = LoadDataFromFile(fBuf, "Data\\index.html");
			fStr.append(fBuf, fLen);

			SetStatusLine();
			AddHeader("Content-Type", "text/html; charset=UTF-8");
			AddMessageBodyStr(fStr);
		}
		else{
			SetStatusLine("404", "Not Found");
			AddMessageBodyStr("<html><h1>ERROR 404 NOT FOUND</h1><html>");
		}

		return;
	}

	if(pHandleData->Method == "POST" && pHandleData->URI == "/upload" && pHandleData->recvdFile){
		string newFileName = pHandleData->fileName;
		newFileName.erase(0, 1);
		newFileName.erase(newFileName.size() - 1, 1);
		newFileName = "Data\\img\\orig\\" + newFileName;
		MoveFileA(pHandleData->tempFileName.c_str(), newFileName.c_str()); 

		string outStr = pHandleData->tempFileName + " " + newFileName;
		
		SetStatusLine();
		AddHeader("Content-Type", "text/html; charset=UTF-8");
		AddMessageBodyStr(outStr);
		return;
	}

	if(pHandleData->URI.substr(0, 8) == "/preview"){
		if(GetFileAttributesA("Data\\preview.txt") != DWORD(-1)){
			char* fBuf = NULL;
			string fStr;
			int fLen;
						
			fLen = LoadDataFromFile(fBuf, "Data\\preview.txt");
			fStr.append(fBuf, fLen);

			SetStatusLine();
			AddHeader("Content-Type", "text/html; charset=UTF-8");
			AddMessageBodyStr(fStr);
		}
		else{
			SetStatusLine("404", "Not Found");
			AddMessageBodyStr("<html><h1>ERROR 404 NOT FOUND</h1><html>");
		}
		
		return;
	}

	if(pHandleData->URI == "/getmaxpages"){
		SetStatusLine();
		AddHeader("Content-Type", "text/html; charset=UTF-8");
		AddMessageBodyStr("5");
		return;
	}

	pHandleData->URI = pHandleData->URI.substr(1, pHandleData->URI.size() - 1);

	for(int i=0;i<pHandleData->URI.size();i++){
		if(pHandleData->URI.at(i)=='/'){
			pHandleData->URI.erase(i, 1);
			pHandleData->URI.insert(i, "\\");
		}
	}

	pHandleData->URI = "Data\\" + pHandleData->URI;

	if(GetFileAttributesA(pHandleData->URI.c_str()) != DWORD(-1)){
		ifstream ifs;
		char* fBuf;
		int fLen;
		LPWSTR mimeBuf = 0;
		char* mimeBufch = new char[256];
		memset(mimeBufch, 0, 256);

		ifs.open(pHandleData->URI.c_str(), ios::binary);
		ifs.seekg(0, ios::end);
		fLen = ifs.tellg();
		ifs.seekg(0, ios::beg);

		fBuf = new char[fLen];
		ifs.read(fBuf, fLen);
		ifs.close();

		if(FindMimeFromData(NULL, NULL, fBuf, fLen, NULL, FMFD_ENABLEMIMESNIFFING, &mimeBuf, 0)!=S_OK){
			SetStatusLine("404", "Not Found");
			AddMessageBodyStr("<html><h1>ERROR 404 NOT FOUND</h1><html>");
		}
		else{
			WideCharToMultiByte(CP_ACP , 0, mimeBuf, /*SysStringLen(mimeBuf)*/wcslen(mimeBuf), mimeBufch, 255, NULL, NULL);//!=0){
			SetStatusLine();
			AddHeader("Content-Type", (string)mimeBufch);
			AddMessageBodyBinary(fBuf, fLen);
		}

		delete[] mimeBufch;
	}
	else{
		SetStatusLine("404", "Not Found");
		AddMessageBodyStr("<html><h1>ERROR 404 NOT FOUND</h1><html>");
	}

}

void HTTPHandler::SetStatusLine(string Version, string StatusCode, string ReasonPhrase){
	m_sBuf = Version + " " + StatusCode + " " + ReasonPhrase + "\r\n";
}

void HTTPHandler::SetStatusLine(string StatusCode, string ReasonPhrase){
	m_sBuf = m_Version + " " + StatusCode + " " + ReasonPhrase + "\r\n";
}

void HTTPHandler::SetStatusLine(){
	m_sBuf = m_Version + " " + m_StatusCode + " " + m_ReasonPhrase + "\r\n";
}

void HTTPHandler::AddHeader(string Header, string Value){
	m_sBuf += Header;
	m_sBuf += ": ";
	m_sBuf += Value;
	m_sBuf += "\r\n";
}

void HTTPHandler::AddHeader(string Header, int Value){
	m_sBuf += Header;
	m_sBuf += ": ";

	char nch[16];
	sprintf_s(nch, "%d", Value);

	m_sBuf.append(nch);
	m_sBuf += "\r\n";	
}

void HTTPHandler::AddMessageBodyStr(string sBuf){
	AddHeader("Content-Length", sBuf.size());
	m_sBuf += "\r\n";
	m_sBuf += sBuf;
	m_sBuf += "\r\n\r\n";

	m_pRespData->RespBuf = new char[m_sBuf.size()];

	memcpy(m_pRespData->RespBuf, m_sBuf.c_str(), m_sBuf.size());

	m_pRespData->RespSize = m_sBuf.size();

	m_pRespData->NoMessageBody = false;
}

void HTTPHandler::AddMessageBodyBinary(char* Buf, int Size){
	AddHeader("Content-Length", Size);
	m_sBuf += "\r\n";

	m_pRespData->RespBuf = new char[m_sBuf.size() + Size];

	memcpy(m_pRespData->RespBuf, m_sBuf.c_str(), m_sBuf.size());
	memcpy(m_pRespData->RespBuf + m_sBuf.size(), Buf, Size);

	m_pRespData->RespSize = m_sBuf.size() + Size;

	m_pRespData->NoMessageBody = false;
}

PRESPDATA HTTPHandler::GetRespData(){
	if(m_pRespData->NoMessageBody){
		m_sBuf += "\r\n";
		m_pRespData->RespBuf = new char[m_sBuf.size()];
		memcpy(m_pRespData->RespBuf, m_sBuf.c_str(), m_sBuf.size());
		m_pRespData->RespSize = m_sBuf.size();
	}
	return m_pRespData;
}


int HTTPHandler::LoadDataFromFile(char *&destBuf, char* fileName){
	if(GetFileAttributesA(fileName) == DWORD(-1)){
		return -1;
	}
	else{
		ifstream ifs;
		int fLen;

		ifs.open(fileName, ios::binary);
		ifs.seekg(0, ios::end);
		fLen = ifs.tellg();
		ifs.seekg(0, ios::beg);

		destBuf = new char[fLen];
		ifs.read(destBuf, fLen);
		ifs.close();

		return fLen;
	}
}

string HTTPHandler::LoadStringFromFile(char* fileName){
	if(GetFileAttributesA(fileName) != DWORD(-1)){
		return "";
	}
	else{
		ifstream ifs;
		char* fBuf;
		int fLen;
		string retStr;

		ifs.open(fileName, ios::binary);
		ifs.seekg(0, ios::end);
		fLen = ifs.tellg();
		ifs.seekg(0, ios::beg);

		fBuf = new char[fLen];
		ifs.read(fBuf, fLen);
		ifs.close();

		retStr.append(fBuf, fLen);

		return retStr;
	}
}

void HTTPHandler::ReplaceStringInPattern(string &Pattern, string Keyword, string Replacement){
	int strPos;

	strPos = Pattern.find(Keyword);
	Pattern.erase(strPos, Keyword.size());
	Pattern.insert(strPos, Replacement);
}


HTTPHandler::HTTPHandler(void)
{
	m_pRespData = new RESPDATA;
	m_Version = "HTTP/1.1";
	m_StatusCode = "200";
	m_ReasonPhrase = "OK";
	m_sBuf = "";
	m_pRespData->NoMessageBody = true;
}


HTTPHandler::~HTTPHandler(void)
{
	delete m_pRespData;
}
