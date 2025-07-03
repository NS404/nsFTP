#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <stdio.h>
#include <string>
#include <iostream>
using namespace std;

#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "21"
#define DEFAULT_BUFLEN 512

int __cdecl main(int argc, char* argv[])
{

	WSADATA wsaData; //contains info about Winsock impl
	SOCKET ConnectSocket = INVALID_SOCKET;
	int recvbuflen = DEFAULT_BUFLEN;
	const char* sendbuf = "this is a test";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;

	// Validate the params
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;


	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed %d\n", iResult);
		WSACleanup();
		return 1;
	}

	//Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		//Create a SOCKET for connecting to a server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("Error at socket(): %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	string command;

	do {
		cout << "NSFTP > ";
		getline(cin, command);

		const char* cmd = command.c_str();
		

		// Send an initial buffer
		iResult = send(ConnectSocket, cmd, (int)strlen(cmd), 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
		printf("Bytes Sent: %ld\n", iResult);
		
		transform(command.begin(), command.end(), command.begin(), ::toupper);

	} while (command != "QUIT");

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();
	

	//// shutdown the connection for sending
	//// the client can still use the ConnectSocket for recieving data
	//iResult = shutdown(ConnectSocket, SD_SEND);
	//if (iResult == SOCKET_ERROR) {
	//	printf("shutdown failed: %d\n", WSAGetLastError());
	//	closesocket(ConnectSocket);
	//	WSACleanup();
	//	return 1;
	//}

	//// Recieve data until the server closes the connection
	//do {
	//	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	//	if (iResult > 0)
	//		printf("Bytes recieved: %d\n", iResult);
	//	else if (iResult == 0)
	//		printf("Connection closed\n");
	//	else
	//		printf("recv failed %d\n", WSAGetLastError());
	//} while (iResult > 0);

	
	return 0;
}