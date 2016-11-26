TEMPLATE = app
TARGET = ../bin/client_live555
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
INCLUDEPATH += ../
INCLUDEPATH +=  ../live555/groupsock/include
INCLUDEPATH += ../live555/BasicUsageEnvironment/include
INCLUDEPATH += ../live555/UsageEnvironment/include
INCLUDEPATH += ../live555/liveMedia/include

LIBS += -L../lib -llive555

SOURCES += \
    testRTSPClient.cpp

HEADERS +=
