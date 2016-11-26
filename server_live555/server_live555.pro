TEMPLATE = app
TARGET = ../bin/server_live555
LIBS += -L ../lib -llive555
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


LIBS += -L../lib -llive555
INCLUDEPATH += ../
INCLUDEPATH +=  ../live555/groupsock/include
INCLUDEPATH += ../live555/BasicUsageEnvironment/include
INCLUDEPATH += ../live555/UsageEnvironment/include
INCLUDEPATH += ../live555/liveMedia/include

LIBS += -L../lib -llive555
INCLUDEPATH += $(TOOLKITS)/include
LIBS += -L$(TOOLKITS)/lib -lmuduo -lprofiler -ltcmalloc -lpthread

SOURCES += \
    ../live555/mediaServer/DynamicRTSPServer.cpp \
    ../live555/mediaServer/live555MediaServer.cpp

HEADERS += \
    ../live555/mediaServer/DynamicRTSPServer.hh
