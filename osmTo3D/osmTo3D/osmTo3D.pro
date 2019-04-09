#-------------------------------------------------
#
# Project created by QtCreator 2018-09-13T16:28:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = osmTo3D
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    paint.cpp \
    opensmloading.cpp \
    oglmanager.cpp \
    resourcemanager.cpp \
    shader.cpp \
    texture2d.cpp \
    light.cpp \
    camera.cpp \
    model.cpp \
    carcontrol.cpp

HEADERS  += mainwindow.h \
    paint.h \
    opensmloading.h \
    oglmanager.h \
    resourcemanager.h \
    shader.h \
    texture2d.h \
    light.h \
    camera.h \
    model.h \
    carcontrol.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc

DISTFILES +=
