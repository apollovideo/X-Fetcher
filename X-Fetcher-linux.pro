QT -= gui
QT += core network multimedia websockets concurrent

CONFIG -= app_bundle
CONFIG += thread
CONFIG += c++11 console
CONFIG += debug console
CONFIG += link_prl

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += _UNICODE QT_DLL QT_CORE_LIB QT_NETWORK_LIB QT_WEBSOCKETS_LIB QT_MULTIMEDIA_LIB QT_CONCURRENT_LIB

MOC_DIR += ../build-X-Fetcher-linux-Desktop_Qt_5_10_1_GCC_64bit-Debug/GeneratedFiles/debug
OBJECTS_DIR += ../build-X-Fetcher-linux-Desktop_Qt_5_10_1_GCC_64bit-Debug/Debug
RCC_DIR += ../build-X-Fetcher-linux-Desktop_Qt_5_10_1_GCC_64bit-Debug/GeneratedFiles

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += .
INCLUDEPATH += ../build-X-Fetcher-linux-Desktop_Qt_5_10_1_GCC_64bit-Debug/GeneratedFiles/debug
DEPENDPATH += .

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

unix:!macx: LIBS += -L$$PWD/../build-XmlRpcLinux-Desktop_Qt_5_10_1_GCC_64bit-Debug/ -lXmlRpcLinux

INCLUDEPATH += $$PWD/../XmlRpcLinux
DEPENDPATH += $$PWD/../XmlRpcLinux

unix:!macx: PRE_TARGETDEPS += $$PWD/../build-XmlRpcLinux-Desktop_Qt_5_10_1_GCC_64bit-Debug/libXmlRpcLinux.a
