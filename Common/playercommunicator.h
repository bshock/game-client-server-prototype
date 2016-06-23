#ifndef PLAYERSOCKET_H
#define PLAYERSOCKET_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>
#include <QUuid>
#include <string>
#include <utility>
#include <memory>
#include <functional>
#include <mutex>
#include "parsemessages.h"

class PlayerCommunicator : public QObject
{
    Q_OBJECT

public:
        enum Communications {LOGIN, GAMEINFO, ALLGAMES, READYTOPLAY, QUITPLAY, PLAY, STATE, MESSAGE, ERRORS};

        static const char DELIMITER;
        static const std::string MSG1_GAME;
        static const std::string MSG1_PLAY;
        static const std::string MSG1_STATE;
        static const std::string MSG1_MESSAGE;
        static const std::string MSG1_HANDSHAKE;
        static const std::string MSG1_LOGIN;
        static const std::string MSG1_ERROR;

        static const std::string MSG2_START;
        static const std::string MSG2_ACK;
        static const std::string MSG2_ALL;
        static const std::string MSG2_READY;
        static const std::string MSG2_QUIT;
        static const std::string MSG2_NEW;
        static const std::string MSG2_RESUME;
        static const std::string MSG2_JOIN;
        static const std::string MSG2_DISCONNECTED;
        static const std::string MSG2_CONNECTIONREFUSED;

        static const std::string STATE_OPEN;
        static const std::string STATE_RESUMABLE;
        static const std::string STATE_WAITING;
        static const std::string STATE_UNKNOWN;

    static std::vector<std::string> tokenize(const std::string &line);

public:
    //don't pass on a parent object to QObject because objects with a parent can't be moved onto a thread
    //  -- socket, workerThread, and this (if KillOnDisconnect set) are all deleted by a connection with
    //  the finished() signal
    explicit PlayerCommunicator();
    ~PlayerCommunicator();

    void Start(qintptr socketID); //for use with QTcpServer
    void Start(const QHostAddress &address, int prt);   //for use as client

    void SendCommunications(Communications cmn, const std::string &data1 = "", const std::string &data2 = "");

    //Delete this object when it disconnects -- only use when creating on heap!
    void KillOnDisconnect();
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //To use SetUserNotificationCallback outside of objects on in lambdas, just use the function name as the param.
    //To use SetUserNotificationCallback inside objects:
    //  using namespace std::placeholders; // for "_1"
    //  socketContainer->SetUserNotificationCallback(std::bind(&CALLING_CLASS::CALLBACK_FUNCTION_IN_CLASS, this, _1));
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    QUuid GetID() const { return playerID; }
    std::string GetLogin() const { return playerLogin; }
    void SetLogin(const std::string & identification) { playerLogin = identification; }
    //-- used for all notifications to calling object -- pinrequested, pin received, etc.
    void SetUserNotificationCallback(std::function<void(const std::string &first)> callback) { UserNotificationCallback = callback; }
    void SetUserNotificationCallback2(std::function<void(std::string first, std::string second)> callback) { UserNotificationCallback2 = callback; }
    //bool IdMatches(const std::string &idToMatch) { return (id == idToMatch); }

    void Shutdown() { running = false; }
    bool IsDead() { return dead; }

signals:
    void error(QTcpSocket::SocketError socketError);
    void finished();
    void relayData(QByteArray data);

private slots:
    void readyRead();
    void disconnected();
    void socketError(QAbstractSocket::SocketError e);
    void sendData(QByteArray data);
    void runCommunications();

private:
    void connectToThread();
    void processReceivedData(QByteArray data);
    void readMessage(const std::pair<std::string, std::string> &msg);
    void send(const std::string &str);
    void relayToCallback(const std::pair<std::string, std::string> &msg);

private:
    void startHandshake();
    void sendAcknowledgeHandshake();
    void callUserNotification(const std::string &first, const std::string &second, bool killOnEnd = false);

private:
    //deprecated
    void SendLogin(const std::string &username, const std::string &password);
    void SendGameInfo(const std::string &gameInfo);
    void SendAllGamesInfo();
    void SendReadyToPlay();
    void SendQuitPlay(const std::string &gameID);
    void SendPlay(const std::string &gameToPlay);
    void SendState(const std::string &state);
    void SendMessages(const std::string &msg);
    void SendError(const std::string &error);

private:
    QThread *playerThread;
    QTcpSocket *socket;

    qintptr socketDescriptor;
    QHostAddress hostAddress;
    int port;

    QUuid playerID;
    std::string playerLogin;

    volatile bool running;
    volatile bool handshake;
    volatile bool dead;
    std::mutex notificationMutex;
    ParseMessages parser;
    std::function<void(const std::string &first)> UserNotificationCallback;
    std::function<void(const std::string &first, const std::string &second)> UserNotificationCallback2;
};

#endif // PLAYERSOCKET_H
