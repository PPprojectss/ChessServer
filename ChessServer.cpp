#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <string>
#include <thread>

#include <vector>

#define buf 512

struct Room 
{ 
    SOCKET* player1; 
    SOCKET* player2; 
    bool started; 
};


std::vector<std::thread*> workers;
std::vector<Room*> rooms;
int g_id = 0;

void sendMessage(SOCKET* client, std::string msg)
{
    send(*client, msg.c_str(), buf, 0);
}

std::string receiveMassage(SOCKET* client)
{
    char text[1024];
    int rc = recv(*client, text, buf, 0);
    return std::string(text);

}

void game(int id)
{
    while (rooms[id]->started != true)
        ;

    bool whiteTurn = true;

    sendMessage(rooms[id]->player1, "Start W");
    sendMessage(rooms[id]->player2, "Start B");

    while (true)
    {
        std::string recv;
        // tura player 1
        recv = receiveMassage(rooms[id]->player1);
        std::cout << recv << std::endl;
        sendMessage(rooms[id]->player2, recv);
        // tura player 2
        recv = receiveMassage(rooms[id]->player2);
        std::cout << recv << std::endl;
        sendMessage(rooms[id]->player1, recv);
    }

    return;
}

void lobby(SOCKET* player)
{
    std::cout << "Dołączył nowy gracz" << std::endl;
    

    std::string ans = receiveMassage(player);

    if (ans[0] == 'C')
    {
        std::cout << "Gracz chce utworzyć pokój" << std::endl;
        
        Room* room = new Room;
        room->player1 = player;
        room->player2 = nullptr;
        room->started = false;
        rooms.push_back(room);

        workers.push_back(new std::thread(game, g_id));
        g_id++;
    }
    else if (ans[0] == 'J')
    {
        ans.erase(ans.begin(), ans.begin() + 2);
        int tmp = strtol(ans.c_str(), NULL, 0);
        std::cout << "Gracz chce dołączyć do pokoju " << tmp << std::endl;

        rooms[tmp]->player2 = player;
        rooms[tmp]->started = true;
    }

    return;
}

int main()
{
    WSADATA wsaData;

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR)
        printf("Initialization error.\n");

    sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(4109);

    SOCKET mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mainSocket == INVALID_SOCKET)
    {
        printf("Error creating socket: %ld\n", WSAGetLastError());
        return 1;
    }

    if (bind(mainSocket, (SOCKADDR*)&saddr, sizeof(saddr)) == SOCKET_ERROR)
    {
        printf("bind() failed.\n");
        closesocket(mainSocket);
        return 1;
    }

    if (listen(mainSocket, 1) == SOCKET_ERROR)
        printf("Error listening on socket.\n");

    while (true)
    {
        SOCKET* acceptSocket = new SOCKET;
        *acceptSocket = SOCKET_ERROR;
        printf("Waiting for a client to connect...\n");

        while (*acceptSocket == SOCKET_ERROR)
        {
            *acceptSocket = accept(mainSocket, NULL, NULL);
        }

        printf("Client connected.\n");

        
        workers.push_back(new std::thread(lobby, acceptSocket));
    }

    //mainSocket = acceptSocket;


    int bytesSent;
    int bytesRecv = SOCKET_ERROR;
    char sendbuf[32] = "Server says hello!";
    char recvbuf[32] = "";

    bytesRecv = recv(mainSocket, recvbuf, 32, 0);
    printf("Bytes received: %ld\n", bytesRecv);
    printf("Received text: %s\n", recvbuf);

    bytesSent = send(mainSocket, sendbuf, strlen(sendbuf), 0);
    printf("Bytes sent: %ld\n", bytesSent);

    system("pause");
}