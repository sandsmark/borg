QT       += core gui widgets

exists(localconfig.pri) {
    include(localconfig.pri)
}

TARGET = borg
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    botmodel.cpp \
    botviewdelegate.cpp \
    patheditor.cpp \
    spinbox.cpp

HEADERS  += mainwindow.h \
    botmodel.h \
    botviewdelegate.h \
    patheditor.h \
    spinbox.h

DISTFILES += \
    localconfig.pri

RESOURCES += \
    resources.qrc
