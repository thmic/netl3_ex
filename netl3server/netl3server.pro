QT += core widgets network

CONFIG += c++17
CONFIG += warn_on

TARGET = QtNetworkDebuggerServer
TEMPLATE = app

SOURCES += \
    main.cpp \
    serverwindow.cpp

HEADERS += \
    serverwindow.h
