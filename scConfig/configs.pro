TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CFLAGS += -std=c99

INCLUDEPATH += \
    ../cJSON

SOURCES += \
    ../cJSON/cJSON.c \
    src/genConfigs.c \
    src/main.c \
    src/scJson2Txt42.c

OBJECTS_DIR = ./debug

HEADERS += \
    ../sctwin.h
