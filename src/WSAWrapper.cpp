//  Copyright © 2023, 2024 Dmitry Tretyakov (aka. Tinelix)
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
#include <stdarg.h>
#include "WSAWrapper.h"

#ifdef _MSVC2005G
	#define Win32OutputLog(...) { if(!is_win32s && debug) { char str[256]; sprintf_s(str, __VA_ARGS__); OutputDebugString(str); } }
#else
	#define Win32OutputLog(...) { if(!is_win32s && debug) { char str[256]; sprintf(str, __VA_ARGS__); OutputDebugString(str); } }
#endif

#ifndef _MSVC2005G
	#pragma comment(lib, "wsock32.lib");
#endif

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
int error_code = 0;
BOOL debug;

struct NetworkStatistics stats;

int WINAPI DllMain(HINSTANCE hInst, DWORD fdReas, PVOID pvRes) {
	switch(fdReas) {
		case DLL_PROCESS_ATTACH:
			error_code = 0;
			// Checking if it's Win32s
			#ifdef _MSVC2005G
				is_win32s = FALSE;
			#else
				if (GetVersion() & 0x80000000 && (GetVersion() & 0xFF) == 3) {
					is_win32s = TRUE;
				}
				else {
					is_win32s = FALSE;
				}
			#endif

			debug = FALSE;
			
			if(!is_win32s) {
				Win32OutputLog("\r\nWinsock Wrapper - Win32 DLL implementation"
					"\r\nCopyright © 2023 Dmitry Tretyakov (aka. Tinelix). Licensed under LGPLv2.1."
					"\r\nSource code: https://github.com/tinelix/WSAWrapper\r\n");
			}
			InitializeWinSock();
			break;
		case DLL_PROCESS_DETACH:
			CloseConnection();
			WSACleanup();
			if(!is_win32s) {
				Win32OutputLog("\r\nWinsock Wrapper is shutting down...\r\n");
			}
			break;
	}
	
	return TRUE;
}

EXPORT void CALLBACK EnableDebugging(BOOL value) {
	if(!is_win32s) {
		debug = value;
		if(debug == TRUE) {
			Win32OutputLog("\r\n[WSAWrapper] Debug Enabled");
		}
	} else {
		MessageBox(NULL, "Direct DLL debugging is not possible in the current Windows version.", 
			"WSAWrapper", MB_ICONSTOP|MB_OK);
	}
}

EXPORT BOOL CALLBACK InitializeWinSock() {
	WSADATA wsadata;
	if(FAILED(WSAStartup(MAKEWORD(1,1), &wsadata))) {
		Win32OutputLog(debug_str, "\r\n[WSAWrapper] Winsock initialization "
			"failed / Error Code: %d", WSAGetLastError());
		return FALSE;
	}
	Win32OutputLog("\r\n[WSAWrapper] Winsock 1.1+ initialized.");
	return TRUE;
}

EXPORT BOOL CALLBACK EnableCustomAsyncMessages(HWND hWnd, int message, int nStatus) {
	if(message != 0xAFFE) {
		int WSAAsync = WSAAsyncSelect(s, hWnd, 0xAFFF, nStatus);
		if(hWnd != NULL) {
			if(WSAAsync > 0 && debug) {
				error_code = WSAGetLastError();
				Win32OutputLog("\r\n[WSAWrapper] Async Messages initialization "
					"failed / Error code: %d", error_code);
				return FALSE;
			}
			Win32OutputLog("\r\n[WSAWrapper] Async Messages initialized.");
			return TRUE;
		} else {
			Win32OutputLog(debug_str, "\r\n[WSAWrapper] Async Messages initialization "
				"failed / hWnd is NULL");
			return FALSE;
		}
	} else {
		Win32OutputLog(debug_str, "\r\n[WSAWrapper] Async Messages initialization "
			"failed / Invalid message code");
		return FALSE;
	}
}

EXPORT BOOL CALLBACK EnableAsyncMessages(HWND hWnd) {
	int WSAAsync = WSAAsyncSelect(s, hWnd, 0xAFFF, FD_READ);
	if(hWnd != NULL) {
		if(WSAAsync > 0) {
			Win32OutputLog("\r\n[WSAWrapper] Async Messages initialization "
				"failed / Error Code: %d", WSAGetLastError());
			return EnableCustomAsyncMessages(hWnd, 0xE0001, FD_CLOSE);
		}
		Win32OutputLog("\r\n[WSAWrapper] Async Messages initialized.");
		return TRUE;
	} else {
		Win32OutputLog("\r\n[WSAWrapper] Async Messages initialized.");
		return FALSE;
	}
}

EXPORT int CALLBACK GetWSAError() {
	return error_code;
}

EXPORT BOOL CALLBACK CreateConnection(char* address, int port) {
	error_code = 0;

	stats.packets_read = 0;
	stats.packets_sent = 0;
	stats.total_bytes_read = 0;
	stats.total_bytes_sent = 0;

	g_address = address;
	if(INVALID_SOCKET == (s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
		error_code = WSAGetLastError();
		Win32OutputLog("\r\n[WSAWrapper] Socket initialization failed / Error Code: %d", error_code);
		return FALSE;
	}
	ZeroMemory(&addr, sizeof(addr));

	Win32OutputLog("\r\n[WSAWrapper] Searching IP Address of %s:%d...", address, port);

	hostent = gethostbyname(address);
	addr.sin_family = AF_INET;
	if(hostent) {
		addr.sin_addr.S_un.S_addr = 
			inet_addr((char*)inet_ntoa(**(in_addr**)hostent->h_addr_list));
		Win32OutputLog("\r\n[WSAWrapper] IP Address Found: %s:%d -> %s:%d", address, port,
			(char*)inet_ntoa(**(in_addr**)hostent->h_addr_list), port);
	} else {
		error_code = WSAGetLastError();
		Win32OutputLog("\r\n[WSAWrapper] IP Search failed / Error Code: %d", error_code);
		return FALSE;
	}
	addr.sin_port = htons(port);
	Win32OutputLog("\r\n[WSAWrapper] Connecting to %s:%d...", address, port);
	if(SOCKET_ERROR == (connect(s, (sockaddr*)&addr, sizeof(addr)))) {
		error_code = WSAGetLastError();
		Win32OutputLog("\r\n[WSAWrapper] Connection failed / Error Code: %d", error_code);
		return FALSE;
	}
	Win32OutputLog("\r\n[WSAWrapper] Successfully connected!");
	error_code = 0;
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
			error_code = WSAGetLastError();
			Win32OutputLog("\r\n[WSAWrapper] Socket initialization failed / "
				"Error Code: %d", error_code);
			return 0;
		}
		ZeroMemory(&addr, sizeof(addr));
		Win32OutputLog("\r\n[WSAWrapper] Searching IP Address of %s:%d...", address, port);
		WSAAsyncGetHostByName(hWnd, message, g_address, hostent_char, sizeof(hostent_char));
		return 2;
	} else {
		hostent = (struct hostent*)hostent_char;
		addr.sin_family = AF_INET;
		if(hostent) {
			addr.sin_addr.S_un.S_addr = inet_addr((char*)inet_ntoa(**(in_addr**)hostent->h_addr_list));
			Win32OutputLog("\r\n[WSAWrapper] Redirecting: %s:%d -> %s:%d", g_address, g_port,
				(char*)inet_ntoa(**(in_addr**)hostent->h_addr_list), g_port);
		} else {
			Win32OutputLog("\r\n[WSAWrapper] IP Search failed / Error Code: %d", WSAGetLastError());
			return 0;
		}
		addr.sin_port = htons(g_port);
		Win32OutputLog("\r\n[WSAWrapper] Connecting to %s:%d...", g_address, g_port);
		if(SOCKET_ERROR == (connect(s, (sockaddr*)&addr, sizeof(addr)))) {
			error_code = WSAGetLastError();
			Win32OutputLog("\r\n[WSAWrapper] Connection failed / Error Code: %d", error_code);
			return 0;
		}
		Win32OutputLog("\r\n[WSAWrapper] Successfully connected!");
		return 1;
	}
}

EXPORT BOOL CALLBACK SendData(char* buff) {
	Win32OutputLog("\r\n[WSAWrapper] Sending data to %s... (%d bytes)",
		g_address, strlen(buff));

	int length = send(s, buff, strlen(buff), 0);

	if(SOCKET_ERROR == length) {
		error_code = WSAGetLastError();
		if(error_code > 0) {				// workaround: check error code
			Win32OutputLog("\r\n[WSAWrapper] Send Data failed / Error Code: %d", error_code);
			return FALSE;
		} else {
			stats.packets_sent = stats.total_bytes_sent / BUFFER_LENGTH;
			stats.total_bytes_sent += strlen(buff);
			error_code = 0;
			return TRUE;
		}
	} else {
		stats.packets_sent = stats.total_bytes_sent / BUFFER_LENGTH;
		stats.total_bytes_sent += strlen(buff);
	}
	error_code = 0;
	return TRUE;
}


EXPORT char* CALLBACK GetInputBuffer(SOCKET s) {
	int length = 0;
	recv_buff = new char[BUFFER_LENGTH];
	length = recv(s, (char*)recv_buff, BUFFER_LENGTH, 0);
	if(SOCKET_ERROR == length) {
		error_code = WSAGetLastError();
		if(error_code == 10035) {	// workaround: if it's non-blocking socket
			Win32OutputLog("\r\n[WSAWrapper] Buffer Read error / "
				" Error Code: %d", g_address, error_code);
			#ifdef _MSVC2005G
				sprintf_s(recv_buff, 200, "\r\n[Missing Socket Data]");
			#else
				sprintf(recv_buff, "\r\n[Missing Socket Data]");
			#endif
			Sleep(200);
		} else if(error_code > 0) {			// workaround: check error code
			Win32OutputLog("\r\n[WSAWrapper] Connection with %s closed / "
				" Error Code: %d", g_address, error_code);
			closesocket(s);
		} else {
			stats.total_bytes_read += length;
			stats.packets_read = stats.total_bytes_read / BUFFER_LENGTH;
			recv_buff[length] = '\0';
			Win32OutputLog("\r\n[WSAWrapper] Receiving Data from %s... (%d bytes)",
				g_address, length);
		}
	} else {
		stats.total_bytes_read += length;
		stats.packets_read = stats.total_bytes_read / BUFFER_LENGTH;
		recv_buff[length] = '\0';
		Win32OutputLog("\r\n[WSAWrapper] Receiving Data from %s... (%d bytes)",
			g_address, length);
	}
	return recv_buff;
}

EXPORT struct NetworkStatistics GetNetworkStatistics() {
	return stats;
}

EXPORT void CALLBACK CloseConnection() {
	try {
		error_code = 0;
		closesocket(s);
		Win32OutputLog("\r\n[WSAWrapper] Successfully closed!");
	} catch(...) {

	}
}



