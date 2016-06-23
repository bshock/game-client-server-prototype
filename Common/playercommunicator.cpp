#include "playercommunicator.h"
#include <QHostAddress>
#include <QCoreApplication>
#include <iostream>
#include <sstream>
#include "portablesleep.h"
#include "parsemessages.h"

//#define CODE_DEBUG
#define PROTOCOL_DEBUG

#define CONNECTION_TIMEOUT 10000

const char PlayerCommunicator::DELIMITER = '~';
const std::string PlayerCommunicator::MSG1_GAME = "GAME";
const std::string PlayerCommunicator::MSG1_PLAY = "PLAY";
const std::string PlayerCommunicator::MSG1_STATE = "STATE";
const std::string PlayerCommunicator::MSG1_MESSAGE = "MESSAGE";
const std::string PlayerCommunicator::MSG1_HANDSHAKE = "HANDSHAKE";
const std::string PlayerCommunicator::MSG1_LOGIN = "LOGIN";
const std::string PlayerCommunicator::MSG1_ERROR = "ERROR";

const std::string PlayerCommunicator::MSG2_START = "START";
const std::string PlayerCommunicator::MSG2_ACK = "ACK";
const std::string PlayerCommunicator::MSG2_ALL = "ALL";
const std::string PlayerCommunicator::MSG2_READY = "READY";
const std::string PlayerCommunicator::MSG2_QUIT = "QUIT";
const std::string PlayerCommunicator::MSG2_NEW = "NEW";
const std::string PlayerCommunicator::MSG2_RESUME = "RESUME";
const std::string PlayerCommunicator::MSG2_JOIN = "JOIN";
const std::string PlayerCommunicator::MSG2_DISCONNECTED = "DISCONNECTED";
const std::string PlayerCommunicator::MSG2_CONNECTIONREFUSED = "REFUSED";

const std::string PlayerCommunicator::STATE_OPEN = "OPEN";
const std::string PlayerCommunicator::STATE_RESUMABLE = "RESUMABLE";
const std::string PlayerCommunicator::STATE_WAITING = "WAITING";
const std::string PlayerCommunicator::STATE_UNKNOWN = "UNKNOWN";


PlayerCommunicator::PlayerCommunicator() : playerThread(nullptr),
    socket(nullptr), socketDescriptor(0), port(0),
    running(false), handshake(false), dead(true)
{
    playerID = QUuid::createUuid();
}

PlayerCommunicator::~PlayerCommunicator()
{
    std::cout << "PlayerCommunicator::dtor\n";
}

void PlayerCommunicator::Start(qintptr socketID)
{
    this->socketDescriptor = socketID;
    connectToThread();
}

void PlayerCommunicator::Start(const QHostAddress &address, int prt)
{
    this->hostAddress = address;
    this->port = prt;
    connectToThread();
}

void PlayerCommunicator::connectToThread()
{
    dead = false;

    playerThread = new QThread();

    connect(playerThread, SIGNAL(started()), this, SLOT(runCommunications()));
    connect(this, SIGNAL(finished()), playerThread, SLOT(quit()));
    connect(playerThread, SIGNAL(finished()), playerThread, SLOT(deleteLater()));
    connect(this, SIGNAL(relayData(QByteArray)), this, SLOT(sendData(QByteArray)));

    this->moveToThread(playerThread);
    playerThread->start();
}

void PlayerCommunicator::KillOnDisconnect()
{
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void PlayerCommunicator::runCommunications()
{
    socket = new QTcpSocket(this);
    if (this->socketDescriptor == 0)
    {
        //outgoing connection
        socket->connectToHost(hostAddress, port);
        handshake = true;
    }
    else if (!socket->setSocketDescriptor(this->socketDescriptor))
    {
        //incoming connection
        emit error(socket->error());
        return;
    }

    //TODO: add timeout for PlayerSocket outgoing connections

    //DirectConnection is multithreaded
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(this, SIGNAL(finished()), socket, SLOT(deleteLater()));

    std::cout << " connection established.\n";

    running = true;
    while (running)
    {
        if (handshake)
        {
            startHandshake();
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        PortableSleep::msleep(100);
    }

    callUserNotification(PlayerCommunicator::MSG1_ERROR, PlayerCommunicator::MSG2_CONNECTIONREFUSED, true); //calls finished signal
}

void PlayerCommunicator::callUserNotification(const std::string &first, const std::string &second, bool killOnEnd)
{
    std::lock_guard<std::mutex> lock(notificationMutex);
    if (dead) {
        return;
    }
    if (UserNotificationCallback2 != nullptr) {
        UserNotificationCallback2(first, second);
    }
    if (killOnEnd) {
        emit finished();
        dead = true;
    }
}

void PlayerCommunicator::disconnected()
{
    std::cout << " SocketContainer::disconnected\n";
    running = false;
}

void PlayerCommunicator::socketError(QAbstractSocket::SocketError e)
{
    switch(e)
    {
    case QAbstractSocket::RemoteHostClosedError:
        std::cout << " RemoteHostClosedError\n";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        std::cout << " ConnectionRefusedError\n";
        //Server side has refused connection, usually because there isn't a server at the specified ip/port.
        callUserNotification(PlayerCommunicator::MSG1_ERROR, PlayerCommunicator::MSG2_CONNECTIONREFUSED);
        //don't fall through to "running=false", which will shut down the object if KillOnDisconnect has been called.
        //  -- we need to return an error message instead.
        return;
    default:
        qDebug() << e;
        break;
    }

    //disconnected sockets don't always show up with the disconnected event or RemoteHostClosedError
    //  -- shut down on ALL errors.
    running = false;
}

void PlayerCommunicator::readyRead()
{
    QByteArray Data = socket->readAll();
    processReceivedData(Data);
}

void PlayerCommunicator::sendData(QByteArray data)
{
    if (running && socket->waitForConnected())
    {
        socket->write(data);
    }
}

void PlayerCommunicator::processReceivedData(QByteArray data)
{
    std::string temp = data.toStdString();
    parser.AddString(temp);
    while (parser.NewMessageAvailable())
    {
        std::pair<std::string, std::string> result = parser.GetKeyValuePair();
        readMessage(result);
    }
}

void PlayerCommunicator::readMessage(const std::pair<std::string, std::string> &msg)
{
    if (msg.first == PlayerCommunicator::MSG1_HANDSHAKE) {
        if (msg.second == PlayerCommunicator::MSG2_ACK) {
            if (handshake) {
                handshake = false;
                callUserNotification(PlayerCommunicator::MSG1_LOGIN, PlayerCommunicator::MSG2_READY);
            }
        }
        else if (msg.second == PlayerCommunicator::MSG2_START){
            sendAcknowledgeHandshake();
        }
    } else {
        callUserNotification(msg.first, msg.second);
    }
}

void PlayerCommunicator::send(const std::string &str)
{
    QString qstr(str.data());
    QByteArray data;
    data.insert(0, qstr);
    emit relayData(data);
}

void PlayerCommunicator::startHandshake()
{
    std::ostringstream oss;
    oss << ParseMessages::START << PlayerCommunicator::MSG1_HANDSHAKE << ParseMessages::EQUAL << PlayerCommunicator::MSG2_START << ParseMessages::END;
    send(oss.str());
}

void PlayerCommunicator::sendAcknowledgeHandshake()
{
    std::ostringstream oss;
    oss << ParseMessages::START << PlayerCommunicator::MSG1_HANDSHAKE << ParseMessages::EQUAL << PlayerCommunicator::MSG2_ACK << ParseMessages::END;
    send(oss.str());
}

void PlayerCommunicator::SendCommunications(Communications cmn, const std::string &data1, const std::string &data2)
{
    std::cout << "SendCommunications:cmn:" << cmn << ";data1:" << data1 << ";data2:" << data2 << "\n";
    std::ostringstream oss;
    switch(cmn) {
    case Communications::LOGIN:
        oss << ParseMessages::START << PlayerCommunicator::MSG1_LOGIN << ParseMessages::EQUAL << data1 << PlayerCommunicator::DELIMITER << data2 << ParseMessages::END;
        break;
    case Communications::GAMEINFO:
        oss << ParseMessages::START << PlayerCommunicator::MSG1_GAME << ParseMessages::EQUAL << data1 << ParseMessages::END;
        break;
    case Communications::ALLGAMES:
        oss << ParseMessages::START << PlayerCommunicator::MSG1_GAME << ParseMessages::EQUAL<< PlayerCommunicator::MSG2_ALL << ParseMessages::END;
        break;
    case Communications::READYTOPLAY:
        oss << ParseMessages::START << PlayerCommunicator::MSG1_PLAY << ParseMessages::EQUAL<< PlayerCommunicator::MSG2_READY << ParseMessages::END;
        break;
    case Communications::QUITPLAY:
        oss << ParseMessages::START << PlayerCommunicator::MSG1_PLAY << ParseMessages::EQUAL<< data1 << PlayerCommunicator::DELIMITER << PlayerCommunicator::MSG2_QUIT << ParseMessages::END;
        break;
    case Communications::PLAY:
        oss << ParseMessages::START << PlayerCommunicator::MSG1_PLAY << ParseMessages::EQUAL << data1 << ParseMessages::END;
        break;
    case Communications::STATE:
        oss << ParseMessages::START << PlayerCommunicator::MSG1_STATE << ParseMessages::EQUAL << data1 << ParseMessages::END;
        break;
    case Communications::MESSAGE:
        oss << ParseMessages::START << PlayerCommunicator::MSG1_MESSAGE << ParseMessages::EQUAL << data1 << ParseMessages::END;
        break;
    case Communications::ERRORS:
        oss << ParseMessages::START << PlayerCommunicator::MSG1_ERROR << ParseMessages::EQUAL << data1 << ParseMessages::END;
        break;
    default:
        //TODO: log this as an error
        return;
    }
    send(oss.str());
}

void PlayerCommunicator::SendLogin(const std::string &username, const std::string &password)
{
    std::ostringstream oss;
    oss << ParseMessages::START << PlayerCommunicator::MSG1_LOGIN << ParseMessages::EQUAL << username << PlayerCommunicator::DELIMITER << password << ParseMessages::END;
    send(oss.str());
}

void PlayerCommunicator::SendGameInfo(const std::string &gameInfo)
{
    std::ostringstream oss;
    oss << ParseMessages::START << PlayerCommunicator::MSG1_GAME << ParseMessages::EQUAL << gameInfo << ParseMessages::END;
    send(oss.str());
}

void PlayerCommunicator::SendAllGamesInfo()
{
    std::ostringstream oss;
    oss << ParseMessages::START << PlayerCommunicator::MSG1_GAME << ParseMessages::EQUAL<< PlayerCommunicator::MSG2_ALL << ParseMessages::END;
    send(oss.str());
}


void PlayerCommunicator::SendReadyToPlay()
{
    std::ostringstream oss;
    oss << ParseMessages::START << PlayerCommunicator::MSG1_PLAY << ParseMessages::EQUAL<< PlayerCommunicator::MSG2_READY << ParseMessages::END;
    send(oss.str());
}

void PlayerCommunicator::SendQuitPlay(const std::string &gameID)
{
    std::ostringstream oss;
    oss << ParseMessages::START << PlayerCommunicator::MSG1_PLAY << ParseMessages::EQUAL<< gameID << PlayerCommunicator::DELIMITER << "QUIT" << ParseMessages::END;
    send(oss.str());
}

void PlayerCommunicator::SendPlay(const std::string &gameToPlay)
{
    std::ostringstream oss;
    oss << ParseMessages::START << PlayerCommunicator::MSG1_PLAY << ParseMessages::EQUAL << gameToPlay << ParseMessages::END;
    send(oss.str());
}

void PlayerCommunicator::SendState(const std::string &state)
{
    std::ostringstream oss;
    oss << ParseMessages::START << PlayerCommunicator::MSG1_STATE << ParseMessages::EQUAL << state << ParseMessages::END;
    send(oss.str());
}

void PlayerCommunicator::SendMessages(const std::string &msg)
{
    std::ostringstream oss;
    oss << ParseMessages::START << PlayerCommunicator::MSG1_MESSAGE << ParseMessages::EQUAL << msg << ParseMessages::END;
    send(oss.str());
}

void PlayerCommunicator::SendError(const std::string &error)
{
    std::ostringstream oss;
    oss << ParseMessages::START << PlayerCommunicator::MSG1_ERROR << ParseMessages::EQUAL << error << ParseMessages::END;
    send(oss.str());
}


std::vector<std::string> PlayerCommunicator::tokenize(const std::string &line) {
   std::istringstream iss(line);
   std::vector<std::string> result;
   for (std::string token; getline(iss, token, DELIMITER);) {
       result.push_back(token);
   }
   return result;
}
