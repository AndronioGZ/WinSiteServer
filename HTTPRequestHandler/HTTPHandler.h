#include <windows.h>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

typedef struct __declspec(dllexport) HeaderData {
    string Header;
	string Value;
} HDRDATA, *PHDRDATA;
// Contains data of one HTTP header

typedef struct __declspec(dllexport) URIData {
    string varName;
	string varValue;
} URIDATA, *PURIDATA;
// Contains data of one variable in URI

typedef struct __declspec(dllexport) POSTData {
    string varName;
	string varValue;
} POSTDATA, *PPOSTDATA;
// Contains data of one variable of POST request

typedef struct __declspec(dllexport) HandleData {
	bool recvdFile;
	// true if we have recieved file by POST request

	string fileName;
	//Original file name
	string tempFileName;
	// Temprorary name of recieved file

	vector<URIDATA> URIData;
	vector<POSTDATA> POSTData;
	vector<HDRDATA> Headers;
	// Containers with headers and possible variables (see above)
	
	string Method;
	string URI;
	string Version;
	// Parts of Request-Line of HTTP request

	char ClientAddr[16];
	// Client's IP

} HNDLDATA, *PHNDLDATA;
// Contains data of current HTTP request

typedef struct __declspec(dllexport) ResponceData {
	char* RespBuf;
	int RespSize;
	bool Connected;
	bool NoMessageBody;
} RESPDATA, *PRESPDATA;
// Contains data, that contains HTTP response and some variables determining the status of the connection

#pragma once
class __declspec(dllexport) HTTPHandler
{
	PRESPDATA m_pRespData;
	// Pointer to data, that contains HTTP response and some variables determining the status of the connection

	string m_Version;
	string m_StatusCode;
	string m_ReasonPhrase;
	// Variables, components of Status-Line

	string m_sBuf;
	// String, containing text(non-binary) data of HTTP responce

public:
	void SetStatusLine(string Version, string StatusCode, string ReasonPhrase);
	void SetStatusLine(string StatusCode, string ReasonPhrase);
	void SetStatusLine();
	// Functions setting the Status-Line of HTTP responce

	void AddHeader(string Header, string Value);
	void AddHeader(string Header, int Value);
	// Functions to add Header to HTTP responce

	void AddMessageBodyStr(string sBuf);
	void AddMessageBodyBinary(char* Buf, int Size);
	// Functions to add Message-Body to HTTP responce

	int LoadDataFromFile(char *&destBuf, char* fileName);
	string LoadStringFromFile(char* fileName);
	// Functions to get data from file

	void ReplaceStringInPattern(string &Pattern, string Keyword, string Replacement);
	// This function replaces string Keyword with other string Replacement in Patern string
	// Use this one to make content from pattern, as usual loaded from HTML file

	void AcceptRequest(PHNDLDATA pHandleData);
	// Here you can add code to change the way that server handles HTTP request

	PRESPDATA GetRespData();
	// Returns pointer m_pRespData (see above)

	HTTPHandler(void);
	~HTTPHandler(void);
	// Constructor and destructor
};

