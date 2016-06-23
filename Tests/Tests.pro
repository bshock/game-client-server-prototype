QT += core network
QT -= gui

CONFIG += c++11

TARGET = Tests
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../Common/parsemessages.cpp \
    ../Common/parsesingle.cpp \
    ../Common/gamestate.cpp \
    ../GameServer/playermanager.cpp \
    ../GameServer/gamecontainer.cpp \
    ../GameServer/gamemanager.cpp \
    ../Common/playercommunicator.cpp

HEADERS += \
    ../Common/parsemessage.h \
    ../Common/parsesingle.h \
    ../Common/parsemessages.h \
    ../Common/playerdeletelock.h \
    ../GameServer/playermanager.h \
    ../GameServer/gamecontainer.h \
    ../GameServer/gamemanager.h \
    ../Common/gamestate.h \
    ../Common/playercommunicator.h
