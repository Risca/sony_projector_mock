QT += core
QT -= gui
QT += serialport

CONFIG += c++11

TARGET = sony_projector_mock
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    serialmock.cpp

HEADERS += \
    serialmock.h
