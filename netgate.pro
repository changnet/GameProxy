TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    CNetGateSession.cpp \
    CTcpSession.cpp \
    CListenSocket.cpp \
    CPool.cpp \
    CSetting.cpp \
    CBackend.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    CNetGateSession.h \
    CTcpSession.h \
    gstypes.h \
    CListenSocket.h \
    gssocket.h \
    gslog.h \
    CPool.h \
    CSetting.h \
    CBackend.h

INCLUDEPATH += ./libs/parson

LIBS += ../netgate/libs/parson/parson.c \
        -lev

