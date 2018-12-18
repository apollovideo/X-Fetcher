QT -= gui
QT += core network multimedia concurrent

CONFIG -= app_bundle
CONFIG += thread
CONFIG += c++11 console
CONFIG += debug console
CONFIG += link_prl

TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += _UNICODE QT_DLL QT_CORE_LIB QT_NETWORK_LIB QT_MULTIMEDIA_LIB QT_CONCURRENT_LIB

MOC_DIR += Debug
OBJECTS_DIR += Debug
RCC_DIR += GeneratedFiles

INCLUDEPATH += .
DEPENDPATH += .

INCLUDEPATH += ../XmlRpc-Linux-X-Fetcher

SOURCES += \
    ContactEvents.cpp \
    main.cpp \
    RecorderAccess.cpp \
    ServiceSettings.cpp \
    XFetcher.cpp

HEADERS += \
    ContactEvents.h \
    Definitions.h \
    fcommand.h \
    RecorderAccess.h \
    ServiceSettings.h \
    XFetcher.h

unix:!macx: LIBS += -L../XmlRpc-Linux-X-Fetcher -lxmlrpc
