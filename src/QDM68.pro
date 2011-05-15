#-------------------------------------------------
#
# Project created by QtCreator 2010-10-31T22:21:53
#
#-------------------------------------------------

QT       += core gui sql

TARGET = QDM68
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    sqltablemodelcheckable.cpp \
    settingsdialog.cpp \
    q3sdc/q3sdc.c \
    q3sdc/parse.c \
    q3sdc/msg.c \
    q3sdc/huff.c \
    q3sdc/dump.c \
    q3sdc/common.c \
    threadparser.cpp \
    listviewcustom.cpp \
    statisticsdialog.cpp

HEADERS  += mainwindow.h \
    sqltablemodelcheckable.h \
    settingsdialog.h \
    q3sdc/q3sdc.h \
    q3sdc/msg.h \
    q3sdc/huff.h \
    q3sdc/common.h \
    q3sdc/dump.h \
    q3sdc/parse.h \
    threadparser.h \
    listviewcustom.h \
    statisticsdialog.h

FORMS    += mainwindow.ui \
    settingsdialog.ui \
    statisticsdialog.ui

RESOURCES += \
    ressources.qrc
