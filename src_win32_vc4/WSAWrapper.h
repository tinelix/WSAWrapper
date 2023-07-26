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


#define EXPORT extern "C" __declspec (dllexport)

struct NetworkStatistics {
	int packets_read;
	int packets_sent;
	int total_bytes_sent;
	int total_bytes_read;
} NetworkStats;

EXPORT BOOL CALLBACK InitializeWinSock();
EXPORT BOOL CALLBACK EnableCustomAsyncMessages(HWND hWnd, int message, int nStatus);
EXPORT BOOL CALLBACK EnableAsyncMessages(HWND hWnd);
EXPORT int CALLBACK GetWSAError();
EXPORT BOOL CALLBACK CreateConnection(char* address, int port);
EXPORT int CALLBACK CreateAsyncConnection(char* address, int port, int part, int message, HWND hWnd);
EXPORT BOOL CALLBACK SendData(char* buff);
EXPORT char* CALLBACK GetInputBuffer();
EXPORT struct NetworkStatistics GetNetworkStatistics();
EXPORT void CloseConnection();