#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <string>
#include <thread>
#include <stdlib.h>
#include <time.h>       

#include <vector>
#include <algorithm>

#define buf 128

struct Room 
{ 
    SOCKET* player1; 
    SOCKET* player2; 
    bool started;

    int id;
};

std::vector<std::thread*> workers;
std::vector<Room*> rooms;
std::vector<int> avaibleRooms;
int g_id = 0;

void sendMessage(SOCKET* client, std::string msg)
{
    send(*client, msg.c_str(), buf, 0);
}

std::string receiveMassage(SOCKET* client)
{
    char text[buf];


    int rc = recv(*client, text, buf, 0);


    if (rc == -1)
        return "DISS";
    else
        return std::string(text);

}

void game(Room room)
{
    short randTurn = rand() % 2;

    if (randTurn == 0)
    {
        sendMessage(room.player1, "Start W");
        sendMessage(room.player2, "Start B");

        while (true)
        {
            std::string recv;

            // tura player 1
            recv = receiveMassage(room.player1);
            if (recv != "DISS")
                sendMessage(room.player2, recv);
            else if (recv == "END")
            {
                sendMessage(room.player2, recv);
                return;
            }
            else
                return;

            // tura player 2
            recv = receiveMassage(room.player2);
            if (recv != "DISS")
                sendMessage(room.player1, recv);
            else if (recv == "END")
            {
                sendMessage(room.player1, recv);
                return;
            }
            else
                return;
        }
    }
    else
    {
        sendMessage(room.player1, "Start B");
        sendMessage(room.player2, "Start W");

        while (true)
        {
            std::string recv;

            // tura player 2
            recv = receiveMassage(room.player2);
            std::cout << recv << std::endl;
            if (recv != "DISS")
                sendMessage(room.player1, recv);
            else if (recv == "END")
            {
                sendMessage(room.player1, recv);
                return;
            }
            else
                return;

            // tura player 1
            recv = receiveMassage(room.player1);
            std::cout << recv << std::endl;
            if (recv != "DISS")
                sendMessage(room.player2, recv);
            else if (recv == "END")
            {
                sendMessage(room.player2, recv);
                return;
            }
            else
                return;
        }
    }

    return;
}

void lobby(SOCKET* player)
{
    std::cout << "New player connected" << std::endl;

    while (true)
    {
        std::string ans = receiveMassage(player);
        int tmp = 0;

        if (ans[0] == 'C')
        {
            std::cout << "Created Room: " << g_id << std::endl;

            sendMessage(player, std::to_string(g_id));

            Room room;
            room.player1 = player;
            room.player2 = nullptr;
            room.started = false;
            room.id = g_id;

            avaibleRooms.push_back(g_id);
            rooms.push_back(&room);

            g_id++;

            while (room.player2 == nullptr);

            workers.push_back(new std::thread(game, room));

            return;
        }
        else if (ans[0] == 'J')
        {
            ans.erase(ans.begin(), ans.begin() + 2);
            tmp = strtol(ans.c_str(), NULL, 0);
            std::cout << "Player want to join room " << tmp << std::endl;

            for (int i = 0; i < avaibleRooms.size(); i++)
            {
                if (tmp == avaibleRooms[i])
                {
                    std::cout << "Player succsesfully connected to room: " << tmp << std::endl;


                    rooms[avaibleRooms[i]]->player2 = player;
                    rooms[avaibleRooms[i]]->started = true;
                    avaibleRooms.erase(std::remove(avaibleRooms.begin(), avaibleRooms.end(), avaibleRooms[i]), avaibleRooms.end());

                    return;
                }

            }
        }

        std::string errr = "";
        errr.append("Error: Room ");
        errr.append(std::to_string(tmp));
        errr.append(" is not aviable");
        sendMessage(player, errr);
    }

    return;
}

int main()
{
    srand(time(NULL));

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

    std::cin;
    std::cin;

    return 0;
}