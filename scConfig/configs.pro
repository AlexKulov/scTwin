TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CFLAGS += -std=c89

INCLUDEPATH += \
    ../cJSON

SOURCES += \
    ../cJSON/cJSON.c \
    src/main.c

OBJECTS_DIR = ./debug
