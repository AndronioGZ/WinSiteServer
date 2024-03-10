#include <windows.h>
#include <vector>
#include <string>
#include <fstream>

#include "../HTTPRequestHandler/HTTPHandler.h"

using namespace std;

#pragma once
class __declspec(dllexport) HTTPParser
{
	char* m_Buf;
	int m_Size;

	vector<HDRDATA> m_Headers;
	vector<URIDATA> m_URIvariables;
	vector<POSTDATA> m_POSTVariables;
	string m_Method;
	string m_URI;
	string m_Version;
		
	bool m_haveData;
	bool m_bigFile;

	DWORD m_ThreadID;

	bool m_Unsupported;
public:
	int FindStrIn(char* buf, string str, int pos1, int pos2);
	void ParseReqst(char* reqstBuf, int reqstSize, bool &bigFile, string &fileName, string &tempFileName, string &Boundary, bool &isUpload);
	void NoParsePushBuf(char reqstBuf, int reqstSize);
	char* GetDataBuf();
	int GetDataSize();
	int FindHeader(string Header);

	vector<URIDATA> GetURIData();
	vector<HDRDATA> GetHeaders();
	vector<POSTDATA> GetPOSTData();
	string GetMethod();
	string GetURI();
	string GetVersion();

	bool HaveURIData();
	bool HavePOSTData();
	bool HaveHeaders();

	void SetThreadID(DWORD ThreadID);

	HTTPParser(void);
	~HTTPParser(void);
};

