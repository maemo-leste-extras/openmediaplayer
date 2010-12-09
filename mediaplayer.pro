#-------------------------------------------------
#
# Project created by QtCreator 2010-12-07T20:00:31
#
#-------------------------------------------------

QT       += core gui

TARGET = mediaplayer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    musicwindow.cpp \
    nowplayingwindow.cpp \
    videoswindow.cpp

HEADERS  += mainwindow.h \
    musicwindow.h \
    nowplayingwindow.h \
    videoswindow.h

FORMS    += mainwindow.ui \
    musicwindow.ui \
    nowplayingwindow.ui \
    videoswindow.ui

CONFIG += mobility
MOBILITY = 

symbian {
    TARGET.UID3 = 0xedf29700
    # TARGET.CAPABILITY += 
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}

maemo5* {
    CONFIG += link_pkgconfig
    PKGCONFIG += libosso
    QT += maemo5
}

RESOURCES += \
    images.qrc
