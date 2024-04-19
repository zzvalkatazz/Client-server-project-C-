#include<iostream>
#include<vector>
#include<algorithm>
#include<thread>
#include<mutex>
#include<winsock2.h>
#include<ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib")

std::mutex mutex;
size_t sendall(SOCKET socket, const void* buff, size_t len, int flags)
{
	size_t total = 0;
	const char* buf = static_cast<const char*>(buff);
	while (total < len)
	{
		size_t sent = send(socket, buf + total, len - total, flags);
		if (sent == -1)
		{
			std::cout << "Send error\n";
			return -1;
		}
		total += sent;
	}
	return total;
}
size_t recvall(SOCKET socket, void* buff, size_t len, int flags)
{
	size_t total = 0;
	char* buf = static_cast<char*>(buff);
	while (total < len)
	{
		size_t received = recv(socket, buf + total, len - total, flags);
		if (received <= 0)
		{
			std::cout << "Error with receiving" << received << "\n";
			return received;
		}
		total += received;
	}
	return total;
}
void parallelQuicksort(std::vector<int>& arr, int low, int high)
{
	if (low < high)
	{
		int pivot = arr[high];
		int i = low - 1;
		for (int j = low; j < high; j++)
		{
			if (arr[j] <= pivot)
			{
				i++;
				std::swap(arr[i], arr[j]);
			}
		}
		std::swap(arr[i + 1], arr[high]);
		int pivotIndex = i + 1;
		std::thread leftThread(parallelQuicksort, std::ref(arr), low, pivotIndex - 1);
		std::thread rightThread(parallelQuicksort, std::ref(arr), pivotIndex + 1, high);

		leftThread.join();
		rightThread.join();
	}
}
//функция за обработка на клиент
void handleClient(SOCKET clientSocket)
{
	//получаване на данни от клиента
	int dataSize;
	int bytesReceived = recvall(clientSocket, reinterpret_cast<char*>(&dataSize), sizeof(dataSize), 0);
	if (bytesReceived == SOCKET_ERROR)
	{
		std::cout << "Error receiving data size:" << WSAGetLastError() << "\n";//взема последен код за грешка свързан със сокета
		closesocket(clientSocket);
		return;
	}
	std::vector<int> data(dataSize);
	bytesReceived = recvall(clientSocket, reinterpret_cast<char*>(data.data()), dataSize * sizeof(int), 0);
	if (bytesReceived == SOCKET_ERROR)
	{
		std::cout << "Error receiving unsorted data:" << WSAGetLastError() << "\n";
		closesocket(clientSocket);
		return;
	}
	//изпълняваме алгоритъма
	parallelQuicksort(data, 0, dataSize - 1);
	//изпращаме данните обратно на клиента
	int bytesSent = sendall(clientSocket, reinterpret_cast<char*>(data.data()), dataSize * sizeof(int), 0);
	if (bytesSent == SOCKET_ERROR)
	{
		std::cout << "Error sending sorted data:" << WSAGetLastError() << "\n";
	}
	closesocket(clientSocket);
}
int main()
{
	//инициализираме Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cout << "initialization Winsock failed\n";
		return -1;
	}
	//настройка на сървъра
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		std::cout << "Error creating socket\n";
		WSACleanup();
		return -1;
	}

	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(2221);
	//свързване на сокета с адреса
	if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR)
	{
		std::cout << "Error binding socket\n";
		closesocket(serverSocket);
		WSACleanup();
		return -1;
	}
	if (listen(serverSocket, 10) == SOCKET_ERROR)
	{
		std::cout << "error listening on socket\n";
		closesocket(serverSocket);
		WSACleanup();
		return-1;
	}
	std::cout << "Server listening on port 2221\n";
	//сървърът приема клиенти в безкраен цикъл
	while (true)
	{
		sockaddr_in clientAddress{};
		int clientAddressLen = sizeof(clientAddress);
		//Приема връзка с клиента;
		SOCKET clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddressLen);
		if (clientSocket == INVALID_SOCKET)
		{
			std::cout << "Error accepting connection\n";
			continue;
		}
		//обработва клиента в отделна нишка
		std::thread(handleClient, clientSocket).detach();
	}
	//почистване на Winsock
	closesocket(serverSocket);
	WSACleanup();
	
	return 0;
}