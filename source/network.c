#include "network.h"
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

void networkInit(const SocketInitConfig* conf) 
{
    socketInitialize(conf);
    nxlinkStdio();
    avformat_network_init();
}

void networkDestroy() 
{
    avformat_network_deinit();
    socketExit();
}

JoyConSocket* createJoyConSocket()
{
    JoyConSocket* socket = (JoyConSocket*)malloc(sizeof(JoyConSocket));
    socket->sock = -1;
    socket->lissock = -1;
    return socket;
}

void freeJoyConSocket(JoyConSocket* connection)
{
    free(connection);
}


int connectJoyConSocket(JoyConSocket* connection, int port)
{
    if (connection->lissock == -1)
    {
        if (connection->sock != -1)
        {
            close(connection->sock);
            connection->sock = -1;
        }

        struct sockaddr_in server;

        connection->lissock = socket(AF_INET, SOCK_STREAM, 0);
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(port);

        if (bind(connection->lissock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            close(connection->lissock);
            connection->lissock = -1;
            return 0;
        }

        listen(connection->lissock, 1);
    }

    if (connection->sock == -1)
    {
        // TODO: We might want to be able to not block if no client is connected
        struct sockaddr_in client;
        int c = sizeof(struct sockaddr_in);
        connection->sock = accept(connection->lissock, (struct sockaddr *)&client, (socklen_t *)&c);
        if(connection->sock < 0) {
            close(connection->lissock);
            connection->sock = -1;
            connection->lissock = -1;
            return 0;
        }
        printf("Got gamepad-connection\n");
    }
    return 1;
}

void sendJoyConInput(JoyConSocket* connection, const JoyPkg* pkg)
{
    if (send(connection->sock, pkg, sizeof(JoyPkg), 0) != sizeof(JoyPkg))
    {
        connection->sock = -1;
    }
}
