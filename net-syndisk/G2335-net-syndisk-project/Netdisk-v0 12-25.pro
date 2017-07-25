#-------------------------------------------------
#
# Project created by QtCreator 2016-12-13T13:51:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Netdisk-v0
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    logindialog.cpp \
    signupdialog.cpp \
    socket.cpp \
    threadSend.cpp \
    threadTcp.cpp \
    threadloop.cpp

HEADERS  += mainwindow.h \
    logindialog.h \
    signupdialog.h \
    socket.h \
    define.h \
    threadSend.h \
    threadTcp.h \
    threadloop.h

FORMS    += mainwindow.ui \
    logindialog.ui \
    signupdialog.ui

LIBS     += -lWS2_32

RESOURCES += \
    icon.qrc

QMAKE_CXXFLAGS_RELEASE += -O0 -g
QMAKE_CFLAGS_RELEASE += -O0 -g
QMAKE_LFLAGS_RELEASE =
DEFINES -=QT_NO_DEBUG_OUTPUT

