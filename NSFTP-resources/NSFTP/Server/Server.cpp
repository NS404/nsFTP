#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include "replies.h"

#include <string>
#include <iostream>

using namespace std;


#pragma comment(lib, "Ws2_32.lib")

#define CONTROL_CONNECTION_PORT "21"
#define DATA_CONNECTION_PORT "20"
#define DEFAULT_BUFLEN 512


int main(void)
{	
	
	SOCKET CTL_CON_SOCK = INVALID_SOCKET;
	int iResult;

	// initialize Winsock
	WSADATA wsaData;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("Winsock Initialization failed: %d\n", iResult);
		return 1;
	}

	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, CONTROL_CONNECTION_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed %d\n", iResult);
		WSACleanup();
		return 1;
	}

	CTL_CON_SOCK = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (CTL_CON_SOCK == INVALID_SOCKET) {
		printf("Failed to create socket %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = bind(CTL_CON_SOCK, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("failed to bind socket %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	
	if (listen(CTL_CON_SOCK, SOMAXCONN) == SOCKET_ERROR) {
		printf("listen failed %d\n", WSAGetLastError());
		WSACleanup();
		closesocket(CTL_CON_SOCK);
		return 1;
	}

	SOCKET CLI_SOCK = INVALID_SOCKET;

	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];


	// Accept a client socket
	CLI_SOCK = accept(CTL_CON_SOCK, NULL, NULL);
	if (CLI_SOCK == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(CTL_CON_SOCK);
		WSACleanup();
		return 1;
	}

	char command[BUFSIZ];

	do {
		iResult = recv(CLI_SOCK, command, BUFSIZ, 0);
		if (iResult == SOCKET_ERROR) {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(CLI_SOCK);
			break;
		}
		printf("Bytes Recieved: %d\n", iResult);
		printf("CLI: %s", command);


	} while (command != "QUIT");


	closesocket(CTL_CON_SOCK);
	WSACleanup();


	return 0;
}

