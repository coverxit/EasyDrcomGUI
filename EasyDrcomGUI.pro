#-------------------------------------------------
#
# Project created by QtCreator 2015-06-22T02:40:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EasyDrcomGUI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    EasyDrcomCore/get_nic_addr.cpp \
    EasyDrcomCore/utils.cpp \
    EasyDrcomCore/_md5.c

HEADERS  += mainwindow.h \
    EasyDrcomCore/drcom_dealer.hpp \
    EasyDrcomCore/drcom_dealer_base.hpp \
    EasyDrcomCore/drcom_dealer_u31.hpp \
    EasyDrcomCore/drcom_dealer_u62.hpp \
    EasyDrcomCore/eap_dealer.hpp \
    EasyDrcomCore/easy_drcom_exception.hpp \
    EasyDrcomCore/get_nic_addr.h \
    EasyDrcomCore/log.hpp \
    EasyDrcomCore/md5.h \
    EasyDrcomCore/udp_dealer.hpp \
    EasyDrcomCore/utils.h \
    PcapHelper.hpp \
    PcapHelper.hpp \
    CancellableThread.h

FORMS    += mainwindow.ui

unix: INCLUDEPATH += /usr/include
unix: LIBS += /usr/lib/x86_64-linux-gnu/libpcap.a

CONFIG += c++11
CONFIG += warn_off
