#include "gamemanager.h"
#include <iostream>
#include <sstream>

GameManager::GameManager()
{
}

GameManager::~GameManager()
{
}

int GameManager::GetGameCount() const
{
    std::lock_guard<std::mutex> lock(gamesListMutex);
    return static_cast<int>(games.size());
}

QUuid GameManager::AddGame( const QUuid &playerID, const std::string &playerLogin) {
    std::lock_guard<std::mutex> lock(gamesListMutex);
    std::unique_ptr<GameContainer> game(new GameContainer());
    game->AddPlayer(playerID, playerLogin);
    QUuid gameID = game->GameID();
    games.insert(std::make_pair(gameID, std::move(game)));
    return gameID;
}

std::vector<std::string> GameManager::FindOpenGames(const std::string &playerLogin)
{
    std::lock_guard<std::mutex> lock(gamesListMutex);
    std::vector<std::string> result;
    for (auto &p:games){
        if (p.second->IsOpenGame() && !p.second->HasPlayer(playerLogin)) {
            std::string otherPlayer = p.second->GetOtherPlayerLogin(playerLogin);
            result.push_back(GetGameInfo(p.first, p.second->TimeStamp(), otherPlayer, GameStatus::OPEN));
        }
    }
    return result;
}

std::vector<std::string> GameManager::FindResumableGames(const std::string &playerLogin)
{
    std::lock_guard<std::mutex> lock(gamesListMutex);
    std::vector<std::string> result;
    for (auto &p:games){
        if (p.second->IsPreviousGame(playerLogin)) {
            std::string otherPlayer = p.second->GetOtherPlayerLogin(playerLogin);
            result.push_back(GetGameInfo(p.first, p.second->TimeStamp(), otherPlayer, GameStatus::RESUMABLE));
        }
    }
    return result;
}

bool GameManager::DeleteGame(const QUuid &gameID)
{
    bool result = false;
    usageDeletionLock.WriteLock();
    const std::unique_ptr<GameContainer> &gm = getGameRef(gameID);
    if (gm) {
        std::lock_guard<std::mutex> lock(gamesListMutex);
        if (!gm->IsAnyPlayerActive()) {
            std::cout << "Ready to delete GameContainer " << gm->GameID().toString().toUtf8().constData() << "\n";
            auto it = games.find(gameID);
            std::unique_ptr<GameContainer> xgm = std::move(it->second);
            games.erase(it);
            result = true;
        }
    }
    usageDeletionLock.WriteUnlock();
    return result;
}

void GameManager::CleanGameList()
{
    std::vector<QUuid> keys;
    if (GetGameCount()>0) {
        std::lock_guard<std::mutex> lock(gamesListMutex);
        for (auto &p:games) {
            keys.push_back(p.first);
        }
    }

    for (auto &k:keys) {
        if (DeleteGame(k)) {
            break;
        }
    }
}

time_t GameManager::GetGameTimeStamp(const QUuid &gameID)
{
    usageDeletionLock.ReadLock();
    time_t result;
    const std::unique_ptr<GameContainer> &gm = getGameRef(gameID);
    if (gm) {
        result = gm->TimeStamp();
    }
    usageDeletionLock.ReadUnlock();
    return result;
}

bool GameManager::AddPlayer(const QUuid &gameID, const QUuid &playerID, const std::string &playerLogin)
{
    usageDeletionLock.ReadLock();
    bool result = false;
    const std::unique_ptr<GameContainer> &gm = getGameRef(gameID);
    if (gm) {
        result = gm->AddPlayer(playerID, playerLogin);
    }
    usageDeletionLock.ReadUnlock();
    return result;
}

bool GameManager::RemovePlayer(const QUuid &gameID, const QUuid &playerID)
{
    usageDeletionLock.ReadLock();
    bool result = false;
    const std::unique_ptr<GameContainer> &gm = getGameRef(gameID);
    if (gm) {
        result = gm->RemovePlayer(playerID);
    }
    usageDeletionLock.ReadUnlock();
    return result;
}

void GameManager::RemoveFromAllGames(const std::string &playerLogin)
{
    usageDeletionLock.ReadLock();
    if (GetGameCount() > 0) {
        std::lock_guard<std::mutex> lock(gamesListMutex);
        for (auto &p:games) {
            if (p.second->HasPlayer(playerLogin)) {
                p.second->RemovePlayer(playerLogin);
            }
        }
    }
}

bool GameManager::ResumePlayer(const QUuid &gameID, const QUuid &playerID, const std::string &playerLogin)
{
    usageDeletionLock.ReadLock();
    bool result = false;
    const std::unique_ptr<GameContainer> &gm = getGameRef(gameID);
    if (gm) {
        result = gm->ResumeGame(playerID, playerLogin);
    }
    usageDeletionLock.ReadUnlock();
    return result;
}

QUuid GameManager::GetOtherPlayerID(const QUuid &gameID, const QUuid &playerID)
{
    usageDeletionLock.ReadLock();
    QUuid result = "";
    const std::unique_ptr<GameContainer> &gm = getGameRef(gameID);
    if (gm) {
        result = gm->GetOtherPlayerID(playerID);
    }
    usageDeletionLock.ReadUnlock();
    return result;
}

QUuid GameManager::GetOtherPlayerID(const QUuid &gameID, int turn)
{
    usageDeletionLock.ReadLock();
    QUuid result = "";
    const std::unique_ptr<GameContainer> &gm = getGameRef(gameID);
    if (gm) {
        result = gm->GetOtherPlayerID(turn);
    }
    usageDeletionLock.ReadUnlock();
    return result;
}

QUuid GameManager::GetPlayerID(const QUuid &gameID, int turn)
{
    usageDeletionLock.ReadLock();
    QUuid result = "";
    const std::unique_ptr<GameContainer> &gm = getGameRef(gameID);
    if (gm) {
        result = gm->GetPlayerID(turn);
    }
    usageDeletionLock.ReadUnlock();
    return result;
}

GameState *GameManager::GetPlayerState(const QUuid &gameID, const QUuid &playerID)
{
    usageDeletionLock.ReadLock();
    GameState *result = nullptr;
    const std::unique_ptr<GameContainer> &gm = getGameRef(gameID);
    if (gm) {
        result = gm->GetPlayerState(playerID);
    }
    usageDeletionLock.ReadUnlock();
    return result;
}

int GameManager::GetPlayerTurn(const QUuid &gameID, const QUuid &playerID)
{
    usageDeletionLock.ReadLock();
    int result = -1;
    const std::unique_ptr<GameContainer> &gm = getGameRef(gameID);
    if (gm) {
        result = gm->GetPlayerTurn(playerID);
    }
    usageDeletionLock.ReadUnlock();
    return result;
}

int GameManager::GetCurrentTurn(const QUuid &gameID)
{
    usageDeletionLock.ReadLock();
    int result = -1;
    const std::unique_ptr<GameContainer> &gm = getGameRef(gameID);
    if (gm) {
        result = gm->CurrentTurn();
    }
    usageDeletionLock.ReadUnlock();
    return result;
}

int GameManager::SwitchTurn(const QUuid &gameID)
{
    usageDeletionLock.ReadLock();
    int result = -1;
    const std::unique_ptr<GameContainer> &gm = getGameRef(gameID);
    if (gm) {
        result = gm->SwitchTurn();
    }
    usageDeletionLock.ReadUnlock();
    return result;
}

bool GameManager::HasPlayer(const QUuid &gameID, const std::string &playerLogin)
{
    usageDeletionLock.ReadLock();
    bool result = false;
    const std::unique_ptr<GameContainer> &gm = getGameRef(gameID);
    if (gm) {
        result = gm->HasPlayer(playerLogin);
    }
    usageDeletionLock.ReadUnlock();
    return result;
}

bool GameManager::AreBothPlayersActive(const QUuid &gameID)
{
    usageDeletionLock.ReadLock();
    bool result = false;
    const std::unique_ptr<GameContainer> &gm = getGameRef(gameID);
    if (gm) {
        result = gm->AreBothPlayersActive();
    }
    usageDeletionLock.ReadUnlock();
    return result;
}

const std::unique_ptr<GameContainer> &GameManager::getGameRef(const QUuid &gameID) const
{
    std::lock_guard<std::mutex> lock(gamesListMutex);
    auto it = games.find(gameID);
    return it->second;
}

std::string GameManager::GetGameInfo(const QUuid &gameID, const time_t &tt, const std::string &playerLogin, GameStatus state) {
    std::string stateStr;
    switch(state) {
    case GameStatus::OPEN:
        stateStr = PlayerCommunicator::STATE_OPEN;
        break;
    case GameStatus::RESUMABLE:
        stateStr = PlayerCommunicator::STATE_RESUMABLE;
        break;
    case GameStatus::WAITING:
        stateStr = PlayerCommunicator::STATE_WAITING;
        break;
    default:
        stateStr = PlayerCommunicator::STATE_UNKNOWN;
        break;
    }

    std::ostringstream oss;
    oss << gameID.toString().toUtf8().constData() << PlayerCommunicator::DELIMITER << ctime(&tt) << PlayerCommunicator::DELIMITER << playerLogin << PlayerCommunicator::DELIMITER << stateStr;

    return oss.str();
}

