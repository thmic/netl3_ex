QT += core widgets network

CONFIG += c++17
CONFIG += warn_on

TARGET = QtNetworkDebuggerClient
TEMPLATE = app

SOURCES += \
    main.cpp \
    clientwindow.cpp

HEADERS += \
    clientwindow.h
