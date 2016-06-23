#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <map>
#include <vector>
#include <string>
#include <QUuid>
#include "../Common/playerdeletelock.h"
#include "gamecontainer.h"

enum GameStatus {UNKNOWN, OPEN, RESUMABLE, WAITING};

class GameManager
{
public:
    GameManager();
    ~GameManager();

    int GetGameCount() const;
    QUuid AddGame(const QUuid &playerID, const std::string &playerLogin);
    std::vector<std::string> FindOpenGames(const std::string &playerLogin);
    std::vector<std::string> FindResumableGames(const std::string &playerLogin);
    bool DeleteGame(const QUuid &gameID);
    void CleanGameList();
    time_t GetGameTimeStamp(const QUuid &gameID);
    std::string GetGameInfo(const QUuid &gameID, const time_t &tt, const std::string &playerLogin, GameStatus state);

    bool AddPlayer(const QUuid &gameID, const QUuid &playerID, const std::string &playerLogin);
    bool RemovePlayer(const QUuid &gameID, const QUuid &playerID);
    void RemoveFromAllGames(const std::string &playerLogin);
    bool ResumePlayer(const QUuid &gameID, const QUuid &playerID, const std::string &playerLogin);  //previously "ResumeGame"
    QUuid GetOtherPlayerID(const QUuid &gameID, const QUuid &playerID);
    QUuid GetOtherPlayerID(const QUuid &gameID, int turn);
    QUuid GetPlayerID(const QUuid &gameID, int turn);
    GameState *GetPlayerState(const QUuid &gameID, const QUuid &playerID);
    int GetPlayerTurn(const QUuid &gameID, const QUuid &playerID);
    int GetCurrentTurn(const QUuid &gameID);
    int SwitchTurn(const QUuid &gameID);
    bool HasPlayer(const QUuid &gameID, const std::string &playerLogin);
    bool AreBothPlayersActive(const QUuid &gameID);
private:
    const std::unique_ptr<GameContainer> &getGameRef(const QUuid &gameID) const;
private:
    std::map<QUuid, std::unique_ptr<GameContainer>> games;
    mutable PlayerDeleteLock usageDeletionLock;
    mutable std::mutex gamesListMutex;
};

#endif // GAMEMANAGER_H


