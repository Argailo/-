#include <iostream>
#include <cstring>
#include <vector>
#include <thread>
#include <cstdlib>
#include <string>
#include <list>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>

#define _WIN32_WINNT 0x501

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

const int PORT = 8080;

const int rows = 3; // номер этажа
const int cols = 3; // номер квартиры на этаже

std::vector<int> users;

std::map<std::string, bool> usersmap;   //ключ: сокет клиента; значение: true - "слышу шум", false - "все тихо"

// Define your messaging protocol
struct Message {
    int  loud;// You can define different message types
    char data[1024]; // Actual message data
};

void recv_fun( int client_socket, Message receivedMessage){
    int nread;
    while((nread = recv(client_socket, receivedMessage.data, sizeof(receivedMessage.data), 0)) > 0){
        
        if (receivedMessage.data[0] == 's') {

            for (const auto& pair : usersmap) {
                std::cout << "Socket: " << pair.first << ", Loud: " << (pair.second ? "true" : "false") << std::endl;
            }

            if (users.size() < rows * cols) {
                std::string error = "error";
                send(client_socket, error.c_str(), sizeof(error), 0);
                continue;
            }

            bool noise = 0;
            std::vector<std::list<int>> neighbors;

            for (int i = 0; i < users.size(); i++) {
                  
                if (usersmap[std::to_string(users[i])]) {    //cлышен шум
                    noise = 1;

                    int m = i / cols; //номер этажа
                    int n = i - m * cols;    //номер квартиры на этаже
                    //std::cout << m << " " << n << '\n';
                    std::list<int> neighb;
                    
                    //определение соседей
                    for (int r = m - 1; r <= m + 1; r++) {

                        for (int c = n - 1; c <= n + 1; c++) {
                            if ((r < rows) && (c < cols) && (r>=0) && (c>=0)) {
                                int n = r * cols + c;   //индекс сокета соседа
                                neighb.push_back(users[n]);
                            }
                        }
                    }

                    neighbors.push_back(neighb);

                }
            }

            //for (size_t i = 0; i < neighbors.size(); ++i) {
            //    std::cout << "List " << i + 1 << ": ";
            //    for (const int& elem : neighbors[i]) {
            //        std::cout << elem << " ";
            //    }
            //    std::cout << std::endl;
            //}

            // Инициализация множества с элементами из первого списка


            if (noise) {
            std::set<int> commonElements(neighbors[0].begin(), neighbors[0].end());

            // Перебор остальных списков
            for (size_t i = 1; i < neighbors.size(); ++i) {
                std::set<int> currentSet(neighbors[i].begin(), neighbors[i].end());
                std::set<int> intersection;

                // Нахождение пересечения с текущим списком
                std::set_intersection(commonElements.begin(), commonElements.end(),
                    currentSet.begin(), currentSet.end(),
                    std::inserter(intersection, intersection.begin()));

                commonElements = intersection;

                // Если пересечение пустое, выходим из цикла
                if (commonElements.empty()) {
                    break;
                }
            }

            std::list<int> commonList(commonElements.begin(), commonElements.end());

            //for (int elem : commonList) {
            //    std::cout << elem << " ";
            //}
            //std::cout << std::endl;

            std::string loud_neighbor = " ";

            for (int elem : commonList) {
                for (int i = 0; i < users.size(); i++) {
                    if (elem == users[i]) {
                        loud_neighbor += std::to_string(i) + " ";
                    }
                }
            }           
            send(client_socket, loud_neighbor.c_str(), sizeof(loud_neighbor), 0);
            
            continue;
            }

            std::string loud_neighbor = "not found";
            send(client_socket, loud_neighbor.c_str(), sizeof(loud_neighbor), 0);
            
            continue;
        }
        else
        {
            //std::cout << receivedMessage.data << '\n';
            //receivedMessage.loud = (receivedMessage.data[0] - '0');


            char text[1017];
            char receiver_port[3];
  //          if (receivedMessage.data[0] - '0' > 0) {
  //              //std::stringstream ss;
  //              //ss << client_socket << " ";
  //              //std::string fd_str = ss.str();
  //              //louds += fd_str;
  ///*              std::cout << receivedMessage.data[0] << '\n';*/
  //              usersmap[std::to_string(client_socket)] = true;
  //          }
  //          else {
  //              usersmap[std::to_string(client_socket)] = false;
  //          }
            usersmap[std::to_string(client_socket)] = (receivedMessage.data[0] - '0' > 0);  //записываем есть ли шум у клиента

            for (int i = 0; i < 3; i++){receiver_port[i] = receivedMessage.data[i + 2];}
            for (int i = 0; i < 1017; i++){text[i]= receivedMessage.data[i + 6];}
            std::cout << "Received message's noise level " << receivedMessage.data[0] << ": " << text<< "\n";// << std::endl;
            std::cout << "Receiver port is: " << receivedMessage.data[2] << receivedMessage.data[3] << receivedMessage.data[4] << std::endl;

            

            for (auto it = users.begin(); it != users.end(); ++it)  
                send(*it, text, nread, 0);           
        }      
    }   
}


int main() {



    WSADATA wsaData; // служебная структура для хранение информации
    // о реализации Windows Sockets
    // старт использования библиотеки сокетов процессом
    // (подгружается Ws2_32.dll)
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    // Если произошла ошибка подгрузки библиотеки
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << "\n";
        return result;
    }

    struct addrinfo* addr = NULL;// структура, хранящая информацию
    // об IP-адресе  слущающего сокета

    // Шаблон для инициализации структуры адреса
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_INET; // AF_INET определяет, что будет
    // использоваться сеть для работы с сокетом
    hints.ai_socktype = SOCK_STREAM; // Задаем потоковый тип сокета
    hints.ai_protocol = IPPROTO_TCP; // Используем протокол TCP
    hints.ai_flags = AI_PASSIVE; // Сокет будет биндиться на адрес,
    // чтобы принимать входящие соединения


    // Инициализируем структуру, хранящую адрес сокета - addr
    // Наш HTTP-сервер будет висеть на 8080-м//8081-м порту локалхоста
    result = getaddrinfo("127.0.0.1", "8080", &hints, &addr);
    // Если инициализация структуры адреса завершилась с ошибкой,
    // выведем сообщением об этом и завершим выполнение программы
    if (result != 0) {
        std::cerr << "getaddrinfo failed: " << result << "\n";
        WSACleanup(); // выгрузка библиотеки Ws2_32.dll
        return 1;
    }




    // Create socket
    int serverSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket\n";
        return -1;
    }
    // Bind the socket to a specific address and port
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding socket\n";
        closesocket(serverSocket);
        return -1;
    }


    // Listen for incoming connections
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error listening for connections\n";
        closesocket(serverSocket);
        return -1;
    }

    std::cout << "Server listening on ports " << PORT << std::endl;
    std::vector<std::thread> threads;

   // std::string louds;
    // Accept connections and handle messages
    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        users.push_back(clientSocket);
        usersmap[std::to_string(clientSocket)] = false;

        if (clientSocket == -1) {
            std::cerr << "Error accepting connection\n";
            continue;
        }
        
        std::cout << "Connection accepted from " << inet_ntoa(clientAddr.sin_addr) << std::endl;

        // Handle messages from the client
        Message receivedMessage;
        
        threads.push_back(std::thread(recv_fun, clientSocket, receivedMessage));
        threads.back().detach();


        //std::cout << "Connection closed with " << inet_ntoa(clientAddr.sin_addr) << std::endl;
    }


    // Close the server socket
    closesocket(serverSocket);

    return 0;
}