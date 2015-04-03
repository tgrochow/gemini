TARGET    = gemini_server

TEMPLATE  = app

CONFIG   += console
CONFIG   += c++11
CONFIG   -= app_bundle

QT       += core
QT       += network
QT       -= gui

SOURCES  += main.cpp \
            server.cpp \
            descriptor.cpp \
            device_list.cpp \
            rule.cpp \
            rule_set.cpp \
            control.cpp

HEADERS  += server.hpp \
            descriptor.hpp \
            device_list.hpp \
            rule.hpp \
            rule_set.hpp \
            control.hpp

unix:!macx: LIBS += -lusb-1.0
