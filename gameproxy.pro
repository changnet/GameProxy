TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    CProxySession.cpp \
    CTcpSession.cpp \
    CListenSocket.cpp \
    CPool.cpp \
    CSetting.cpp \
    CBackend.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    CProxySession.h \
    CTcpSession.h \
    gstypes.h \
    CListenSocket.h \
    gssocket.h \
    gslog.h \
    CPool.h \
    CSetting.h \
    CBackend.h

INCLUDEPATH += ./libs/parson

LIBS += ../GameProxy/libs/parson/parson.c \
        -lev

