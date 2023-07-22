//  Author: Dmitry Tretyakov (aka. Tinelix). 2023
//
//  This program is free software: you can redistribute it and/or modify it under the terms of
//  the GNU Lesser General Public License as published by the Free Software Foundation, either
//  version 2.1 of the License, or (at your option) any later version.
//  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License along with this
//  program. If not, see https://www.gnu.org/licenses/.
//
//  Source code: https://github.com/tinelix/WSAWrapper

#include "stdafx.h"
#include "WSAWrapper.h"
#include <winsock.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "wsock32.lib");

/////////////////////////////////////////////////////////////////////////////
// CWSAWrapperApp

BEGIN_MESSAGE_MAP(CWSAWrapperApp, CWinApp)
	//{{AFX_MSG_MAP(CWSAWrapperApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWSAWrapperApp construction

CWSAWrapperApp::CWSAWrapperApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWSAWrapperApp object

CWSAWrapperApp theApp;
SOCKET s;
int BUFFER_LENGTH = 4096;
char* recv_buff;
struct hostent *hostent;
struct sockaddr_in addr;

/////////////////////////////////////////////////////////////////////////////
// CWSAWrapperApp initialization

BOOL CWSAWrapperApp::InitInstance()
{
	WSADATA wsadata;
	if(FAILED(WSAStartup(MAKEWORD(1,1), &wsadata))) {
		return FALSE;
	}

	return TRUE;
}

BOOL CWSAWrapperApp::EnableAsyncMessages(HWND hWnd) {
	int WSAAsync = WSAAsyncSelect(s, hWnd, 0xAFFF, FD_READ | FD_CLOSE);
	if(WSAAsync > 0) {
		return FALSE;
	}
	return TRUE;
}

int CWSAWrapperApp::GetWSAError() {
	return WSAGetLastError();
}

SOCKET CWSAWrapperApp::Connect(LPSTR address, int port) {
	if(INVALID_SOCKET == (s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
		return NULL;
	}
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(address);
	addr.sin_port = htons(port);
	if(SOCKET_ERROR == (connect(s, (sockaddr*) &addr, sizeof(addr)))) {
		return NULL;
	}
	return s;
}

BOOL CWSAWrapperApp::SendData(char* buff) {
	if(SOCKET_ERROR == (send(s, buff, 512, 0))) {
		return FALSE;
	}
	return TRUE;
}

void CWSAWrapperApp::CloseSocket() {
	closesocket(s);
}

char* CWSAWrapperApp::GetInputBuffer() {
	int length = 0;
	if(SOCKET_ERROR == (length = recv(s, (char*)recv_buff, BUFFER_LENGTH, 0))) {
		strcpy(recv_buff, "[Connection closed]");	
	}
	return recv_buff;
}


