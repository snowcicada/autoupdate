#-------------------------------------------------
#
# Project created by QtCreator 2017-07-15T10:14:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DESTDIR = ../huama_bin/autoupdate/
TARGET = autoupdate
TEMPLATE = app

INCLUDEPATH += ./include

LIBS += -L../autoupdate/lib -lcurl

SOURCES += main.cpp\
        cautoupdate.cpp \
    jsoncpp/json_writer.cpp \
    jsoncpp/json_valueiterator.inl \
    jsoncpp/json_value.cpp \
    jsoncpp/json_reader.cpp \
    jsoncpp/json_internalmap.inl \
    jsoncpp/json_internalarray.inl \
    jsoncpp/CJson.cpp \
    ccurl.cpp

HEADERS  += cautoupdate.h \
    jsoncpp/writer.h \
    jsoncpp/value.h \
    jsoncpp/reader.h \
    jsoncpp/json_batchallocator.h \
    jsoncpp/json.h \
    jsoncpp/forwards.h \
    jsoncpp/features.h \
    jsoncpp/config.h \
    jsoncpp/CJson.h \
    jsoncpp/autolink.h \
    ccurl.h

FORMS    += cautoupdate.ui

QMAKE_CXXFLAGS += -std=c++11

RESOURCES += \
    images/images.qrc

RC_FILE = images/logo.rc

