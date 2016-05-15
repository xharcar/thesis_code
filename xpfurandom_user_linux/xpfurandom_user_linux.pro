TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CFLAGS += -Wno-sign-compare

SOURCES += \
    xpfurandom.c \
    xpfurandom_user.c

HEADERS += \
    xpfurandom.h
