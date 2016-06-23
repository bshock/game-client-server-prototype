#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <QJsonObject>
#include <QUuid>
#include <QString>
#include <mutex>

class GameState
{
public:
    GameState();
    GameState(const GameState &rhs);
    GameState &operator=(const GameState &rhs);
    virtual ~GameState();

    virtual void read(const QJsonObject &json);
    virtual void write(QJsonObject &json);

    //Indicates this is the first information sent to the client about that client's gamestate.
    //  -- IsInitialData must never be set true for receiving an opponent's gamestate.
    //  e.g. client receives IsInitialData = true and ThisSideTurn = 0. When he receives gamestate with
    //      ThisSideTurn = 1 (the opponent's gamestate), IsInitialData must be false.
    //TODO: fix this so that there's less chance of an error.
    void SetIsInitialData(bool i) {isInitialData = i;}
    bool IsInitialData() const {return isInitialData;}

    void SetThisSideTurn(bool t) {thisSideTurn = t; }
    bool ThisSideTurn() const {return thisSideTurn; }

    void SetPlayerNumber(int p) {playerNumber = p; }
    int PlayerNumber() const { return playerNumber; }

    void SetGameID(const QUuid &g) {gameID = g; }
    QUuid GameID() const { return gameID; }

private:
    bool isInitialData;
    bool thisSideTurn;
    int playerNumber;
    QUuid gameID;
    std::mutex readWriteMutex;
};

#endif // GAMESTATE_H
