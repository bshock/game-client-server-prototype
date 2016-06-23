QT += core
QT += network
QT -= gui

CONFIG += c++11

TARGET = GameServer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../Common/parsemessages.cpp \
    ../Common/parsesingle.cpp \
    gameserver.cpp \
    gamecontainer.cpp \
    ../Common/gamestate.cpp \
    playermanager.cpp \
    gamemanager.cpp \
    ../Common/playercommunicator.cpp

HEADERS += \
    ../Common/portablesleep.h \
    ../Common/parsemessages.h \
    ../Common/parsesingle.h \
    gamecontainer.h \
    gameserver.h \
    ../Common/gamestate.h \
    ../Common/playerdeletelock.h \
    playermanager.h \
    playermanager.h \
    gamemanager.h \
    ../Common/playercommunicator.h

DISTFILES += \
    readme
