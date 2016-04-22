TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CFLAGS += -std=c11 -Wno-unused-parameter
SOURCES += \
    simple_urandom_call.c
