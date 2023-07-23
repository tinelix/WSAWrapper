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


#include <windows.h>
#include <winsock.h>
#include "WSAWrapper.h"

#pragma comment(lib, "wsock32.lib");

SOCKET s;
int BUFFER_LENGTH = 4096;
char* recv_buff;
struct hostent *hostent;
struct sockaddr_in addr;

int WINAPI DllMain(HINSTANCE hInst, DWORD fdReas, PVOID pvRes) {
	switch(fdReas) {
		case DLL_PROCESS_ATTACH:
			OutputDebugString("\r\nWinsock Wrapper"
			"\r\nCopyright © 2023 Dmitry Tretyakov (aka. Tinelix). Licensed under LGPLv2.1"
			"\r\n");
			break;
		case DLL_PROCESS_DETACH:
			OutputDebugString("\r\nWinsock Wrapper is shutting down...\r\n");
			break;
	}
	
	return TRUE;
}

EXPORT BOOL CALLBACK InitializeWinSock() {
	WSADATA wsadata;
	if(FAILED(WSAStartup(MAKEWORD(1,1), &wsadata))) {
		return FALSE;
	}
	return TRUE;
}

EXPORT BOOL CALLBACK EnableAsyncMessages(HWND hWnd) {
	int WSAAsync = WSAAsyncSelect(s, hWnd, 0xAFFF, FD_READ|FD_CLOSE);
	if(WSAAsync > 0) {
		return FALSE;
	}
	return TRUE;
}

EXPORT int CALLBACK GetWSAError() {
	return WSAGetLastError();
}

EXPORT SOCKET CALLBACK CreateConnection(LPSTR address, int port) {
	if(INVALID_SOCKET == (s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
		return NULL;
	}
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(address);
	addr.sin_port = htons(port);
	if(SOCKET_ERROR == (connect(s, (sockaddr*)&addr, sizeof(addr)))) {
		return NULL;
	}
	return s;
}

EXPORT BOOL CALLBACK SendData(char* buff) {
	if(SOCKET_ERROR == (send(s, buff, 512, 0))) {
		return FALSE;
	}
	return TRUE;
}


EXPORT char* CALLBACK GetInputBuffer() {
	int length = 0;
	if(SOCKET_ERROR == (length = recv(s, (char*)recv_buff, BUFFER_LENGTH, 0))) {
		strcpy(recv_buff, "\r\n[Connection closed]");
	}
	return recv_buff;
}

EXPORT void CloseConnection() {
	closesocket(s);
}



