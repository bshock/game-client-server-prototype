#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connectiondialog.h"
#include "logindialog.h"
#include <iostream>
#include <sstream>
#include <QJsonDocument>
#include <QFile>
#include "connectiondata.h"

const QString MainWindow::MSG_LOGIN = ">Please log in";
const QString MainWindow::MSG_LOGGED_IN_AS = ">Logged in as \"";
const QString MainWindow::MSG_REFUSED_CONNECTION = ">Server refused connection.\n>Make sure IP/Port are correct and server exists!";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(this, SIGNAL(openSetConnection()), this, SLOT(setConnection()));
    connect(this, SIGNAL(openLogin()), this, SLOT(login()));
    connect(this, SIGNAL(startConnection()), this, SLOT(on_pushButton_clicked()));
    connect(this, SIGNAL(enableButtons()), this, SLOT(enableButtonsOnConnection()));
    connect(this, SIGNAL(disableButtons()), this, SLOT(disableButtonsOnDisconnection()));
    connect(this, SIGNAL(setGameStart()), this, SLOT(setGameStartButtons()));
    connect(this, SIGNAL(setGameQuit()), this, SLOT(setGameQuitButtons()));
    connect(this, SIGNAL(addGame(const QString&)), this, SLOT(addItemGamesList(const QString&)));
    connect(this, SIGNAL(addMessage(const QString&)), this, SLOT(addItemMessagesList(const QString&)));
    connect(this, SIGNAL(update()), this, SLOT(updateUI()));
    connect(this, SIGNAL(refreshGames()), this, SLOT(on_pushButtonRefreshGamesList_clicked()));
    connect(this, SIGNAL(disconnect()), this, SLOT(on_pushButtonDisconnect_clicked()));
    client = new PlayerCommunicator();
    client->KillOnDisconnect();
}

MainWindow::~MainWindow()
{
    std::cout << "mainwindow dtor\n";
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    loadConnectionData();
    if (ip.length() > 0) {
        QHostAddress address(ip);
        auto localClient = &client;
        auto localUI = ui;
        client->SetUserNotificationCallback2([this, localClient, localUI](const std::string &first, const std::string &second){

                if (first == PlayerCommunicator::MSG1_LOGIN) {
                    emit addMessage(MSG_LOGIN);
                    emit openLogin();
                }
                else if (first == PlayerCommunicator::MSG1_GAME) {
                    if (!state.GameID().isNull()) {
                        return; //never show other games when you're in the middle of a game
                    }
                    emit addGame(second.data());
                } else if (first == PlayerCommunicator::MSG1_PLAY) {
                    if (second == PlayerCommunicator::MSG2_READY) {
                        emit enableButtons();
                        QString uname(username.data());
                        QString msg;
                        msg = MSG_LOGGED_IN_AS + uname + "\"";
                        emit addMessage(msg);
                    } else {
                        emit addGame(second.data());
                    }
                } else if (first == PlayerCommunicator::MSG1_MESSAGE) {
                    emit addMessage(second.data());
                } else if (first == PlayerCommunicator::MSG1_STATE) {
                    QByteArray byteArray(second.c_str(), static_cast<int>(second.length()));
                    QJsonDocument stateDoc(QJsonDocument::fromJson(byteArray));
                    GameState unknownState;
                    unknownState.read(stateDoc.object());
                    if (unknownState.IsInitialData()) {
                        state.read(stateDoc.object());
                        state.SetIsInitialData(false);
                        emit setGameStart();
                    } else if (unknownState.PlayerNumber() == state.PlayerNumber()) {
                        state.read(stateDoc.object());
                    } else {
                        opponentState.read(stateDoc.object());
                    }
                    emit update();
                } else if (first == PlayerCommunicator::MSG1_ERROR) {
                    if (second == PlayerCommunicator::MSG2_CONNECTIONREFUSED) {
                        emit addMessage(MSG_REFUSED_CONNECTION);
                    }
                }
            });

        client->Start(address, port);

    } else {
        emit setConnection();
    }
}

void MainWindow::on_pushButtonNewGame_clicked()
{
    client->SendCommunications(PlayerCommunicator::Communications::PLAY, PlayerCommunicator::MSG2_NEW);
    //client->SendPlay(PlayerCommunicator::MSG2_NEW);
    ui->listWidgetGames->clear();
}

void MainWindow::setConnection()
{
    ConnectionDialog dialog;
    dialog.setModal(true);

    if (loadConnectionData()) {
        dialog.setIP(ip);
        dialog.setPort(port);
    }

    if (ip == "" && port == 0) {
        dialog.setIP("127.0.0.1");
        dialog.setPort(1234);
    }

    if (dialog.exec() == ConnectionDialog::Rejected) {
        return;
    }

    ip = dialog.ip();
    port = dialog.port();

    saveConnectionData();

    emit startConnection();
}

void MainWindow::login()
{
    LoginDialog dialog;
    dialog.setModal(true);
    dialog.exec();
    QString un = dialog.userName();
    QString pw = dialog.password();
    username = un.toUtf8().constData();
    password = pw.toUtf8().constData();
    client->SendCommunications(PlayerCommunicator::Communications::LOGIN, username, password);
    //client->SendLogin(username, password);
}

void MainWindow::enableButtonsOnConnection()
{
    ui->pushButton->setEnabled(false);
    ui->pushButtonNewGame->setEnabled(true);
    ui->pushButtonQuitGame->setEnabled(false);
    ui->pushButtonDisconnect->setEnabled(true);
    ui->pushButtonRefreshGamesList->setEnabled(true);
}

void MainWindow::disableButtonsOnDisconnection()
{
    ui->pushButton->setEnabled(true);
    ui->pushButtonNewGame->setEnabled(false);
    ui->pushButtonQuitGame->setEnabled(false);
    ui->pushButtonDisconnect->setEnabled(false);
    ui->pushButtonRefreshGamesList->setEnabled(false);
    ui->pushButtonFinishTurn->setEnabled(false);
    ui->pushButtonSendMsg->setEnabled(false);
}

void MainWindow::setGameStartButtons() {
    ui->pushButtonNewGame->setEnabled(false);
    ui->pushButtonQuitGame->setEnabled(true);
}

void MainWindow::setGameQuitButtons() {
    ui->pushButtonNewGame->setEnabled(true);
    ui->pushButtonQuitGame->setEnabled(false);
}

void MainWindow::updateUI() {
    ui->pushButtonSendMsg->setEnabled(!state.GameID().isNull());
    ui->pushButtonFinishTurn->setEnabled(state.ThisSideTurn());
}

void MainWindow::addItemGamesList(const QString &msg) {
    std::string localData = msg.toUtf8().constData();
    auto tokens = PlayerCommunicator::tokenize(localData);
    if (tokens.size() < 5) {
        //TODO: log error
        return;
    }
    std::ostringstream ossLine;
    ossLine << tokens[4] << ":" << tokens[2] << ":" << tokens[1];
    std::ostringstream ossData;
    ossData << tokens[0] << PlayerCommunicator::DELIMITER << tokens[4];
    QString itemText(ossLine.str().data());
    QListWidgetItem *lwi = new QListWidgetItem(itemText);
    QVariant gameID(ossData.str().data());
    lwi->setData(Qt::UserRole, gameID);
    ui->listWidgetGames->addItem(lwi);
    ui->listWidgetGames->scrollToBottom();
}

void MainWindow::addItemMessagesList(const QString &msg) {
    QListWidgetItem *lwi = new QListWidgetItem(msg);
    ui->listWidgetMessages->addItem(lwi);
    ui->listWidgetMessages->scrollToBottom();
}

void MainWindow::on_pushButtonDisconnect_clicked()
{
    state.SetGameID("");
    opponentState.SetGameID("");
    ui->listWidgetGames->clear();
    emit disableButtons();
    client->Shutdown();
    client = new PlayerCommunicator();
    client->KillOnDisconnect();
}

void MainWindow::on_listWidgetGames_itemClicked(QListWidgetItem *item)
{
    QString data = item->data(Qt::UserRole).toString();
    std::string dataStr = data.toUtf8().constData();
    auto tokens = PlayerCommunicator::tokenize(dataStr);
    if (tokens[0].length() == 0) {
        return;
    }
    std::string state;
    if (tokens[1] == PlayerCommunicator::STATE_RESUMABLE) {
        state = PlayerCommunicator::MSG2_RESUME;
    } else if (tokens[1] == PlayerCommunicator::STATE_OPEN) {
        state = PlayerCommunicator::MSG2_JOIN;
    } else {
        //you're already in this game
        return;
    }
    std::ostringstream oss;
    oss << tokens[0] << PlayerCommunicator::DELIMITER << state;
    client->SendCommunications(PlayerCommunicator::Communications::PLAY, oss.str());
    //client->SendPlay(oss.str());
    ui->listWidgetGames->clear();
}

void MainWindow::on_pushButtonFinishTurn_clicked()
{
    if (opponentState.GameID().isNull()) {
        return; //you can't finish a turn if your opponent is missing from the game
    }
    state.SetThisSideTurn(false);
    QJsonObject object2;
    state.write(object2);
    QJsonDocument  stateDoc2(object2);
    QByteArray byteArray2 = stateDoc2.toJson();
    std::string stateStr(byteArray2.constData(), byteArray2.length());
    client->SendCommunications(PlayerCommunicator::Communications::STATE, stateStr);
    //client->SendState(stateStr);
}

void MainWindow::on_pushButtonSendMsg_clicked()
{
    QString msg = ui->textEdit->toPlainText();
    std::string gid(state.GameID().toString().toUtf8().constData());
    std::string message(msg.toUtf8().constData());
    std::string msgStr = gid + PlayerCommunicator::DELIMITER + username + ":" + message;
    QString uname(username.data());
    QString localMsg = uname + ":" + msg;
    emit addMessage(localMsg);
    client->SendCommunications(PlayerCommunicator::Communications::MESSAGE, msgStr);
    ui->textEdit->setText("");
}

void MainWindow::on_pushButtonQuitGame_clicked()
{
    if (!state.GameID().isNull()) {
        std::string gid(state.GameID().toString().toUtf8().constData());
        client->SendCommunications(PlayerCommunicator::Communications::QUITPLAY, gid);
        //client->SendQuitPlay(gid);
        state.SetGameID("");
        opponentState.SetGameID("");
        emit refreshGames();
    }
}

void MainWindow::on_pushButtonRefreshGamesList_clicked()
{
    if (!state.GameID().isNull()) {
        return; //don't get other games when in the middle of a game.
    }
    ui->listWidgetGames->clear();
    client->SendCommunications(PlayerCommunicator::Communications::ALLGAMES);
    //client->SendAllGamesInfo();
    emit setGameQuit();
}

void MainWindow::on_pushButton_2_clicked()
{
    emit setConnection();
}

bool MainWindow::loadConnectionData()
{
    try {
        QFile file("connection.dat");
        file.open(QFile::ReadOnly);
        QDataStream stream(&file);
        ConnectionData data;
        stream >> data;
        file.close();
        ip = data.ip;
        port = data.port;
    } catch (...) {
        return false;
    }
    return true;
}

bool MainWindow::saveConnectionData()
{
    try {
        QFile file("connection.dat");
        file.open(QFile::WriteOnly|QFile::Truncate);
        QDataStream stream(&file);
        ConnectionData data {ip, port};
        stream << data;
        file.close();
    } catch(...) {
        return false;
    }
    return true;
}
