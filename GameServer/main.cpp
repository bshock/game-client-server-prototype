#include <QCoreApplication>
#include "gameserver.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::cout << "Listening port? (Default is 1234) " << "\n";
    int port = 0;
    std::cin >> port;
    if (port == 0) {
        port = 1234;
    }

    GameServer server;
    server.startServer(port);

    return a.exec();
}
