#include "playermanager.h"
#include <utility>
#include <iostream>

PlayerManager::PlayerManager()
{
}

PlayerManager::~PlayerManager()
{
}

int PlayerManager::GetPlayerCount() const
{
    std::lock_guard<std::mutex> lock(playersListMutex);
    return static_cast<int>(players.size());
}

QUuid PlayerManager::AddPlayer()
{
    std::lock_guard<std::mutex> lock(playersListMutex);
    std::unique_ptr<PlayerCommunicator> player(new PlayerCommunicator());
    QUuid playerID = player->GetID();
    players.insert(std::make_pair(playerID, std::move(player)));
    return playerID;
}

void PlayerManager::SetPlayerNotifications(const QUuid &playerID, std::function<void (std::string, std::string)> callback)
{
    playerDeleteLock.ReadLock();
    const std::unique_ptr<PlayerCommunicator> &ps = getPlayerRef(playerID);
    if (ps) {
        ps->SetUserNotificationCallback2(callback);
    }
    playerDeleteLock.ReadUnlock();
}

void PlayerManager::StartPlayer(const QUuid &playerID, qintptr socketID)
{
    playerDeleteLock.ReadLock();
    const std::unique_ptr<PlayerCommunicator> &ps = getPlayerRef(playerID);
    if (ps) {
        ps->Start(socketID);
    }
    playerDeleteLock.ReadUnlock();
}

void PlayerManager::SetPlayerLogin(const QUuid &playerID, const std::string &login)
{
    playerDeleteLock.ReadLock();
    const std::unique_ptr<PlayerCommunicator> &ps = getPlayerRef(playerID);
    if (ps) {
        ps->SetLogin(login);
    }
    playerDeleteLock.ReadUnlock();
}

std::string PlayerManager::GetPlayerLogin(const QUuid &playerID) const
{
    std::string result;
    playerDeleteLock.ReadLock();
    const std::unique_ptr<PlayerCommunicator> &ps = getPlayerRef(playerID);
    if (ps) {
        result = ps->GetLogin();
    }
    playerDeleteLock.ReadUnlock();
    return result;
}

void PlayerManager::SendPlayerCommunications(const QUuid &playerID, PlayerCommunicator::Communications cmn, const std::string &data1, const std::string &data2)
{
    playerDeleteLock.ReadLock();
    const std::unique_ptr<PlayerCommunicator> &ps = getPlayerRef(playerID);
    if (ps) {
        ps->SendCommunications(cmn, data1, data2);
    }
    playerDeleteLock.ReadUnlock();
}

void PlayerManager::SendToAllPlayersButCurrent(const std::string &msgType, const std::string &msg, const QUuid &playerID)
{
    playerDeleteLock.ReadLock();

    std::vector<QUuid> keys;
    if (GetPlayerCount()>0) {
        std::lock_guard<std::mutex> lock(playersListMutex);
        for (auto &p:players) {
            keys.push_back(p.first);
        }
    }

    for (auto &k:keys) {
        if (k != playerID) {
            if (msgType == PlayerCommunicator::MSG1_PLAY) {
                SendPlayerCommunications(k, PlayerCommunicator::Communications::PLAY, msg);
            } else if (msgType == PlayerCommunicator::MSG1_STATE) {
                //TODO: fix this
            } else if (msgType == PlayerCommunicator::MSG1_GAME) {
                SendPlayerCommunications(k, PlayerCommunicator::Communications::GAMEINFO, msg);
            } else if (msgType == PlayerCommunicator::MSG1_MESSAGE) {
                SendPlayerCommunications(k, PlayerCommunicator::Communications::MESSAGE, msg);
            } else if (msgType == PlayerCommunicator::MSG1_ERROR) {
                SendPlayerCommunications(k, PlayerCommunicator::Communications::ERRORS, msg);
            }
        }
    }
    playerDeleteLock.ReadUnlock();
}

const std::unique_ptr<PlayerCommunicator> &PlayerManager::getPlayerRef(const QUuid &playerID) const
{
    std::lock_guard<std::mutex> lock(playersListMutex);
    auto it = players.find(playerID);
    return it->second;
}

//TODO: Check to see if connected or previously connected PlayerCommunicator objects blow up when unique_ptr drops them.
bool PlayerManager::DeletePlayer(const QUuid &playerID)
{
    bool result = false;
    playerDeleteLock.WriteLock();
    const std::unique_ptr<PlayerCommunicator> &ps = getPlayerRef(playerID);
    if (ps) {
        if (ps->IsDead()) {
            std::lock_guard<std::mutex> lock(playersListMutex);
            //ps->GetID()... to string
            std::cout << "Ready to delete PlayerCommunicator\n";

            auto it = players.find(playerID);
            std::unique_ptr<PlayerCommunicator> xps = std::move(it->second);
            players.erase(it);

            result = true;
        }
    }
    playerDeleteLock.WriteUnlock();
    return result;
}

//only cleans out one dead player key value pair per call
void PlayerManager::CleanPlayerList()
{
    std::vector<QUuid> keys;
    if (GetPlayerCount()>0) {
        std::lock_guard<std::mutex> lock(playersListMutex);
        for (auto &p:players) {
            keys.push_back(p.first);
        }
    }

    for (auto &k:keys) {
        if (DeletePlayer(k)) {
            break;
        }
    }
}
