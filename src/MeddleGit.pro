#-------------------------------------------------
#
# Project created by QtCreator 2017-11-28T18:32:34
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MeddleGit
TEMPLATE = app

win32 {
    RC_FILE = MeddleGit.rc
    LIBS += -luser32
}

macx {
    ICON = images/MeddleGit.icns
    QMAKE_INFO_PLIST = Info.plist
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    main.cpp \
    MainWindow.cpp \
    Git.cpp \
    CommitList.cpp

HEADERS += \
    MainWindow.h \
    Git.h \
    VectorMap.h \
    CommitList.h

FORMS += \
    MainWindow.ui
