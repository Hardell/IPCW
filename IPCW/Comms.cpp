#include "Comms.h"
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#include <iostream>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "19834"

SOCKET ConnectSocket = INVALID_SOCKET;
char recvbuf[DEFAULT_BUFLEN];
int iResult;

Comms::Comms()
{
}


Comms::~Comms()
{
}

bool Comms::Setup()
{

	WSADATA wsaData;
	
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char *sendbuf = "this is a test";

	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return false;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return false;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("Error at socket(): %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return false;
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
		return false;
	}
	printf("CONNECtED\n");

	return true;
}

void Comms::Receive()
{

	iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
	if (iResult > 0){
		std::cout << "Message recieved: " << recvbuf << "\nTotal " << iResult << " bytes.\n";
	}
	else if (iResult == 0){
		std::cout << "Closing connection... " << std::endl;
	}
	else{
		std::cout << "Recieving failed.\nError code: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}
}

void Comms::Receive2(int* state)
{

	iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
	if (iResult > 0){
		std::cout << "Message recieved: " << (int)recvbuf[0] << "\n";
		if ((int)recvbuf[0] == 0 || (int)recvbuf[0] == 1)
			*state = (int)recvbuf[0];

		//std::cout << "Message recieved: " << *state << " " << state;
	}
	else if (iResult == 0){
//		std::cout << "Closing connection... " << std::endl;
	}
	else{
		//std::cout << "Recieving failed.\nError code: " << WSAGetLastError() << std::endl;
		//closesocket(ConnectSocket);
		//WSACleanup();
		return;
	}
}

void Comms::Send(char* message)
{
	iResult = send(ConnectSocket, message, 5, 0);
	if (iResult == SOCKET_ERROR){
		std::cout << "Sending data failed.\nError code: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}
	else{
		std::cout << "Sent " << iResult << " bytes : " << message << "\n";
	}
}

void Comms::SendTowerData(std::vector<char>& message)
{
	for (int i = 0; i < message.size(); i++)
		std::cout << int(message.at(i)) << "\n";
	char* messageP = reinterpret_cast<char*>(message.data());
	iResult = send(ConnectSocket, messageP, message.size(), 0);
	if (iResult == SOCKET_ERROR){
		std::cout << "Sending data failed.\nError code: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}
	else{
		std::cout << "Sent " << iResult << " bytes : " << messageP << "\n";
	}
}

void Comms::Close()
{
	iResult = shutdown(ConnectSocket, SD_BOTH);
	if (iResult == SOCKET_ERROR){
		std::cout << "shutdown client failed.\nError code: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}
	closesocket(ConnectSocket);
	WSACleanup();
}




