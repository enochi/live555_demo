QT       -= core gui

QMAKE_LFLAGS+=-rdynamic
DEFINES += RESEND_PROTOCOL_LIBRARY
QMAKE_CXXFLAGS += -std=c++0x
TARGET = ../lib/resend_protocol
TEMPLATE = lib

DEFINES += LEIZHOU

INCLUDEPATH += ../suyun_sdk ../suyun_sdk/adt ../suyun_sdk/adt/linux ../
INCLUDEPATH += ../Foundation/include/


HEADERS += \
    ring_buf_handler.h \
    resend_protocol.h \
    pkg_buf.h \
    pkg_recevicer.h

SOURCES += \
    ring_buf_handler.cpp \
    pkg_buf.cpp \
    pkg_recevicer.cpp

#QMAKE_CXXFLAGS += -m32
