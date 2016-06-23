#ifndef PLAYERMANAGER_H
#define PLAYERMANAGER_H

#include <QUuid>
#include <map>
#include <string>
#include <memory>
#include "../Common/playercommunicator.h"
#include "../Common/playerdeletelock.h"

class PlayerManager
{
public:
    PlayerManager();
    ~PlayerManager();

    int GetPlayerCount() const;
    QUuid AddPlayer();
    void SetPlayerNotifications(const QUuid &playerID, std::function<void(std::string first, std::string second)> callback);
    void StartPlayer(const QUuid &playerID, qintptr socketID);
    void SetPlayerLogin(const QUuid &playerID, const std::string &login);
    std::string GetPlayerLogin(const QUuid &playerID) const;
    void SendPlayerCommunications(const QUuid &playerID, PlayerCommunicator::Communications cmn, const std::string &data1 = "", const std::string &data2 = "");
    void SendToAllPlayersButCurrent(const std::string &msgType, const std::string &msg, const QUuid &playerID);
    bool DeletePlayer(const QUuid &playerID);
    void CleanPlayerList();

private:
    const std::unique_ptr<PlayerCommunicator> &getPlayerRef(const QUuid &playerID) const;
private:
    //Note: QObject-derived objects give compilation errors if you try to put them into containers by value
    std::map<QUuid, std::unique_ptr<PlayerCommunicator>> players;
    mutable PlayerDeleteLock playerDeleteLock;
    mutable std::mutex playersListMutex;
};

#endif // PLAYERMANAGER_H
