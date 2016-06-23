#include "gamecontainer.h"
#include <algorithm>
#include <iostream>

GameContainer::GameContainer(): currentTurnPlayer(0)
{
    gameID = QUuid::createUuid();
    timeStamp = std::chrono::system_clock::now();
}

GameContainer::GameContainer(const GameContainer &rhs):
    players(rhs.players), gameID(rhs.gameID),
    timeStamp(rhs.timeStamp), currentTurnPlayer(rhs.currentTurnPlayer)
{
}

GameContainer &GameContainer::operator=(const GameContainer &rhs)
{
    if (&rhs == this) {
        return *this;
    }
    GameContainer lhs(rhs);
    std::swap(*this, lhs);
    return *this;
}

bool GameContainer::operator==(const GameContainer &rhs)
{
    return (gameID == rhs.gameID);
}

GameContainer::~GameContainer()
{

}

bool GameContainer::AddPlayer(const QUuid &playerID, const std::string &playerLogin)
{
    std::lock_guard<std::mutex> lock(playersMutex);
    if (players.size() < 2) {

        GameState ps;
        ps.SetGameID(gameID);
        ps.SetIsInitialData(true);
        int turn = (players.size() == 0)?0:1;   // first player in a game always gets the first turn (0)
        ps.SetPlayerNumber(turn);
        ps.SetIsInitialData(true);

        PlayerData pd { playerID, playerLogin, ps };
        players.push_back(pd);
        return true;
    }
    return false;
}

bool GameContainer::RemovePlayer(const QUuid &playerID)
{
    int turn = GetPlayerTurn(playerID);
    std::lock_guard<std::mutex> lock(playersMutex);
    if (turn > -1 && (int)players.size() > turn) {
        players[turn].PlayerID = "";
        return true;
    }
    return false;
}

bool GameContainer::RemovePlayer(const std::string &playerLogin) {
    int turn = GetPlayerTurn(playerLogin);
    std::lock_guard<std::mutex> lock(playersMutex);
    if (turn > -1 && (int)players.size() > turn) {
        players[turn].PlayerID = "";
        return true;
    }
    return false;
}

bool GameContainer::HasPlayer(const std::string &playerLogin) const
{
    std::lock_guard<std::mutex> lock(playersMutex);
    auto it = std::find_if(players.begin(), players.end(), [&playerLogin](const PlayerData &player) { return (player.PlayerLogin == playerLogin);});
    return (it != players.end());
}

bool GameContainer::IsPlayerActive(const std::string &playerLogin) const
{
    std::lock_guard<std::mutex> lock(playersMutex);
    auto it = std::find_if(players.begin(), players.end(), [&playerLogin](const PlayerData &player) { return (player.PlayerLogin == playerLogin && !player.PlayerID.isNull());});
    return (it != players.end());
}

bool GameContainer::IsPreviousGame(const std::string &playerLogin) const
{
    std::lock_guard<std::mutex> lock(playersMutex);
    auto it = std::find_if(players.begin(), players.end(), [&playerLogin](const PlayerData &player) { return (player.PlayerLogin == playerLogin && player.PlayerID.isNull());});
    return (it != players.end());
}

bool GameContainer::IsAnyPlayerActive() const
{
    std::lock_guard<std::mutex> lock(playersMutex);
    return ((players.size() > 0 && !players[0].PlayerID.isNull()) || (players.size() > 1 && !players[1].PlayerID.isNull()));
}

bool GameContainer::AreBothPlayersActive() const {
    std::lock_guard<std::mutex> lock(playersMutex);
    return (players.size() > 1 && !players[0].PlayerID.isNull() && !players[1].PlayerID.isNull());
}

bool GameContainer::ResumeGame(const QUuid &playerID, const std::string &playerLogin)
{
    std::lock_guard<std::mutex> lock(playersMutex);
    auto it = std::find_if(players.begin(), players.end(), [&playerLogin](PlayerData &player) { return (player.PlayerLogin == playerLogin);});
    if (it != players.end()) {
        it->PlayerID = playerID;
        return true;
    }
    return false;
}

bool GameContainer::IsOpenGame() const
{
    std::lock_guard<std::mutex> lock(playersMutex);
    return (players.size() < 2);
}

int GameContainer::GetPlayerTurn(const QUuid &playerID) const {
    std::lock_guard<std::mutex> lock(playersMutex);
    if (players.size() > 0 && players[0].PlayerID == playerID) {
        return 0;
    } else if (players.size() > 1 && players[1].PlayerID == playerID) {
        return 1;
    }
    return -1;
}

int GameContainer::GetPlayerTurn(const std::string &playerLogin) const {
    std::lock_guard<std::mutex> lock(playersMutex);
    if (players.size() > 0 && players[0].PlayerLogin == playerLogin) {
        return 0;
    } else if (players.size() > 1 && players[1].PlayerLogin == playerLogin) {
        return 1;
    }
    return -1;
}

std::string GameContainer::GetOtherPlayerLogin(const std::string &playerLogin) const {
    std::lock_guard<std::mutex> lock(playersMutex);
    if (players.size() == 2) {
        if (players[0].PlayerLogin == playerLogin) {
            return players[1].PlayerLogin;
        } else if (players[1].PlayerLogin == playerLogin) {
            return players[0].PlayerLogin;
        }
    }
    return "";
}

std::string GameContainer::GetOtherPlayerLogin(int turn) const {
    std::lock_guard<std::mutex> lock(playersMutex);
    if (players.size() == 2) {
        if (turn == 0) {
            return players[1].PlayerLogin;
        } else if (turn == 1) {
            return players[0].PlayerLogin;
        }
    }
    return "";
}

std::string GameContainer::GetPlayerLogin(int turn) const {
    std::lock_guard<std::mutex> lock(playersMutex);
    if (turn == 0 && players.size() > 0) {
        return players[0].PlayerLogin;
    } else if (turn == 1 && players.size() > 1) {
        return players[1].PlayerLogin;
    }
    return "";
}

QUuid GameContainer::GetOtherPlayerID(const QUuid &playerID) const {
    std::lock_guard<std::mutex> lock(playersMutex);
    if (players.size() == 2) {
        if (players[0].PlayerID == playerID) {
            return players[1].PlayerID;
        } else if (players[1].PlayerID == playerID) {
            return players[0].PlayerID;
        }
    }
    return "";
}

QUuid GameContainer::GetOtherPlayerID(int turn) const {
    std::lock_guard<std::mutex> lock(playersMutex);
    if (players.size() == 2) {
        if (turn == 0) {
            return players[1].PlayerID;
        } else if (turn == 1) {
            return players[0].PlayerID;
        }
    }
    return "";
}

QUuid GameContainer::GetPlayerID(const std::string &playerLogin) const {
    std::lock_guard<std::mutex> lock(playersMutex);
    if (players.size() > 0 && players[0].PlayerLogin == playerLogin) {
        return players[0].PlayerID;
    } else if (players.size() > 1 && players[1].PlayerLogin == playerLogin) {
        return players[1].PlayerID;
    }
    return "";
}

QUuid GameContainer::GetPlayerID(int turn) const {
    std::lock_guard<std::mutex> lock(playersMutex);
    if (turn == 0 && players.size() > 0) {
        return players[0].PlayerID;
    } else if (turn == 1 && players.size() > 1) {
        return players[1].PlayerID;
    }
    return "";
}

int GameContainer::SwitchTurn() {
    return ++currentTurnPlayer %= 2;
}

GameState *GameContainer::GetPlayerState(int turn) {
    std::lock_guard<std::mutex> lock(playersMutex);
    if (players.size() > 0 && turn==0) {
        return &(players[turn].PlayerState);
    } else if (players.size() > 1 && turn==1) {
        return &(players[turn].PlayerState);
    }
    return nullptr;
}

GameState *GameContainer::GetPlayerState(const QUuid &playerID) {
    int turn = GetPlayerTurn(playerID);
    if (turn > -1) {
        return GetPlayerState(turn);
    }
    return nullptr;
}

GameState *GameContainer::GetPlayerState(const std::string &playerLogin) {
    int turn = GetPlayerTurn(playerLogin);
    if (turn > -1) {
        return GetPlayerState(turn);
    }
    return nullptr;
}
