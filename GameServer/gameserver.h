#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTimer>
#include <QUuid>
#include <vector>
#include <string>
#include <map>
#include <mutex>
#include <memory>
#include "gamecontainer.h"
#include "../Common/playercommunicator.h"
#include "playermanager.h"
#include "gamemanager.h"

class GameServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit GameServer(QObject *parent = 0);
    void startServer(int port);

signals:

private slots:
    void cleanPlayerList();
    void cleanGameList();
protected:
    void incomingConnection(qintptr socketDescriptor);
private:
    bool checkLogin(const std::string &login);

    void sendToOtherPlayer(const std::string &msgType, const std::string &msg, const QUuid &gameID, const QUuid &playerID);
    void sendToOtherPlayer(const std::string &msgType, const std::string &msg, GameContainer *game, const QUuid &playerID);
    void sendToBothPlayers(const std::string &msgType, const std::string &msg, const QUuid &gameID, const QUuid &playerID);
    void sendToBothPlayers(const std::string &msgType, const std::string &msg, GameContainer *game, const QUuid &playerID);
    void sendGameStateToOtherPlayer(const QUuid &gameID, const QUuid &playerID);
    void sendGameStateToSelf(const QUuid &gameID, const QUuid &playerID, bool initial = false);

private:
    PlayerManager playerManager;
    GameManager gameManager;
    QTimer *playerListCleanTimer;
    QTimer *gameListCleanTimer;

    static const std::string MSG_LEFT_GAME;
    static const std::string MSG_OTHER_LEFT_GAME;
    static const std::string MSG_STARTED_NEW_GAME;
    static const std::string MSG_WAITING_FOR_OTHER;
    static const std::string MSG_NEW_GAME_AVAILABLE;
    static const std::string MSG_ALREADY_IN_GAME;
    static const std::string MSG_CANNOT_JOIN_GAME;
    static const std::string MSG_2_PLAYERS;
    static const std::string MSG_STILL_WAITING;
    static const std::string MSG_OTHER_PLAYERS_TURN;
    static const std::string MSG_OTHER_PLAYER_RETURNED;
    static const std::string MSG_YOU_HAVE_RETURNED;
    static const std::string MSG_YOUR_TURN;
    static const std::string MSG_CANNOT_LOGIN;

    static const std::string ERR_CANNOT_JOIN_GAME2;
    static const std::string ERR_JOIN_GAME_NOT_FOUND;
    static const std::string ERR_INCORRECT_JOIN_COMMAND;
    static const std::string ERR_COULD_NOT_BE_RESUMED;
    static const std::string ERR_RESUME_GAME_NOT_FOUND;
    static const std::string ERR_INCORRECT_RESUME_COMMAND;
    static const std::string ERR_OTHER_NOT_FOUND;
    static const std::string ERR_GAME_NOT_FOUND;
    static const std::string ERR_INCORRECT_USERNAMEPASSWORD;

    static const int PLAYERLIST_CLEANTIMER_PERIOD;
    static const int GAMELIST_CLEANTIMER_PERIOD;
};

#endif // SOCKETSERVER_H
