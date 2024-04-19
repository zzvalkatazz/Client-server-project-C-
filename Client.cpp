#include<iostream>
#include<vector>
#include<winsock2.h>
#include<ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib")
//������� �� ����������� ���������
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
//������� �� ����������� ����������
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
	//������������� �� Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cout << "initialization Winsock failed\n";
		return -1;
	}
	//��������� �� �����
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		std::cout << "Error creating socket\n";
		WSACleanup();
		return -1;
	}
	//�������� �� ���������� �� ������ �� �������
	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET;//������,�� ���������� IPv4
	serverAddress.sin_port = htons(2221);//������ ����� �� ������� 2221
	//������������� �� IP ����� �� ����� � ������� �����
	if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0)
	{
		std::cout << "Invalid address\n";
		closesocket(clientSocket);
		WSACleanup();
		return-1;
	}
	//��������� ��� �������
	if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR)
	{
		std::cout << "Server connection error\n";
		closesocket(clientSocket);
		WSACleanup();
		return -1;
	}
	//��������� �� ���������� ����� �� �����������
	std::vector<int> inputData;
	int number;
	std::cout << "Enter integers for the vector:\n";
	while (std::cin >> number)
	{
		inputData.push_back(number);
	}
	int dataSize = static_cast<int>(inputData.size());
	//��������� �� ������� �� ������� �� �������
	sendall(clientSocket, &dataSize, sizeof(dataSize), 0);
	//��������� �� ������������ ����� �� �������
	sendall(clientSocket, inputData.data(), dataSize * sizeof(int), 0);
	//���������� �� ���������� ����� �� �������
	recvall(clientSocket, inputData.data(), dataSize * sizeof(int), 0);
	//��������� �� ���������� �����
	std::cout << "Sorted Data:";
	for (int num : inputData)
	{
		std::cout << num << " ";
	}
	std::cout<<"\n";
	//���������� �� Winsock
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}