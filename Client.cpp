#include<iostream>
#include<vector>
#include<winsock2.h>
#include<ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib")
//функция за гарантирано изпращане
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
//фунцкия за гарантирано получаване
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
int main()
{
	//Инициализаиця на Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cout << "initialization Winsock failed\n";
		return -1;
	}
	//създаване на сокет
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		std::cout << "Error creating socket\n";
		WSACleanup();
		return -1;
	}
	//Задаване на информация за адреса на сървъра
	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET;//Указва,че използваме IPv4
	serverAddress.sin_port = htons(2221);//Задава порта на сървъра 2221
	//Преобразуване на IP адрес от текст в бинарно число
	if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0)
	{
		std::cout << "Invalid address\n";
		closesocket(clientSocket);
		WSACleanup();
		return-1;
	}
	//свързване със сървъра
	if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR)
	{
		std::cout << "Server connection error\n";
		closesocket(clientSocket);
		WSACleanup();
		return -1;
	}
	//въвеждане на несортиран масив от потребителя
	std::vector<int> inputData;
	int number;
	std::cout << "Enter integers for the vector:\n";
	while (std::cin >> number)
	{
		inputData.push_back(number);
	}
	int dataSize = static_cast<int>(inputData.size());
	//изпращане на размера на данните на сървъра
	sendall(clientSocket, &dataSize, sizeof(dataSize), 0);
	//изпращане на несортирания масив на сървъра
	sendall(clientSocket, inputData.data(), dataSize * sizeof(int), 0);
	//Получаване на сортирания масив от сървъра
	recvall(clientSocket, inputData.data(), dataSize * sizeof(int), 0);
	//Извеждане на сортирания масив
	std::cout << "Sorted Data:";
	for (int num : inputData)
	{
		std::cout << num << " ";
	}
	std::cout<<"\n";
	//Почистване на Winsock
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}