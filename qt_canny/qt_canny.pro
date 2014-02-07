TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH +=`pkg-config opencv --cflags`
LIBS += `pkg-config opencv --libs`


SOURCES += main.cpp


