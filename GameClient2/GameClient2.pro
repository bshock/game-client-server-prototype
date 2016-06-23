#-------------------------------------------------
#
# Project created by QtCreator 2016-06-04T15:03:14
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GameClient2
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../Common/parsemessages.cpp \
    ../Common/parsesingle.cpp \
    logindialog.cpp \
    ../Common/gamestate.cpp \
    connectiondialog.cpp \
    ../Common/playercommunicator.cpp

HEADERS  += mainwindow.h \
    ../Common/parsemessages.h \
    ../Common/parsesingle.h \
    ../Common/portablesleep.h \
    logindialog.h \
    ../Common/gamestate.h \
    connectiondialog.h \
    connectiondata.h \
    ../Common/playercommunicator.h

FORMS    += mainwindow.ui \
    logindialog.ui \
    connectiondialog.ui
