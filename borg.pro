#-------------------------------------------------
#
# Project created by QtCreator 2015-03-25T01:01:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = borg
TEMPLATE = app


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
