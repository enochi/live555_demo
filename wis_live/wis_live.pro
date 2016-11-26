TEMPLATE = app
TARGET = ../bin/wis_live
DEFINES += COMMON_LIBRARY POCO_OS_FAMILY_UNIX
QMAKE_CXXFLAGS += -std=c++0x

#INCLUDEPATH += ../arq_to_rtsp_server/
#INCLUDEPATH += ../arq_to_rtsp_server/arq_protocol
INCLUDEPATH += $(TOOLKITS)/include
INCLUDEPATH += ../
INCLUDEPATH +=  ../live555/groupsock/include
INCLUDEPATH += ../live555/BasicUsageEnvironment/include
INCLUDEPATH += ../live555/UsageEnvironment/include
INCLUDEPATH += ../live555/liveMedia/include
INCLUDEPATH += ../Foundation/include/
LIBS += -L../lib -L$(TOOLKITS)/lib -lresend_protocol -lFoundation -llive555 -lACE
SOURCES += \
#webcam_test.cpp \
    wis_media_source.cpp \
    wis_media_subsession.cpp \
    wis_rtsp_server.cpp \
    main.cpp \
    rtsp_module.cpp

HEADERS += \
    wis_media_source.h \
    wis_media_subsession.h \
    wis_rtsp_server.h \
    rtsp_module.h \
    signal_handler.h
