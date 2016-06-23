#include <QCoreApplication>
#include <iostream>
#include "../Common/parsemessages.h"
#include "../Common/parsesingle.h"
#include "../Common/playercommunicator.h"
#include <mutex>
#include <thread>
#include "../Common/playerdeletelock.h"
#include "../GameServer/playermanager.h"
#include "../GameServer/gamecontainer.h"
#include "../GameServer/gamemanager.h"

void TestParseMessages() {
    std::string first = "this_game";
    std::string second = "CREATE_NEW_GAME";

    std::string message = ParseMessages::START + first + ParseMessages::EQUAL + second + ParseMessages::END;
    ParseMessages parser;
    parser.AddString(message);
    if (parser.NewMessageAvailable()) {
        auto stuff = parser.GetKeyValuePair();
        if (stuff.first == first && stuff.second == second) {
            std::cout << "TestParseMessages:Succeeded\n";
        } else {
            std::cout << "TestParseMessages:Failed. first=" << stuff.first << ";second=" << stuff.second << "\n";
        }
        if (parser.NewMessageAvailable()) {
            std::cout << "TestParseMessages:Failed to eliminate old messages.\n";
        }

    } else {
        std::cout << "TestParseMessages:Failed to find new message.\n";
    }

}

//Does this still work with two messages in a row?
void TestParseMessages2() {
    std::string first = "this_game";
    std::string second = "CREATE_NEW_GAME";
    std::string first2 = "some other game";
    std::string second2 = "DO SOMETHING cool";

    std::string message = ParseMessages::START + first + ParseMessages::EQUAL + second + ParseMessages::END;
    std::string message2 = ParseMessages::START + first2 + ParseMessages::EQUAL + second2 + ParseMessages::END;
    ParseMessages parser;
    parser.AddString(message);
    parser.AddString(message2);
    if (parser.NewMessageAvailable()) {
        auto stuff = parser.GetKeyValuePair();
        if (stuff.first == first && stuff.second == second) {
            std::cout << "TestParseMessages2:Succeeded on 1\n";
        } else {
            std::cout << "TestParseMessages2:Failed. first=" << stuff.first << ";second=" << stuff.second << "\n";
        }
        if (!parser.NewMessageAvailable()) {
            std::cout << "TestParseMessages2:Failed to find second message.\n";
        }

    } else {
        std::cout << "TestParseMessages2:Failed to find new messages.\n";
    }
    if (parser.NewMessageAvailable()) {
        auto stuff = parser.GetKeyValuePair();
        if (stuff.first == first2 && stuff.second == second2) {
            std::cout << "TestParseMessages2:Succeeded on 2\n";
        } else {
            std::cout << "TestParseMessages2:Failed. first=" << stuff.first << ";second=" << stuff.second << "\n";
        }
        if (parser.NewMessageAvailable()) {
            std::cout << "TestParseMessages2:Failed to eliminate old messages.\n";
        }

    } else {
        std::cout << "TestParseMessages2:Failed to find new messages.\n";
    }
}

void TestParseMessages3() {
    std::string first = "this_game";
    std::string second = "CREATE_NEW_GAME";
    std::string first2 = "some other game";
    std::string second2 = "DO SOMETHING cool";
    std::string first3 = "GAME";
    std::string second3 = "not a very flexible test";

    std::string messageA = ParseMessages::START + first;
    std::string messageB = ParseMessages::EQUAL + second + ParseMessages::END;
    std::string message2A = ParseMessages::START + first2 + ParseMessages::EQUAL;

    ParseMessages parser;
    parser.AddString(messageA);
    parser.AddString(messageB);
    parser.AddString(message2A);

    if (parser.NewMessageAvailable()) {
        auto stuff = parser.GetKeyValuePair();
        if (stuff.first == first && stuff.second == second) {
            std::cout << "TestParseMessages3:Succeeded on 1\n";
        } else {
            std::cout << "TestParseMessages3:Failed. first=" << stuff.first << ";second=" << stuff.second << "\n";
        }
        if (parser.NewMessageAvailable()) {
            std::cout << "TestParseMessages3:Failed to recognize incomplete message.\n";
        }

    } else {
        std::cout << "TestParseMessages3:Failed to find new messages.\n";
    }

    std::string message2B = second2 + ParseMessages::END;
    std::string message3A = ParseMessages::START + first3;
    std::string message3B = ParseMessages::EQUAL;
    std::string message3C = second3 + ParseMessages::END;
    parser.AddString(message2B);
    parser.AddString(message3A);
    parser.AddString(message3B);
    parser.AddString(message3C);

    if (parser.NewMessageAvailable()) {
        auto stuff = parser.GetKeyValuePair();
        if (stuff.first == first2 && stuff.second == second2) {
            std::cout << "TestParseMessages3:Succeeded on 2\n";
        } else {
            std::cout << "TestParseMessages3:Failed. first=" << stuff.first << ";second=" << stuff.second << "\n";
        }
        if (!parser.NewMessageAvailable()) {
            std::cout << "TestParseMessages3:Failed to find new messages.\n";
        }

    } else {
        std::cout << "TestParseMessages3:Failed to find new messages.\n";
    }

    if (parser.NewMessageAvailable()) {
        auto stuff = parser.GetKeyValuePair();
        if (stuff.first == first3 && stuff.second == second3) {
            std::cout << "TestParseMessages3:Succeeded on 3\n";
        } else {
            std::cout << "TestParseMessages3:Failed. first=" << stuff.first << ";second=" << stuff.second << "\n";
        }
        if (parser.NewMessageAvailable()) {
            std::cout << "TestParseMessages3:Failed to eliminate old messages.\n";
        }

    } else {
        std::cout << "TestParseMessages3:Failed to find new messages.\n";
    }

}


void TestParseSingle() {
    std::string first = "this_game";
    std::string second = "CREATE_NEW_GAME";

    std::string message = ParseMessages::START + first + ParseMessages::EQUAL + second + ParseMessages::END;
    ParseSingle parser(message);
    if (parser.IsComplete()) {
        auto stuff = parser.GetKeyValuePair();
        if (stuff.first == first && stuff.second == second) {
            std::cout << "TestParseSingle:Succeeded\n";
        } else {
            std::cout << "TestParseSingle:Failed. first=" << stuff.first << ";second=" << stuff.second << "\n";
        }

    } else {
        std::cout << "TestParseSingle:Failed to find a complete message.\n";
    }

}

PlayerDeleteLock lock;
int readCount;
int writeCount;
bool readLockFailure;
bool writeLockFailure;

void pause_read_thread(int n)
{
    lock.ReadLock();
    ++readCount;
    if (writeCount > 0) {
        std::cout << "TestRWLock: Failed to prevent write while reading.\n";
        readLockFailure = true;
    }
    std::cout << "Read thread pausing\n";
    std::this_thread::sleep_for (std::chrono::seconds(n));
    std::cout << "pause of " << n << " seconds ended\n";
    --readCount;
    lock.ReadUnlock();
}

void pause_write_thread(int n)
{
    lock.WriteLock();
    ++writeCount;
    if (readCount > 0) {
        std::cout << "TestRWLock: Failed to prevent read while writing.\n";
        writeLockFailure = true;
    }
    std::cout << "Write thread pausing\n";
    std::this_thread::sleep_for (std::chrono::seconds(n));
    std::cout << "pause of " << n << " seconds ended\n";
    -- writeCount;
    lock.WriteUnlock();
}

void TestRWLock() {
    std::cout << "TestRWLock start\n";
    readCount = 0;
    writeCount = 0;
    readLockFailure = false;
    writeLockFailure = false;
    std::thread t1 (pause_read_thread, 2);
    std::thread t2 (pause_write_thread, 2);
    std::thread t3 (pause_read_thread, 2);
    t1.join();
    t2.join();
    t3.join();
    std::cout << "TestRWLock complete\n";
    if (!readLockFailure && !writeLockFailure) {
        std::cout << "TestRWLock: Multiple read, single write exclusion succeeded.\n";
    } else {
        std::cout << "TestRWLock: Multiple read, single write exclusion failed.\n";
    }
}

void TestPlayerManager_AddPlayer() {
    PlayerManager pm;
    int pre_count = pm.GetPlayerCount();
    pm.AddPlayer();
    int post_count = pm.GetPlayerCount();
    if (pre_count == 0 && post_count == 1) {
        std::cout << "TestPlayerManager_AddPlayer:Succeeded in adding a player.\n";
    } else {
        std::cout << "TestPlayerManager_AddPlayer:Failed in adding a player.\n";
    }
}

void TestPlayerManager_SetLogin_GetLogin() {
    PlayerManager pm;
    auto result = pm.AddPlayer();
    const std::string login = {"a,b"};
    pm.SetPlayerLogin(result, login);
    std::string login_result = pm.GetPlayerLogin(result);
    if (login_result == login) {
        std::cout << "TestPlayerManager_SetLogin_GetLogin:Succeeded in set/get login.\n";
    } else {
        std::cout << "TestPlayerManager_SetLogin_GetLogin:Failed in set/get login:" << login_result << "\n";
    }
}

void TestPlayerManager_DeletePlayer() {
    PlayerManager pm;
    int pre_count = pm.GetPlayerCount();
    auto result = pm.AddPlayer();
    int post_count = pm.GetPlayerCount();
    pm.DeletePlayer(result);
    int final_count = pm.GetPlayerCount();
    if (pre_count == 0 && post_count == 1 && final_count == 0) {
        std::cout << "TestPlayerManager_DeletePlayer:Succeeded in deleting a player.\n";
    } else {
        std::cout << "TestPlayerManager_DeletePlayer:Failed in deleting a player.\n";
    }
}

void TestGameContainer_AddPlayer_RemovePlayer_Various() {
    GameContainer gc;
    QUuid playerID("d7f3f426-197a-4b0d-921c-f9b2e2deb9c1");
    std::string playerLogin = "jdoe,12345";
    gc.AddPlayer(playerID, playerLogin);
    if (gc.IsOpenGame()) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various:Succeeded showing open game.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various:FAILED showing open game.\n";
    }
    if (gc.IsPlayerActive(playerLogin)) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various:Succeeded showing player active.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various:FAILED showing player active.\n";
    }
    if (!gc.IsPreviousGame(playerLogin)) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various:Succeeded showing not previous game.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various:FAILED showing not previous game.\n";
    }
    if (gc.GetPlayerID(playerLogin) == playerID) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various:Succeeded getting player id.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various:FAILED getting player id.\n";
    }
    if (!gc.AreBothPlayersActive()) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various:Succeeded showing both players aren't active.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various:FAILED showing both players aren't active.\n";
    }
    gc.RemovePlayer(playerLogin);
    if (!gc.IsPlayerActive(playerLogin)) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various:Succeeded removing player.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various:FAILED removing.\n";
    }
}

void TestGameContainer_AddPlayer_RemovePlayer_Various2() {
    GameContainer gc;
    QUuid playerID("d7f3f426-197a-4b0d-921c-f9b2e2deb9c1");
    std::string playerLogin = "jdoe,12345";
    gc.AddPlayer(playerID, playerLogin);
    QUuid playerID2("e3d2a13b-a378-4527-808c-a99a918315d7");
    std::string playerLogin2 = "jroe,67890";
    gc.AddPlayer(playerID2, playerLogin2);
    if (!gc.IsOpenGame()) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:Succeeded showing not open game.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:FAILED showing not open game.\n";
    }
    if (gc.IsPlayerActive(playerLogin)) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:Succeeded showing player active.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:FAILED showing player active.\n";
    }
    if (gc.IsPlayerActive(playerLogin2)) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:Succeeded showing player2 active.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:FAILED showing player2 active.\n";
    }
    if (!gc.IsPreviousGame(playerLogin2)) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:Succeeded showing not previous game.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:FAILED showing not previous game.\n";
    }
    if (gc.GetPlayerID(playerLogin) == playerID) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:Succeeded getting player id.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:FAILED getting player id.\n";
    }
    if (gc.GetPlayerID(playerLogin2) == playerID2) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:Succeeded getting player2 id.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:FAILED getting player2 id.\n";
    }
    if (gc.AreBothPlayersActive()) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:Succeeded showing both players are active.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:FAILED showing both players are active.\n";
    }
    gc.RemovePlayer(playerLogin);
    if (!gc.IsPlayerActive(playerLogin)) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:Succeeded removing player.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:FAILED removing player.\n";
    }
    gc.RemovePlayer(playerLogin2);
    if (!gc.IsPlayerActive(playerLogin2)) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:Succeeded removing player2.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:FAILED removing player2.\n";
    }
    if (gc.IsPreviousGame(playerLogin)) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:Succeeded showing previous game for player1.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:FAILED showing previous game for player1.\n";
    }
    if (gc.IsPreviousGame(playerLogin2)) {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:Succeeded showing previous game for player2.\n";
    } else {
        std::cout << "TestGameContainer_AddPlayer_RemovePlayer_Various2:FAILED showing previous game for player2.\n";
    }
}

void TestGameContainer_GetTurn_SwitchTurn() {
    GameContainer gc;
    QUuid playerID("d7f3f426-197a-4b0d-921c-f9b2e2deb9c1");
    std::string playerLogin = "jdoe,12345";
    gc.AddPlayer(playerID, playerLogin);
    QUuid playerID2("e3d2a13b-a378-4527-808c-a99a918315d7");
    std::string playerLogin2 = "jroe,67890";
    gc.AddPlayer(playerID2, playerLogin2);
    if (gc.GetPlayerTurn(playerLogin) == 0) {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:Succeeded showing player1 turn.\n";
    } else {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:FAILED showing player1 turn.\n";
    }
    if (gc.GetPlayerTurn(playerLogin2) == 1) {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:Succeeded showing player2 turn.\n";
    } else {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:FAILED showing player2 turn.\n";
    }
    if (gc.GetPlayerTurn(playerLogin) == gc.CurrentTurn()) {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:Succeeded showing player1 current turn.\n";
    } else {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:FAILED showing player1 current turn.\n";
    }
    if (!gc.GetPlayerTurn(playerLogin2) == gc.CurrentTurn()) {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:Succeeded showing player2 current turn.\n";
    } else {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:FAILED showing player2 current turn.\n";
    }
    if (gc.GetPlayerID(0) == playerID) {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:Succeeded showing player1 id.\n";
    } else {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:FAILED showing player1 id.\n";
    }
    if (gc.GetPlayerLogin(0) == playerLogin) {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:Succeeded showing player1 login.\n";
    } else {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:FAILED showing player1 login.\n";
    }
    if (gc.GetPlayerID(1) == playerID2) {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:Succeeded showing player2 id.\n";
    } else {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:FAILED showing player2 id.\n";
    }
    if (gc.GetPlayerLogin(1) == playerLogin2) {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:Succeeded showing player2 login.\n";
    } else {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:FAILED showing player2 login.\n";
    }
    gc.SwitchTurn();
    if (!gc.GetPlayerTurn(playerLogin) == gc.CurrentTurn()) {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:Succeeded showing player1 switch turn.\n";
    } else {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:FAILED showing player1 switch turn.\n";
    }
    if (gc.GetPlayerTurn(playerLogin2) == gc.CurrentTurn()) {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:Succeeded showing player2 switch turn.\n";
    } else {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:FAILED showing player2 switch turn.\n";
    }
    gc.SwitchTurn();
    if (gc.GetPlayerTurn(playerLogin) == gc.CurrentTurn()) {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:Succeeded showing player1 switch turn again.\n";
    } else {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:FAILED showing player1 switch turn again.\n";
    }
    if (!gc.GetPlayerTurn(playerLogin2) == gc.CurrentTurn()) {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:Succeeded showing player2 switch turn again.\n";
    } else {
        std::cout << "TestGameContainer_GetTurn_SwitchTurn:FAILED showing player2 switch turn again.\n";
    }
}

void TestGameContainer_GetNullPlayerID() {
    GameContainer gc;
    QUuid playerID("d7f3f426-197a-4b0d-921c-f9b2e2deb9c1");
    std::string playerLogin = "jdoe,12345";
    gc.AddPlayer(playerID, playerLogin);
    if (gc.GetPlayerID("wrong").isNull()) {
        std::cout << "TestGameContainer_GetNullPlayerID:Succeeded detecting null player id.\n";
    } else {
        std::cout << "TestGameContainer_GetNullPlayerID:FAILED detecting null player id.\n";
    }
}

void TestGameManager_AddGame() {
    GameManager gm;
    int pre_count = gm.GetGameCount();
    QUuid playerID("d7f3f426-197a-4b0d-921c-f9b2e2deb9c1");
    std::string playerLogin = "jdoe,12345";
    gm.AddGame(playerID, playerLogin);
    int post_count = gm.GetGameCount();
    if (pre_count == 0 && post_count == 1) {
        std::cout << "TestGameManager_AddGame:Succeeded in adding a game.\n";
    } else {
        std::cout << "TestGameManager_AddGame:Failed in adding a game.\n";
    }
}

void TestGameManager_AddPlayer() {
    GameManager gm;
    QUuid playerID("d7f3f426-197a-4b0d-921c-f9b2e2deb9c1");
    std::string playerLogin = "jdoe,12345";
    QUuid playerID2("e3d2a13b-a378-4527-808c-a99a918315d7");
    std::string playerLogin2 = "jroe,67890";
    QUuid gameID = gm.AddGame(playerID, playerLogin);
    QUuid otherPlayerID = gm.GetOtherPlayerID(gameID, playerID);
    if (otherPlayerID == "") {
        std::cout << "TestGameManager_AddPlayer:Succeeded in seeing only one player in game.\n";
    } else {
        std::cout << "TestGameManager_AddPlayer:Failed in seeing only one player in game.\n";
    }
    if (!gm.AddPlayer(gameID, playerID2, playerLogin2)) {
        std::cout << "TestGameManager_AddPlayer:Failed in adding second player to game.\n";
    }
    otherPlayerID = gm.GetOtherPlayerID(gameID, playerID);
    if (otherPlayerID == playerID2) {
        std::cout << "TestGameManager_AddPlayer:Succeeded in seeing second player in game.\n";
    } else {
        std::cout << "TestGameManager_AddPlayer:Failed in seeing second player in game.\n";
    }
}

void TestGameManager_RemovePlayer() {
    GameManager gm;
    QUuid playerID("d7f3f426-197a-4b0d-921c-f9b2e2deb9c1");
    std::string playerLogin = "jdoe,12345";
    QUuid playerID2("e3d2a13b-a378-4527-808c-a99a918315d7");
    std::string playerLogin2 = "jroe,67890";
    QUuid gameID = gm.AddGame(playerID, playerLogin);
    QUuid otherPlayerID = gm.GetOtherPlayerID(gameID, playerID);
    gm.AddPlayer(gameID, playerID2, playerLogin2);
    otherPlayerID = gm.GetOtherPlayerID(gameID, playerID);
    if (!gm.RemovePlayer(gameID, playerID)) {
        std::cout << "TestGameManager_RemovePlayer:Failed in removing player to game.\n";
    }
    otherPlayerID = gm.GetOtherPlayerID(gameID, playerID2);
    if (otherPlayerID == "") {
        std::cout << "TestGameManager_RemovePlayer:Succeeded in seeing first player removed from game.\n";
    } else {
        std::cout << "TestGameManager_RemovePlayer:Failed in seeing second player removed from game.\n";
    }
}

void TestGameManager_ResumePlayer() {
    GameManager gm;
    QUuid playerID("d7f3f426-197a-4b0d-921c-f9b2e2deb9c1");
    std::string playerLogin = "jdoe,12345";
    QUuid playerID2("e3d2a13b-a378-4527-808c-a99a918315d7");
    std::string playerLogin2 = "jroe,67890";
    QUuid gameID = gm.AddGame(playerID, playerLogin);
    QUuid otherPlayerID = gm.GetOtherPlayerID(gameID, playerID);
    gm.AddPlayer(gameID, playerID2, playerLogin2);
    if (!gm.AreBothPlayersActive(gameID)) {
        std::cout << "TestGameManager_ResumePlayer:Failed in seeing both players active in game.\n";
    }
    otherPlayerID = gm.GetOtherPlayerID(gameID, playerID);
    if (!gm.RemovePlayer(gameID, playerID)) {
        std::cout << "TestGameManager_ResumePlayer:Failed in removing player to game.\n";
    }
    otherPlayerID = gm.GetOtherPlayerID(gameID, playerID2);
    if (otherPlayerID == "") {
        std::cout << "TestGameManager_ResumePlayer:Succeeded in seeing first player removed from game.\n";
    } else {
        std::cout << "TestGameManager_ResumePlayer:Failed in seeing second player removed from game.\n";
    }
    if (gm.AreBothPlayersActive(gameID)) {
        std::cout << "TestGameManager_ResumePlayer:Failed in seeing  both players not active in game.\n";
    }
    if (!gm.ResumePlayer(gameID, playerID, playerLogin)) {
        std::cout << "TestGameManager_ResumePlayer:Failed in resuming player in game.\n";
    }
    if (gm.HasPlayer(gameID, playerLogin)) {
        std::cout << "TestGameManager_ResumePlayer:Succeeded in seeing both players active in game.\n";
    } else {
        std::cout << "TestGameManager_ResumePlayer:Failed in seeing both players active in game.\n";
    }
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TestParseMessages();
    TestParseMessages2();
    TestParseMessages3();
    TestParseSingle();

    TestRWLock();

    TestPlayerManager_AddPlayer();
    TestPlayerManager_SetLogin_GetLogin();
    TestPlayerManager_DeletePlayer();

    TestGameContainer_AddPlayer_RemovePlayer_Various();
    TestGameContainer_AddPlayer_RemovePlayer_Various2();
    TestGameContainer_GetTurn_SwitchTurn();
    TestGameContainer_GetNullPlayerID();

    TestGameManager_AddGame();
    TestGameManager_AddPlayer();
    TestGameManager_RemovePlayer();
    TestGameManager_ResumePlayer();


    return a.exec();
}
