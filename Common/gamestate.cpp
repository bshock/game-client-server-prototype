#include "gamestate.h"

GameState::GameState():isInitialData(false), thisSideTurn(false) {
}

GameState::GameState(const GameState &rhs):
    isInitialData(rhs.isInitialData), thisSideTurn(rhs.thisSideTurn)
{
}

GameState &GameState::operator=(const GameState &rhs)
{
    if (&rhs == this) {
        return *this;
    }
    GameState lhs(rhs);
    std::swap(*this, lhs);
    return *this;
}

GameState::~GameState()
{
}

void GameState::read(const QJsonObject &json)
{
    std::lock_guard<std::mutex> lock(readWriteMutex);
    //TODO: read properties
     isInitialData = json["initial"].toBool();
     thisSideTurn = json["turn"].toBool();
     playerNumber = json["player"].toInt();
     QString gameIDStr = json["gameID"].toString();
     gameID = QUuid(gameIDStr);
}

void GameState::write(QJsonObject &json)
{
    std::lock_guard<std::mutex> lock(readWriteMutex);
    //TODO: write properties
    json["initial"] = isInitialData;
    json["turn"] = thisSideTurn;
    json["player"] = playerNumber;
    json["gameID"] = gameID.toString();
}
