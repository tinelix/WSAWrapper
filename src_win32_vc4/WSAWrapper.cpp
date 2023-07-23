//  Copyright © 2023 Dmitry Tretyakov (aka. Tinelix)
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
#include <stdio.h>
#include "WSAWrapper.h"

#pragma comment(lib, "wsock32.lib");

SOCKET s;
int BUFFER_LENGTH = 4096;
char* recv_buff;
struct hostent *hostent;
struct sockaddr_in addr;
char* g_address;
char debug_str[400];
BOOL is_win32s;

int WINAPI DllMain(HINSTANCE hInst, DWORD fdReas, PVOID pvRes) {
	switch(fdReas) {
		case DLL_PROCESS_ATTACH:
			// Checking if it's Win32s
			if(GetVersion() & 0x80000000 && (GetVersion() & 0xFF) == 3) {
				is_win32s = TRUE;
			} else {
				is_win32s = FALSE;
			}
			if(!is_win32s) {
				OutputDebugString("\r\nWinsock Wrapper - Win32 DLL"
				"\r\nCopyright © 2023 Dmitry Tretyakov (aka. Tinelix). Licensed under LGPLv2.1."
				"\r\nSource code: https://github.com/tinelix/WSAWrapper\r\n");
			}
			InitializeWinSock();
			break;
		case DLL_PROCESS_DETACH:
			CloseConnection();
			WSACleanup();
			if(!is_win32s) {
				OutputDebugString("\r\nWinsock Wrapper is shutting down...\r\n");
			}
			break;
	}
	
	return TRUE;
}

EXPORT BOOL CALLBACK InitializeWinSock() {
	WSADATA wsadata;
	if(FAILED(WSAStartup(MAKEWORD(1,1), &wsadata))) {
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Winsock initialization "
				"failed / Error code: %d", WSAGetLastError());
			OutputDebugString(debug_str);
		}
		return FALSE;
	}
	if(!is_win32s) {
		sprintf(debug_str, "\r\n[WSAWrapper] Winsock 1.1+ initialized.");
		OutputDebugString(debug_str);
	}
	return TRUE;
}

EXPORT BOOL CALLBACK EnableAsyncMessages(HWND hWnd) {
	int WSAAsync = WSAAsyncSelect(s, hWnd, 0xAFFF, FD_READ|FD_CLOSE);
	if(hWnd != NULL) {
		if(WSAAsync > 0) {
			if(!is_win32s) {
				sprintf(debug_str, "\r\n[WSAWrapper] Async Messages initialization "
					"failed / Error code: %d", WSAGetLastError());
				OutputDebugString(debug_str);
			}
			return FALSE;
		}
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Async Messages initialized.");
			OutputDebugString(debug_str);
		}
		return TRUE;
	} else {
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Async Messages initialization "
				"failed / hWnd is NULL");
			OutputDebugString(debug_str);
		}
		return FALSE;
	}
}

EXPORT int CALLBACK GetWSAError() {
	return WSAGetLastError();
}

EXPORT BOOL CALLBACK CreateConnection(char* address, int port) {
	g_address = address;
	if(INVALID_SOCKET == (s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Connection failed / Error code: %d", WSAGetLastError());
			OutputDebugString(debug_str);
		}
		return FALSE;
	}
	ZeroMemory(&addr, sizeof(addr));
	if(!is_win32s) {
		sprintf(debug_str, "\r\n[WSAWrapper] Searching IP of %s:%d...", address, port);
		OutputDebugString(debug_str);
	}
	hostent = gethostbyname(address);
	addr.sin_family = AF_INET;
	if(hostent) {
		addr.sin_addr.S_un.S_addr = 
			inet_addr((char*)inet_ntoa(**(in_addr**)hostent->h_addr_list));
	} else {
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Connection failed / Error code: %d", WSAGetLastError());
			OutputDebugString(debug_str);
		}
		return FALSE;
	}
	addr.sin_port = htons(port);
	if(!is_win32s) {
		sprintf(debug_str, "\r\n[WSAWrapper] Connecting to %s:%d", address, port);
		OutputDebugString(debug_str);
	}
	if(SOCKET_ERROR == (connect(s, (sockaddr*)&addr, sizeof(addr)))) {
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Connection failed / Error code: %d", WSAGetLastError());
			OutputDebugString(debug_str);
		}
		return FALSE;
	}
	if(!is_win32s) {
		sprintf(debug_str, "\r\n[WSAWrapper] Successfully connected!");
		OutputDebugString(debug_str);
	}
	return TRUE;
}

EXPORT BOOL CALLBACK SendData(char* buff) {
	if(!is_win32s) {
		sprintf(debug_str, "\r\n[WSAWrapper] Sending data to %s...", g_address);
		OutputDebugString(debug_str);
	}
	if(SOCKET_ERROR == (send(s, buff, 512, 0))) {
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Sending failed / Error code: %d", WSAGetLastError());
			OutputDebugString(debug_str);
		}
		return FALSE;
	}
	if(!is_win32s) {
		sprintf(debug_str, "\r\n[WSAWrapper] Successfully sent!");
		OutputDebugString(debug_str);
	}
	return TRUE;
}


EXPORT char* CALLBACK GetInputBuffer() {
	int length = 0;
	if(recv_buff == NULL) {
		recv_buff = new char[BUFFER_LENGTH];
	}
	if(SOCKET_ERROR == (length = recv(s, (char*)recv_buff, BUFFER_LENGTH, 0))) {
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Connection with %s closed.", g_address);
			OutputDebugString(debug_str);
		}
		strcpy(recv_buff, "\r\n[Connection closed]");
	} else {
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Reading data from %s... (%d bytes)", g_address, length);
			OutputDebugString(debug_str);
		}
		recv_buff[length] = '\0';
	}
	if(!is_win32s) {
		sprintf(debug_str, "\r\n[WSAWrapper] Successfully read!");
		OutputDebugString(debug_str);
	}
	return recv_buff;
}

EXPORT void CloseConnection() {
	try {
		closesocket(s);
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Successfully closed!");
			OutputDebugString(debug_str);
		}
	} catch(...) {

	}
}



