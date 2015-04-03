TARGET     = gemini_gui

TEMPLATE   = app

CONFIG    += c++11

QT        += core
QT        += gui

greaterThan(QT_MAJOR_VERSION,4): QT += widgets
greaterThan(QT_MAJOR_VERSION,4): QT += network

SOURCES   += main.cpp\
             main_window.cpp \
             device_info.cpp \
             rule_info.cpp

HEADERS   += main_window.hpp \
             device_info.hpp \
             rule_info.hpp

FORMS     += main_window.ui

RESOURCES += icons/icons.qrc
