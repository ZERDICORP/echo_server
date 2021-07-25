#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <ws2tcpip.h>

#define DEFAULT_BUFFER_LENGTH 512
#define DEFAULT_PORT "80"

int forceExit(std::string sMessage)
{
	std::cerr << sMessage << std::endl;

	WSACleanup();
	
	return 1;
}

int main()
{
	WSADATA wsaData;

	SOCKET listeningSocket = INVALID_SOCKET;
	SOCKET clientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;
	
	/*
		Initialize Winsock.
	*/
	if (WSAStartup(MAKEWORD(2,2), &wsaData))
	{
		std::cout << "[error]: WSAStartup(...) failed.." << std::endl;
		return 1;
	}

	std::cout << "[info]: server start on port " << DEFAULT_PORT << ".." << std::endl;

	while (true)
	{
		std::cout << "[info]: start listening.." << std::endl;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		/*
			Resolve the server address and port.
		*/
		if (getaddrinfo(NULL, DEFAULT_PORT, &hints, &result))
			return forceExit("[error]: getaddrinfo(...) failed..");

		/*
			Create a SOCKET for connecting to server.
		*/
		listeningSocket = socket(result -> ai_family, result -> ai_socktype, result -> ai_protocol);
		if (listeningSocket == INVALID_SOCKET)
		{
			freeaddrinfo(result);
			return forceExit("[error]: socket(...) failed..\n[text]: " + WSAGetLastError());
		}

		/*
			Setup the TCP listening socket.
		*/
		if (bind(listeningSocket, result -> ai_addr, (int)result -> ai_addrlen) == SOCKET_ERROR)
		{
			freeaddrinfo(result);
			closesocket(listeningSocket);
			return forceExit("[error]: bind(...) failed..\n[text]: " + WSAGetLastError());
		}

		freeaddrinfo(result);

		if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			closesocket(listeningSocket);
			return forceExit("[error]: listen(...) failed..\n[text]: " + WSAGetLastError());
		}

		/*
			Accept a client socket.
		*/
		clientSocket = accept(listeningSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET)
		{
			closesocket(listeningSocket);
			return forceExit("[error]: accept(...) failed..\n[text]: " + WSAGetLastError());
		}

		/*
			No longer need server socket.
		*/
		closesocket(listeningSocket);

		/*
			Buffer for incoming request.
		*/
		char buffer[DEFAULT_BUFFER_LENGTH];

		/*
			Receive until the peer shuts down the connection.
		*/
		do
		{
			int iResult = recv(clientSocket, buffer, DEFAULT_BUFFER_LENGTH, 0);
			if (iResult > 0)
			{
				if (iResult > 2)
				{
					/*
						Send buffer back to the sender.
					*/					
					int iSendResult = send(clientSocket, buffer, iResult, 0);
					if (iSendResult == SOCKET_ERROR)
					{
						closesocket(clientSocket);
						return forceExit("[error]: send(...) failed..\n[text]: " + WSAGetLastError());
					}

					std::cout << "[info]: bytes received: " << iResult << std::endl;
					std::cout << "[info]: bytes sent: " << iSendResult << std::endl;	
				}
			}
			else if (iResult == 0)
			{
				std::cout << "[info]: connection closing.." << std::endl;
				break;
			}
			else 
			{
				closesocket(clientSocket);
				return forceExit("[error]: recv(...) failed..\n[text]: " + WSAGetLastError());
			}
		}
		while (true);

		/*
			Shutdown the connection since we're done.
		*/
		if (shutdown(clientSocket, SD_SEND) == SOCKET_ERROR)
		{
			closesocket(clientSocket);
			return forceExit("[error]: shutdown(...) failed..\n[text]: " + WSAGetLastError());
		}

		/*
			Cleanup.
		*/
		closesocket(clientSocket);
	}

	WSACleanup();

	system("pause > nul");
	return 0;
}