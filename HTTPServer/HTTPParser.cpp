#include "stdafx.h"
#include "HTTPParser.h"


void HTTPParser::ParseReqst(char* reqstBuf, int reqstSize, bool &bigFile, string &fileName, string &tempFileName, string &Boundary, bool &isUpload){

	if(memcmp(reqstBuf, "POST", 4) != 0 && memcmp(reqstBuf, "GET", 3) != 0 && !isUpload){
		m_Unsupported = true;
	}

	//////////////////////////////////////////////GET/////////////////////////////////////////////////


	if(memcmp(reqstBuf, "GET", 3) == 0){
		m_Method = "GET";

		m_haveData = false;

		string reqstStr;
		int strPos1, strPos2;

		reqstStr.append(reqstBuf, reqstSize);

		m_Method = "GET";

		strPos1 = reqstStr.find(" ", 5);
		m_URI = reqstStr.substr(4, strPos1 - 4);

		strPos2 = reqstStr.find("\r\n");
		m_Version = reqstStr.substr(strPos1 + 1, strPos2 - strPos1 - 1);

		strPos2+=2;

		HDRDATA hdrData;

		while(strPos2<reqstSize - 4){
			strPos1 = reqstStr.find(": ", strPos2);

			hdrData.Header = reqstStr.substr(strPos2, strPos1 - strPos2);

			strPos1+=2;
			strPos2 = reqstStr.find("\r\n", strPos1);

			hdrData.Value = reqstStr.substr(strPos1, strPos2 - strPos1);

			m_Headers.push_back(hdrData);
			strPos2+=2;
		}

		strPos1 = m_URI.find("?");
		
		if(strPos1>-1){
			strPos1++;
			URIDATA uriData;

			while(strPos1>-1){
				strPos2 = m_URI.find("=", strPos1);
				uriData.varName = m_URI.substr(strPos1, strPos2 - strPos1);

				strPos2++;
				strPos1 = m_URI.find("&", strPos2);

				uriData.varValue = m_URI.substr(strPos2, strPos1 - strPos2);

				m_URIvariables.push_back(uriData);
			}

			strPos1 = m_URI.find("?");
			m_URI = m_URI.substr(0, strPos1);
		}

	}


	/////////////////////////////////////////////////////POST////////////////////////////////////////////////////////

	if(memcmp(reqstBuf, "POST", 4) == 0){
		m_Method = "POST";
		m_haveData = true;

		string reqstStr;
		int strPos1, strPos2, headersSize;

		strPos1 = FindStrIn(reqstBuf, "\r\n\r\n", 0, (reqstSize > 2048 ? 2048 : reqstSize - 1));
		headersSize = strPos1;

		reqstStr.append(reqstBuf, strPos1);

		strPos1 = reqstStr.find(" ", 6);
		m_URI = reqstStr.substr(5, strPos1 - 5);

		strPos2 = reqstStr.find("\r\n");
		m_Version = reqstStr.substr(strPos1 + 1, strPos2 - strPos1 - 1);
		strPos2+=2;

		HDRDATA hdrData;

		while(strPos2 > 0){
			strPos1 = reqstStr.find(": ", strPos2);

			hdrData.Header = reqstStr.substr(strPos2, strPos1 - strPos2);

			strPos1+=2;
			strPos2 = reqstStr.find("\r\n", strPos1);

			hdrData.Value = reqstStr.substr(strPos1, strPos2 - strPos1);

			m_Headers.push_back(hdrData);

			if(strPos2 > 0) strPos2+=2;
		}

		int headerPos = FindHeader("Content-Type");

		if(headerPos>=0){
			if(m_Headers[headerPos].Value.substr(0, 19) == "multipart/form-data"){
				strPos1 = reqstStr.find("boundary=") + 9;
				strPos2 = reqstStr.find("\r\n", strPos1);
				Boundary = reqstStr.substr(strPos1, strPos2 - strPos1);

				strPos1 = FindStrIn(reqstBuf, "filename=", 0, reqstSize - 1) + 9; //reqstStr.find("filename=") + 9;
				strPos2 = FindStrIn(reqstBuf, "\r\n", strPos1, reqstSize - 1); //reqstStr.find("\r\n", strPos1);
				char fileNameCh[32];
				memcpy(fileNameCh, reqstBuf + strPos1, strPos2 - strPos1);
				if(strPos1 - 9 > 0) fileName.append(fileNameCh, strPos2 - strPos1); //reqstStr.substr(strPos1, strPos2 - strPos1);

				isUpload = true;

				Boundary = "--" + Boundary;

				if(FindStrIn(reqstBuf, Boundary, strPos1, reqstSize - 1)>0){ //reqstStr.find(Boundary, strPos1)>0){

					string tempBoundary = Boundary;

					strPos1 = FindStrIn(reqstBuf, tempBoundary, 0, reqstSize - 1);
					tempBoundary += "--";
					strPos2 = FindStrIn(reqstBuf, tempBoundary, 0, reqstSize - 1);

					if(strPos2>=0){
						m_Size = strPos2 - strPos1 - Boundary.size() -2;
						m_Buf = new char[m_Size];

						memcpy(m_Buf, reqstBuf + strPos1 + Boundary.size() + 1, m_Size);

						char tempch[16];
						sprintf_s(tempch, "%d", m_ThreadID);
						
						tempFileName = "Data\\Temp\\tempfile";
						tempFileName.append(tempch);
						tempFileName+=".temp";

						ofstream ofTempFile(tempFileName, ofstream::app | ofstream::binary);

						ofTempFile.write(m_Buf, m_Size);

						ofTempFile.close();
					}
					else{
						strPos2 = reqstSize - 1;

						m_bigFile = true;

						m_Size = strPos2 - strPos1 - Boundary.size() -2;
						m_Buf = new char[m_Size];

						memcpy(m_Buf, reqstBuf + strPos1 + Boundary.size() + 1, m_Size);

						char tempch[16];
						sprintf_s(tempch, "%d", m_ThreadID);

						string tempFileName = "tempfile";
						tempFileName.append(tempch);
						tempFileName+=".temp";

						ofstream ofTempFile(tempFileName, ofstream::app | ofstream::binary);

						ofTempFile.write(m_Buf, m_Size);

						ofTempFile.close();
					}
				}
				else{
					m_bigFile = true;
					bigFile = true;
				}
			}
			else{
				if(m_Headers[headerPos].Value.substr(0, 19) == "application/x-www-form-urlencoded"){
					reqstStr.clear();
					reqstStr.append(reqstBuf, reqstSize);

					strPos1 = reqstStr.find("\r\n\r\n");
					strPos2 = reqstStr.find("\r\n", strPos1);

					POSTDATA POSTdata;

					bool fillName = true;
					
					for(int i=strPos1;i<strPos2;i++){
						if(reqstStr.at(i)=='='){
							fillName = false;
						}
						else{
							if(reqstStr.at(i)=='&'){
								fillName = true;
								m_POSTVariables.push_back(POSTdata);
								POSTdata.varName.clear();
								POSTdata.varValue.clear();
							}
							else{
								if(fillName){
									(POSTdata.varName).append(reqstStr.at(i), 1);
								}
								else{
									(POSTdata.varValue).append(reqstStr.at(i), 1);
								}						
							}
						}
					}


				}


			}
		}
	}

	////////////////////////////////////////////////////////// file Uploading ///////////////////////////////////////////////////

	if(memcmp(reqstBuf, "POST", 4) != 0 && memcmp(reqstBuf, "GET", 3) != 0 && isUpload && m_bigFile){
		string tempBoundaryEnd = Boundary + "--";

		//Middle of data
		if(FindStrIn(reqstBuf, Boundary, 0, Boundary.size()) < 0 && FindStrIn(reqstBuf, tempBoundaryEnd, reqstSize - tempBoundaryEnd.size() - 3, reqstSize - tempBoundaryEnd.size()) < 0){
			char tempch[16];
			sprintf_s(tempch, "%d", m_ThreadID);

			string tempFileName = "Data\\Temp\\tempfile";
			tempFileName.append(tempch);
			tempFileName+=".temp";
			
			ofstream ofTempFile(tempFileName, ofstream::app | ofstream::binary);

			ofTempFile.write(reqstBuf, reqstSize);

			ofTempFile.close();
		}

		//Beginning of data
		if(FindStrIn(reqstBuf, Boundary, 0, Boundary.size()) == 0 && FindStrIn(reqstBuf, tempBoundaryEnd, reqstSize - tempBoundaryEnd.size() - 3, reqstSize  - tempBoundaryEnd.size()) < 0){
			char tempch[16];
			sprintf_s(tempch, "%d", m_ThreadID);

			int strPos1 = FindStrIn(reqstBuf, "filename=", 0, (reqstSize > 512 ? 512 : reqstSize) - 1) + 9;
			int strPos2 = FindStrIn(reqstBuf, "\r\n", strPos1, (reqstSize > 512 ? 512 : reqstSize) - 1);
			char fileNameCh[32];
			memcpy(fileNameCh, reqstBuf + strPos1, strPos2 - strPos1);
			if(strPos1 - 9 > 0) fileName.append(fileNameCh, strPos2 - strPos1); 
			
			tempFileName = "Data\\Temp\\tempfile";
			tempFileName.append(tempch);
			tempFileName+=".temp";

			int strPos = FindStrIn(reqstBuf, "\r\n\r\n", 0, reqstSize - 4) + 4;
			
			ofstream ofTempFile(tempFileName, ofstream::app | ofstream::binary);

			ofTempFile.write(reqstBuf + strPos, reqstSize - strPos);

			ofTempFile.close();
		}

		//End of data
		if(FindStrIn(reqstBuf, Boundary, 0, Boundary.size()) < 0 && FindStrIn(reqstBuf, tempBoundaryEnd, reqstSize - tempBoundaryEnd.size() - 3, reqstSize - tempBoundaryEnd.size()) >= 0){
			char tempch[16];
			sprintf_s(tempch, "%d", m_ThreadID);

			tempFileName = "Data\\Temp\\tempfile";
			tempFileName.append(tempch);
			tempFileName+=".temp";

			int strPos;

			strPos = reqstSize - FindStrIn(reqstBuf, Boundary, (reqstSize <= 512 ? 0 : reqstSize - 512), reqstSize - tempBoundaryEnd.size() - 3);

			ofstream ofTempFile(tempFileName, ofstream::app | ofstream::binary);

			ofTempFile.write(reqstBuf, reqstSize - strPos - 2);

			ofTempFile.close();

			//tempFileName = "";
			//fileName = "";
			m_bigFile = false;
			bigFile = false;
			isUpload = false;
		}

		//Short file
		if(FindStrIn(reqstBuf, Boundary, 0, Boundary.size()) == 0 && FindStrIn(reqstBuf, tempBoundaryEnd, reqstSize - tempBoundaryEnd.size() - 3, reqstSize - tempBoundaryEnd.size()) >= 0){
			char tempch[16];
			sprintf_s(tempch, "%d", m_ThreadID);

			int strPos1 = FindStrIn(reqstBuf, "filename=", 0, (reqstSize > 512 ? 512 : reqstSize) - 1) + 9;
			int strPos2 = FindStrIn(reqstBuf, "\r\n", strPos1, (reqstSize > 512 ? 512 : reqstSize) - 1);
			char fileNameCh[32];
			memcpy(fileNameCh, reqstBuf + strPos1, strPos2 - strPos1);
			if(strPos1 - 9 > 0) fileName.append(fileNameCh, strPos2 - strPos1); 

			tempFileName = "Data\\Temp\\tempfile";
			tempFileName.append(tempch);
			tempFileName+=".temp";

			strPos1 = FindStrIn(reqstBuf, "\r\n\r\n", 0, reqstSize - 4) + 4;

			int strPos;

			strPos = reqstSize - FindStrIn(reqstBuf, Boundary, (reqstSize <= 512 ? 0 : reqstSize - 512), reqstSize - tempBoundaryEnd.size() - 3);

			ofstream ofTempFile(tempFileName, ofstream::binary);

			ofTempFile.write(reqstBuf + strPos1, reqstSize - strPos - 2 - strPos1);

			ofTempFile.close();

			//tempFileName = "";
			//fileName = "";
			m_bigFile = false;
			bigFile = false;
			//isUpload = false;
		}
	}

	if(isUpload){
		delete m_Buf;
	}
}

int HTTPParser::FindHeader(string Header){
	for(int i=0;i<m_Headers.size();i++){
		if(m_Headers[i].Header == Header){
			return i;
		}
	}
	return -1;
}

void HTTPParser::NoParsePushBuf(char reqstBuf, int reqstSize){

}

char* HTTPParser::GetDataBuf(){
	if(!m_haveData){
		return NULL;
	}
	return m_Buf;
}

int HTTPParser::GetDataSize(){
	if(!m_haveData){
		return -1;
	}
	return m_Size;
}

vector<URIDATA> HTTPParser::GetURIData(){
	return m_URIvariables;
}

vector<POSTDATA> HTTPParser::GetPOSTData(){
	return m_POSTVariables;
}

vector<HDRDATA> HTTPParser::GetHeaders(){
	return m_Headers;
}

int HTTPParser::FindStrIn(char* buf, string str, int pos1, int pos2){
	char* tempbuf;
	tempbuf = new char[str.size()];

	for(int i=pos1;i<pos2;i++){
		memcpy((char*)tempbuf, (char*)(buf+i), str.size());
		if(memcmp((char*)tempbuf, (char*)str.c_str(), str.size())==0){
			delete[] tempbuf;
			return i;
		}
	}
	delete[] tempbuf;
	return -1;
}

void HTTPParser::SetThreadID(DWORD ThreadID){
	m_ThreadID = ThreadID;
}

string HTTPParser::GetMethod(){
	return m_Method;
}

string HTTPParser::GetURI(){
	return m_URI;
}

string HTTPParser::GetVersion(){
	return m_Version;
}

bool HTTPParser::HaveURIData(){
	return m_URIvariables.size()>0;
}

bool HTTPParser::HavePOSTData(){
	return m_POSTVariables.size()>0;
}

bool HTTPParser::HaveHeaders(){
	return m_Headers.size()>0;
}

HTTPParser::HTTPParser(void)
{
	m_Unsupported = false;
	m_bigFile = false;
}


HTTPParser::~HTTPParser(void)
{
	
}
