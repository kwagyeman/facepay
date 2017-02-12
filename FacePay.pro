QT += core gui widgets network multimedia multimediawidgets
TARGET = FacePay
TEMPLATE = app
SOURCES += main.cpp mainwindow.cpp
HEADERS += mainwindow.h
FORMS += mainwindow.ui
CONFIG += openssl-linked
RESOURCES += mainwindow.qrc
