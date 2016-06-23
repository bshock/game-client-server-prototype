#ifndef GAMECONTAINER_H
#define GAMECONTAINER_H

#include <QUuid>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <mutex>
#include "../Common/playercommunicator.h"
#include "../Common/gamestate.h"
#include "../Common/playerdeletelock.h"

class GameContainer
{
    struct PlayerData {
        QUuid PlayerID;
        std::string PlayerLogin;
        GameState PlayerState;
    };

public:
    GameContainer();
    GameContainer(const GameContainer &rhs);
    GameContainer &operator=(const GameContainer &rhs);
    bool operator==(const GameContainer &rhs);
    virtual ~GameContainer();

    bool AddPlayer(const QUuid &playerID, const std::string &playerLogin);
    bool RemovePlayer(const QUuid &playerID);
    bool RemovePlayer(const std::string &playerLogin);
    bool HasPlayer(const std::string &playerLogin) const;
    bool IsPlayerActive(const std::string &playerLogin) const;
    bool IsPreviousGame(const std::string &playerLogin) const;
    bool IsAnyPlayerActive() const;
    bool AreBothPlayersActive() const;
    bool IsOpenGame() const;
    bool ResumeGame(const QUuid &playerID, const std::string &playerLogin);
    std::string GetOtherPlayerLogin(const std::string &playerLogin) const;
    std::string GetOtherPlayerLogin(int turn) const;
    std::string GetPlayerLogin(int turn) const;
    QUuid GetOtherPlayerID(const QUuid &playerID) const;
    QUuid GetPlayerID(const std::string &playerLogin) const;
    QUuid GetOtherPlayerID(int turn) const;
    QUuid GetPlayerID(int turn) const;
    GameState *GetPlayerState(int turn);
    GameState *GetPlayerState(const QUuid &playerID);
    GameState *GetPlayerState(const std::string &playerLogin);
    int GetPlayerTurn(const QUuid &playerID) const;
    int GetPlayerTurn(const std::string &playerLogin) const;
    int SwitchTurn();
    int CurrentTurn() const { return currentTurnPlayer; }

    QUuid GameID() const { return gameID; }
    std::chrono::system_clock::time_point GetCreationTimePoint() { return timeStamp; }
    std::time_t TimeStamp() const { return std::chrono::system_clock::to_time_t(timeStamp); }
private:
    std::vector<PlayerData> players;
    mutable std::mutex playersMutex;
    QUuid gameID;
    std::chrono::system_clock::time_point timeStamp;
    int currentTurnPlayer;
};

#endif // GAMECONTAINER_H
