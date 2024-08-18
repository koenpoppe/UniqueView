QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase silent c++20
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_UniqueView.cpp

HEADERS += \
    UniqueView.h

DISTFILES += \
    Readme.md
