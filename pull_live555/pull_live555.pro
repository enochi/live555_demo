TEMPLATE = app
TARGET = ../bin/pull_live555
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


INCLUDEPATH += ../
INCLUDEPATH +=  ../live555/groupsock/include
INCLUDEPATH += ../live555/BasicUsageEnvironment/include
INCLUDEPATH += ../live555/UsageEnvironment/include
INCLUDEPATH += ../live555/liveMedia/include

LIBS += -L../lib -llive555
INCLUDEPATH += $(TOOLKITS)/include
#LIBS += -L$(TOOLKITS)/lib -lmuduo -lprofiler -ltcmalloc -lpthread
#DEFINES += _WIN32
#DEFINES +=-DFD_SETSIZE=10000
#LIBS += -lws2_32

SOURCES += \
    testRTSPClient.cpp \
    simplertpsession.cpp

HEADERS += \
    simplertpsession.h
