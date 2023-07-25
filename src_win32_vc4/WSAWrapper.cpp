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
char hostent_char[MAXGETHOSTSTRUCT];
struct sockaddr_in addr;
char* g_address;
int g_port;
char debug_str[400];
char ip_addr[40];
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

EXPORT BOOL CALLBACK EnableCustomAsyncMessages(HWND hWnd, int message, int nStatus) {
	if(message != 0xAFFE) {
		int WSAAsync = WSAAsyncSelect(s, hWnd, 0xAFFF, nStatus);
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
	} else {
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Async Messages initialization "
				"failed / Invalid message code");
			OutputDebugString(debug_str);
		}
		return FALSE;
	}
}

EXPORT BOOL CALLBACK EnableAsyncMessages(HWND hWnd) {
	int WSAAsync = WSAAsyncSelect(s, hWnd, 0xAFFF, FD_READ);
	if(hWnd != NULL) {
		if(WSAAsync > 0) {
			if(!is_win32s) {
				sprintf(debug_str, "\r\n[WSAWrapper] Async Messages initialization "
					"failed / Error code: %d", WSAGetLastError());
				OutputDebugString(debug_str);
			}
			return EnableCustomAsyncMessages(hWnd, 0xE0001, FD_CLOSE);
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
		sprintf(debug_str, "\r\n[WSAWrapper] Connecting to %s:%d...", address, port);
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

EXPORT int CALLBACK CreateAsyncConnection(
							char* address, int port, int part, int message, HWND hWnd
					 ) {
	if(part == 0) {
		g_address = address;
		g_port = port;
		PostMessage(hWnd, 0xAFFE, 0, 0);
		if(INVALID_SOCKET == (s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
			if(!is_win32s) {
				sprintf(debug_str, "\r\n[WSAWrapper] Socket initialization failed / "
					"Error code: %d", WSAGetLastError());
				OutputDebugString(debug_str);
			}
			return 0;
		}
		ZeroMemory(&addr, sizeof(addr));
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Searching IP of %s:%d...", address, port);
			OutputDebugString(debug_str);
		}
		WSAAsyncGetHostByName(hWnd, message, g_address, hostent_char, sizeof(hostent_char));
		return 2;
	} else {
		hostent = (struct hostent*)hostent_char;
		addr.sin_family = AF_INET;
		if(hostent) {
			addr.sin_addr.S_un.S_addr = inet_addr((char*)inet_ntoa(**(in_addr**)hostent->h_addr_list));
		} else {
			if(!is_win32s) {
				sprintf(debug_str, "\r\n[WSAWrapper] Connection failed / Error code: %d", WSAGetLastError());
				OutputDebugString(debug_str);
			}
			return 0;
		}
		addr.sin_port = htons(g_port);
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Connecting to %s:%d...", g_address, g_port);
			OutputDebugString(debug_str);
		}
		if(SOCKET_ERROR == (connect(s, (sockaddr*)&addr, sizeof(addr)))) {
			if(!is_win32s) {
				sprintf(debug_str, "\r\n[WSAWrapper] Connection failed / Error code: %d", WSAGetLastError());
				OutputDebugString(debug_str);
			}
			return 0;
		}
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Successfully connected!");
			OutputDebugString(debug_str);
		}
		return 1;
	}
}

EXPORT BOOL CALLBACK SendData(char* buff) {
	if(!is_win32s) {
		sprintf(debug_str, "\r\n[WSAWrapper] Sending data to %s...", g_address);
		OutputDebugString(debug_str);
	}
	if(SOCKET_ERROR == (send(s, buff, strlen(buff), 0))) {
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Sending failed / Error code: %d", WSAGetLastError());
			OutputDebugString(debug_str);
		}
		return FALSE;
	}
	return TRUE;
}


EXPORT char* CALLBACK GetInputBuffer() {
	int length = 0;
	recv_buff = new char[BUFFER_LENGTH];
	length = recv(s, (char*)recv_buff, BUFFER_LENGTH, 0);
	if(SOCKET_ERROR == length) {
		if(!is_win32s) {
			sprintf(debug_str, "\r\n[WSAWrapper] Connection with %s closed / "
				" Error code: %d", g_address, WSAGetLastError());
			OutputDebugString(debug_str);
		}
		closesocket(s);
		sprintf(recv_buff, "[WSAWrapper] 0xE0001\r\n");
	} else {
		recv_buff[length] = '\0';
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



