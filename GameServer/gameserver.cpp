#include "gameserver.h"
#include "../Common/parsesingle.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <QJsonDocument>
#include "../Common/gamestate.h"

const std::string GameServer::MSG_LEFT_GAME = ">You have just left the game.";
const std::string GameServer::MSG_OTHER_LEFT_GAME = ">The other player has left the game.";
const std::string GameServer::MSG_STARTED_NEW_GAME = ">Player has started a new game.";
const std::string GameServer::MSG_WAITING_FOR_OTHER = ">Waiting for second player to join.";
const std::string GameServer::MSG_NEW_GAME_AVAILABLE = ">New game session available.";
const std::string GameServer::MSG_ALREADY_IN_GAME = ">You are already in this game.";
const std::string GameServer::MSG_CANNOT_JOIN_GAME = ">You cannot join this game.";
const std::string GameServer::MSG_2_PLAYERS = ">Two players have now joined this game session.\n>Play can now begin.";
const std::string GameServer::MSG_STILL_WAITING = ">Still waiting for other player.";
const std::string GameServer::MSG_OTHER_PLAYERS_TURN = ">Other player's turn.\n>Please wait for the other side to move.";
const std::string GameServer::MSG_OTHER_PLAYER_RETURNED = ">Other player has returned to this game.";
const std::string GameServer::MSG_YOU_HAVE_RETURNED = ">You have returned to this game.";
const std::string GameServer::MSG_YOUR_TURN = ">Your turn. Please make your move.";
const std::string GameServer::MSG_CANNOT_LOGIN = ">The server has rejected your login.";

const std::string GameServer::ERR_CANNOT_JOIN_GAME2 = "GAME COULD NOT BE JOINED:";
const std::string GameServer::ERR_JOIN_GAME_NOT_FOUND = "GAME TO JOIN NOT FOUND:";
const std::string GameServer::ERR_INCORRECT_JOIN_COMMAND = "INCORRECT JOIN COMMAND:";
const std::string GameServer::ERR_COULD_NOT_BE_RESUMED = "GAME COULD NOT BE RESUMED:";
const std::string GameServer::ERR_RESUME_GAME_NOT_FOUND = "GAME TO RESUME NOT FOUND:";
const std::string GameServer::ERR_INCORRECT_RESUME_COMMAND = "INCORRECT RESUME COMMAND:";
const std::string GameServer::ERR_OTHER_NOT_FOUND = "OTHER PLAYER NOT FOUND:";
const std::string GameServer::ERR_GAME_NOT_FOUND = "GAME NOT FOUND.";
const std::string GameServer::ERR_INCORRECT_USERNAMEPASSWORD = "INCORRECT USERNAME AND/OR PASSWORD.";

const int GameServer::PLAYERLIST_CLEANTIMER_PERIOD = 300000; //in milliseconds, about 5 minutes
const int GameServer::GAMELIST_CLEANTIMER_PERIOD = 300000;

GameServer::GameServer(QObject *parent) : QTcpServer(parent)
{
}

void GameServer::startServer(int port)
{
    if (!this->listen(QHostAddress::Any, port))
    {
        std::cout << " could not start server (Is port " << port << "in use by another application?)\n";
    }
    else
    {
        std::cout << "listening on port " << port << "...\n";
    }

    playerListCleanTimer = new QTimer(this);
    connect(playerListCleanTimer, SIGNAL(timeout()), this, SLOT(cleanPlayerList()));
    playerListCleanTimer->start(PLAYERLIST_CLEANTIMER_PERIOD);

    gameListCleanTimer = new QTimer(this);
    connect(gameListCleanTimer, SIGNAL(timeout()), this, SLOT(cleanGameList()));
    gameListCleanTimer->start(GAMELIST_CLEANTIMER_PERIOD);

}

void GameServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << " connecting to socket " << socketDescriptor;

    auto playerID = playerManager.AddPlayer();
    playerManager.SetPlayerNotifications(playerID,
        [this, playerID](const std::string &first, const std::string &second){
        auto tokens = PlayerCommunicator::tokenize(second);
        if (first == PlayerCommunicator::MSG1_LOGIN) {
            if (this->checkLogin(second)) {
                this->playerManager.SetPlayerLogin(playerID, second);
                auto gameList1 = this->gameManager.FindResumableGames(second);
                for(auto game:gameList1)
                {
                    this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::GAMEINFO, game);
                }
                auto gameList2 = this->gameManager.FindOpenGames(second);
                for(auto game:gameList2)
                {
                    this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::GAMEINFO, game);
                }
                this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::READYTOPLAY);
            } else {
                this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::MESSAGE, MSG_CANNOT_LOGIN);
                this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::ERRORS, ERR_INCORRECT_USERNAMEPASSWORD);
            }
        }
        else if (first == PlayerCommunicator::MSG1_PLAY) {
            if (second == PlayerCommunicator::MSG2_READY) {
                if (tokens.size() > 1) {
                    QUuid gameID(tokens[0].data());
                    this->sendToOtherPlayer(PlayerCommunicator::MSG1_PLAY, tokens[1], gameID, playerID );
                }
            } else if (tokens.size() > 1 && tokens[1] == PlayerCommunicator::MSG2_QUIT) {
                QUuid gameID(tokens[0].data());
                    this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::MESSAGE, MSG_LEFT_GAME);
                    auto otherPlayerID = gameManager.GetOtherPlayerID(gameID, playerID);
                    this->playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::MESSAGE, MSG_OTHER_LEFT_GAME);
                    gameManager.RemovePlayer(gameID, playerID);
            } else if (second == PlayerCommunicator::MSG2_DISCONNECTED) {
                std::string playerLogin = this->playerManager.GetPlayerLogin(playerID);
                this->gameManager.RemoveFromAllGames(playerLogin);
            } else {
                //Expect:
                    //GUID,JOIN
                    //GUID,RESUME
                    //NEW
                if (second==PlayerCommunicator::MSG2_NEW) {
                    std::string playerLogin = this->playerManager.GetPlayerLogin(playerID);
                    auto gameID = gameManager.AddGame(playerID, playerLogin);
                    auto state = gameManager.GetPlayerState(gameID, playerID);
                    state->SetIsInitialData(true);
                    state->SetGameID(gameID);
                    int playerTurn = gameManager.GetPlayerTurn(gameID, playerID);
                    state->SetPlayerNumber(playerTurn);
                    state->SetThisSideTurn((gameManager.GetCurrentTurn(gameID) == playerTurn));

                    sendGameStateToSelf(gameID, playerID, true);

                    this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::MESSAGE, MSG_STARTED_NEW_GAME);
                    this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::MESSAGE, MSG_WAITING_FOR_OTHER);
                    this->playerManager.SendToAllPlayersButCurrent(PlayerCommunicator::MSG1_MESSAGE, MSG_NEW_GAME_AVAILABLE, playerID);
                    auto timestamp = this->gameManager.GetGameTimeStamp(gameID);
                    std::string msg = this->gameManager.GetGameInfo(gameID, timestamp, playerLogin, GameStatus::OPEN);
                    this->playerManager.SendToAllPlayersButCurrent(PlayerCommunicator::MSG1_GAME, msg, playerID);
                } else if (second.find(PlayerCommunicator::MSG2_JOIN) != std::string::npos) {
                    if (tokens.size() > 0) {
                        QUuid gameID(tokens[0].data());

                            std::string playerLogin = this->playerManager.GetPlayerLogin(playerID);
                            if (this->gameManager.HasPlayer(gameID, playerLogin)) {
                                this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::MESSAGE, MSG_ALREADY_IN_GAME);
                                return;
                            }
                            if (!this->gameManager.AddPlayer(gameID, playerID, playerLogin)) {
                                this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::MESSAGE, MSG_CANNOT_JOIN_GAME);
                                this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::ERRORS, ERR_CANNOT_JOIN_GAME2 + tokens[0]);
                                return;
                            }

                            auto state = this->gameManager.GetPlayerState(gameID, playerID);
                            state->SetIsInitialData(true);
                            state->SetGameID(gameID);
                            int playerTurn = this->gameManager.GetPlayerTurn(gameID, playerID);
                            state->SetPlayerNumber(playerTurn);
                            state->SetThisSideTurn((this->gameManager.GetCurrentTurn(gameID) == playerTurn));
                            sendGameStateToSelf(gameID, playerID, true);

                            std::string msg;
                            if (this->gameManager.AreBothPlayersActive(gameID)) {
                                msg = MSG_2_PLAYERS;
                            } else {
                                msg = MSG_STILL_WAITING;
                            }
                            this->sendToBothPlayers(PlayerCommunicator::MSG1_MESSAGE, msg, gameID, playerID);
                            if (this->gameManager.AreBothPlayersActive(gameID)) {
                                auto otherPlayerID = this->gameManager.GetOtherPlayerID(gameID, playerID);

                                sendGameStateToOtherPlayer(gameID, playerID);
                                sendGameStateToOtherPlayer(gameID, otherPlayerID);
                                this->playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::MESSAGE, MSG_YOUR_TURN);
                                this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::MESSAGE, MSG_OTHER_PLAYERS_TURN);

                        } else {
                            this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::ERRORS, ERR_JOIN_GAME_NOT_FOUND + tokens[0]);
                        }
                    } else {
                        this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::ERRORS, ERR_INCORRECT_JOIN_COMMAND + second);
                    }
                } else if (second.find(PlayerCommunicator::MSG2_RESUME) != std::string::npos) {
                    if (tokens.size() > 0) {
                        QUuid gameID(tokens[0].data());

                        std::string playerLogin = this->playerManager.GetPlayerLogin(playerID);
                        if (!this->gameManager.ResumePlayer(gameID, playerID, playerLogin)) {
                            this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::ERRORS, ERR_COULD_NOT_BE_RESUMED + tokens[0]);
                            return;
                        }
                        auto state = this->gameManager.GetPlayerState(gameID, playerID);
                        sendGameStateToSelf(gameID, playerID, true);
                        if (this->gameManager.AreBothPlayersActive(gameID)) {
                            auto otherPlayerID = this->gameManager.GetOtherPlayerID(gameID, playerID);
                            sendGameStateToOtherPlayer(gameID, playerID);
                            sendGameStateToOtherPlayer(gameID, otherPlayerID);
                            this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::MESSAGE, MSG_YOU_HAVE_RETURNED);
                            this->playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::MESSAGE, MSG_OTHER_PLAYER_RETURNED);

                            if (state->ThisSideTurn()) {
                                this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::MESSAGE, MSG_YOUR_TURN);
                                this->playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::MESSAGE, MSG_OTHER_PLAYERS_TURN);
                            } else {
                                this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::MESSAGE, MSG_OTHER_PLAYERS_TURN);
                                this->playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::MESSAGE, MSG_YOUR_TURN);
                            }
                        } else {
                            this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::MESSAGE, MSG_YOU_HAVE_RETURNED);
                            this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::MESSAGE, MSG_STILL_WAITING);
                        }
                    } else {
                        this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::ERRORS, ERR_INCORRECT_RESUME_COMMAND + second);
                    }
                }
            }
        } else if (first == PlayerCommunicator::MSG1_GAME) {
            if (second==PlayerCommunicator::MSG2_ALL) {
                std::string playerLogin = this->playerManager.GetPlayerLogin(playerID);
                auto gameList1 = this->gameManager.FindResumableGames(playerLogin);
                for(auto game:gameList1)
                {
                    this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::GAMEINFO, game);
                }
                auto gameList2 = this->gameManager.FindOpenGames(playerLogin);
                for(auto game:gameList2)
                {
                    this->playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::GAMEINFO, game);
                }
            }
        } else if (first == PlayerCommunicator::MSG1_STATE) {
            QByteArray byteArray(second.c_str(), static_cast<int>(second.length()));
            QJsonDocument stateDoc(QJsonDocument::fromJson(byteArray));
            GameState state;
            state.read(stateDoc.object());

            QUuid gameID = state.GameID();
            //if a player sends in a gamestate saying it's his turn and he's no longer active, switch the turn over to the other player
            //  and update both players and the server on this change of state
            int currentTurn = this->gameManager.GetCurrentTurn(gameID);
            if (state.PlayerNumber() == currentTurn && state.ThisSideTurn() == false) {
                auto currentPlayerID = this->gameManager.GetPlayerID(gameID, currentTurn);
                this->playerManager.SendPlayerCommunications(currentPlayerID, PlayerCommunicator::Communications::STATE, second);
                auto pstate = this->gameManager.GetPlayerState(gameID, playerID);
                pstate->read(stateDoc.object()); //copy the state into the game record of that player's state

                auto otherPlayerID = this->gameManager.GetOtherPlayerID(gameID, currentTurn);
                this->playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::STATE, second);

                int newTurn = this->gameManager.SwitchTurn(gameID);
                GameState state2;
                state2.SetGameID(state.GameID());
                state2.SetPlayerNumber(newTurn);
                state2.SetThisSideTurn(true);
                QJsonObject object;
                state2.write(object);
                QJsonDocument  stateDoc(object);
                QByteArray byteArray = stateDoc.toJson();
                std::string stateStr(byteArray.constData(), byteArray.length());

                this->playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::STATE, stateStr);
                GameState *pstate2 = this->gameManager.GetPlayerState(gameID, otherPlayerID);
                pstate2->read(object);
                this->playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::MESSAGE, MSG_YOUR_TURN);

                this->playerManager.SendPlayerCommunications(currentPlayerID, PlayerCommunicator::Communications::STATE, stateStr);
                this->playerManager.SendPlayerCommunications(currentPlayerID, PlayerCommunicator::Communications::MESSAGE, MSG_OTHER_PLAYERS_TURN);
            }
        } else if (first == PlayerCommunicator::MSG1_MESSAGE) {
            if (tokens.size() > 1) {
                QUuid gameID(tokens[0].data());
                this->sendToOtherPlayer(PlayerCommunicator::MSG1_MESSAGE, tokens[1], gameID, playerID);
            }
        }
    });

    playerManager.StartPlayer(playerID, socketDescriptor);
}

bool GameServer::checkLogin(const std::string &login)
{
    //TODO: (1) break up username/password with PlayerCommunicator::DELIMITER;
    //TODO: (2) check password against a list or database or whatever;
    //TODO: (3) add encryption so that you're not sending passwords around in plaintext. That's just not cool.

    return true;
}

//TODO: fix switch redundancy
void GameServer::sendToOtherPlayer(const std::string &msgType, const std::string &msg, const QUuid &gameID, const QUuid &playerID)
{
        auto otherPlayerID = gameManager.GetOtherPlayerID(gameID, playerID);
        if (!otherPlayerID.isNull()) {
            if (msgType == PlayerCommunicator::MSG1_PLAY) {
                playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::PLAY, msg);
            } else if (msgType == PlayerCommunicator::MSG1_STATE) {
                playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::STATE, msg);
            } else if (msgType == PlayerCommunicator::MSG1_MESSAGE) {
                playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::MESSAGE, msg);
            } else if (msgType == PlayerCommunicator::MSG1_ERROR) {
                playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::ERRORS, msg);
            }
        } else {
            playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::ERRORS, ERR_OTHER_NOT_FOUND + gameID.toString().toUtf8().constData());
        }
}

//TODO: fix switch redundancy
void GameServer::sendToBothPlayers(const std::string &msgType, const std::string &msg, const QUuid &gameID, const QUuid &playerID)
{
        auto otherPlayerID = gameManager.GetOtherPlayerID(gameID, playerID);
        if (otherPlayerID != nullptr) {
            if (msgType == PlayerCommunicator::MSG1_PLAY) {
                playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::PLAY, msg);
                playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::PLAY, msg);
            } else if (msgType == PlayerCommunicator::MSG1_STATE) {
                playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::STATE, msg);
                playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::STATE, msg);
            } else if (msgType == PlayerCommunicator::MSG1_MESSAGE) {
                playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::MESSAGE, msg);
                playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::MESSAGE, msg);
            } else if (msgType == PlayerCommunicator::MSG1_ERROR) {
                playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::ERRORS, msg);
                playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::ERRORS, msg);
            }
        } else {
            playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::ERRORS, ERR_OTHER_NOT_FOUND + gameID.toString().toUtf8().constData());
        }
}

void GameServer::cleanPlayerList()
{
    playerManager.CleanPlayerList();
}

void GameServer::cleanGameList()
{
    gameManager.CleanGameList();
}

void GameServer::sendGameStateToOtherPlayer(const QUuid &gameID, const QUuid &playerID)
{
    auto state = gameManager.GetPlayerState(gameID, playerID);
    if (state == nullptr) {
        return;
    }
    state->SetIsInitialData(false);
    QJsonObject object;
    state->write(object);
    QJsonDocument  stateDoc(object);
    QByteArray byteArray = stateDoc.toJson();
    std::string stateStr(byteArray.constData(), byteArray.length());
    auto otherPlayerID = gameManager.GetOtherPlayerID(gameID, playerID);
    if (otherPlayerID.isNull()) {
        return;
    }
    playerManager.SendPlayerCommunications(otherPlayerID, PlayerCommunicator::Communications::STATE, stateStr);
}

void GameServer::sendGameStateToSelf(const QUuid &gameID, const QUuid &playerID, bool initial)
{
    auto state = gameManager.GetPlayerState(gameID, playerID);
    if (state == nullptr) {
        return;
    }
    if (initial) {
        state->SetIsInitialData(true);
    } else {
        state->SetIsInitialData(false);
    }
    QJsonObject object;
    state->write(object);
    QJsonDocument  stateDoc(object);
    QByteArray byteArray = stateDoc.toJson();
    std::string stateStr(byteArray.constData(), byteArray.length());
    playerManager.SendPlayerCommunications(playerID, PlayerCommunicator::Communications::STATE, stateStr);
}
