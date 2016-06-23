#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "../Common/playercommunicator.h"
#include "../Common/gamestate.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void openSetConnection();
    void openLogin();
    void startConnection();
    void enableButtons();
    void disableButtons();
    void setGameStart();
    void setGameQuit();
    void addGame(const QString &msg);
    void addMessage(const QString &msg);
    void update();
    void refreshGames();
    void disconnect();

private slots:
    void on_pushButton_clicked();
    void on_pushButtonNewGame_clicked();

    void setConnection();
    void login();
    void enableButtonsOnConnection();
    void disableButtonsOnDisconnection();
    void setGameStartButtons();
    void setGameQuitButtons();
    void addItemGamesList(const QString &msg);
    void addItemMessagesList(const QString &msg);
    void updateUI();
    void on_pushButtonDisconnect_clicked();
    void on_listWidgetGames_itemClicked(QListWidgetItem *item);
    void on_pushButtonFinishTurn_clicked();
    void on_pushButtonSendMsg_clicked();
    void on_pushButtonQuitGame_clicked();
    void on_pushButtonRefreshGamesList_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    //PlayerSocket client;
    PlayerCommunicator *client;
    QString ip;
    int port;
    GameState state;
    GameState opponentState;
    std::string username;
    std::string password;
    QString test;

    bool loadConnectionData();
    bool saveConnectionData();

    static const QString MSG_LOGIN;
    static const QString MSG_LOGGED_IN_AS;
    static const QString MSG_REFUSED_CONNECTION;
};

#endif // MAINWINDOW_H
