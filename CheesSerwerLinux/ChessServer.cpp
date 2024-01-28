#include <iostream>

#include <string>
#include <thread>
#include <stdlib.h>
#include <time.h>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <vector>
#include <algorithm>

#define buf 64 //z góry okoreślony bufor przyjmowanych danych

// struktura dla wątku gry która przechowuje oba sockety dopasowanych graczy
struct Room 
{ 
    int* player1;
    int* player2;
    bool started;

    int id;
};
std::vector<Room*> rooms;

// dynamiczna tablice przechowująca wszystkie wątki jakie zostaną utworzone podczas działania serwera
//'pokoje' czyli wątki z przypisanymi im połączonymi klientami są obsługiwane niezależnie
std::vector<std::thread*> workers;


// dynamiczna tablica z listą utworzonych pokoi
std::vector<int> avaibleRooms;
int g_id = 0;


void sendMessage(int* client, std::string msg) // funkcja do wysyłu danych
{
    std::string data = msg;
    msg.append("\n");
    send(*client, msg.c_str(), buf, 0);
}

std::string receiveMassage(int* client) // funkcja do odbioru danych
{
    std::string data = "";
    std::string garbage = "";

    //int rc = read(m_con, buf, sizeof(buf));

    bool EOM = false; // end of messege

    while (!EOM)
    {
        char tmp[buf];
        int rc = recv(*client, tmp, buf, 0);

        if(rc == -1)
            return "DISS";

        for (int i = 0; i < rc; i++)
        {
            if (tmp[i] == '\n')
            {
                EOM = true;

                for (int j = i; j < buf; j++)
                    garbage += tmp[j];

                break;
            }
            data += tmp[i];
        }

    }



    std::cout << "Received: " << data << std::endl;

    return data;

}

// Funkcnja / pokój rozpoczynające gre dwóch graczy
void game(Room room)
{
    short randTurn = rand() % 2; // losowy wybór kto będzie białymi pionkami a kto czarnymi

    if (randTurn == 0)
    {
        sendMessage(room.player1, "Start W");
        sendMessage(room.player2, "Start B");

        while (true)
        {
            std::string recv;

            // tura player 1
            recv = receiveMassage(room.player1);
            if (recv != "DISS") // wiadomość o rozłączeniu gracza
                sendMessage(room.player2, recv);
            else if (recv == "END") // wiadomość koniec gry od gracza
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
// funkcja / pokój w którym przetrzymywani są gracze czekający na drugą osobe lub przeydzielająca do pokoju (zależy od wyboru klienta)
void lobby(int* player)
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

    std::string ip;
    short port;


    std::cout << "Aby uruchomic serwer: " << std::endl;
    std::cout << "Podaj adres IP: ";
    std::cin >> ip;
    std::cout << "Podaj port: ";
    std::cin >> port;

    sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(ip.c_str());
    saddr.sin_port = htons(port);

    int mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


    if (bind(mainSocket, (struct sockaddr*)&saddr, sizeof(saddr)) == -1)
    {
        printf("bind() failed.\n");
        close(mainSocket);
        return 1;
    }

    if (listen(mainSocket, 1) == -1)
        printf("Error listening on socket.\n");

    while (true) // pętla do obsługi nowo podłączonych graczy
    {
        int* acceptSocket = new int;
        *acceptSocket = -1;
        printf("Waiting for a client to connect...\n");

        while (*acceptSocket == -1)
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
