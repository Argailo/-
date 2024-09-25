#include <iostream>
#include <sstream>
#include <string>

// Для корректной работы freeaddrinfo в MinGW
// Подробнее: http://stackoverflow.com/a/20306451
#define _WIN32_WINNT 0x501

#include <WinSock2.h>
#include <WS2tcpip.h>

// Необходимо, чтобы линковка происходила с DLL-библиотекой
// Для работы с сокетам
#pragma comment(lib, "Ws2_32.lib")

using std::cerr;

const int PORT = 8080;


struct Message {
    int loud; // You can define different message types
    char data[1024]; // Actual message data
};

int main()
{
    WSADATA wsaData; // служебная структура для хранение информации
    // о реализации Windows Sockets
    // старт использования библиотеки сокетов процессом
    // (подгружается Ws2_32.dll)
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Если произошла ошибка подгрузки библиотеки
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << "\n";
        return result;
    }

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    //std::cout << clientSocket;
    if (clientSocket == -1) {
        std::cerr << "Error creating socket\n";
        return -1;
    }
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to server\n";
        closesocket(clientSocket);
        WSACleanup();
        return -1;
}

    std::cout << "Connected to server\n";


    while (true) {
    // Example: Send a message of type 1
    std::string s;
    char recvAddr[4];
    std::cout << "Enter a: |loud(1/0) PORT_of_receiver Message| // or show :";
    std::getline(std::cin, s);
    //std::cin.getline(messageData, sizeof(messageData));
    //std::cout << "Enter Receiver port: ";

    Message sendMessage;
    //sendMessage.loud = 1;
    //sendMessage.data[0] =   sendMessage.loud + '0';

    //strcat(sendMessage.data, recvAddr);
    memcpy(sendMessage.data, s.c_str(), s.length()+1);

    send(clientSocket, sendMessage.data, sizeof(sendMessage.data), 0);

    // Receive and process the server's response
    Message receivedMessage;

    recv(clientSocket, receivedMessage.data, sizeof(receivedMessage.data), 0);
    
    std::cout << "Server response " << receivedMessage.data << std::endl;
    }

    // Close the client socket
    closesocket(clientSocket);

    return 0;
}
