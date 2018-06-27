QT += core gui widgets

TEMPLATE = app
CONFIG += console c++11

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp \
    config.cpp \
    databaseio.cpp \
    gitcrawler.cpp \
    handler.cpp \
    misc.cpp \
    statistics.cpp \
    mainwindow.cpp

HEADERS += \
    config.h \
    databaseio.h \
    gitcrawler.h \
    handler.h \
    misc.h \
    statistics.h \
    mainwindow.h

LIBS += -L/usr/include/cppconn/ -lboost_filesystem -lmysqlcppconn -lcurlpp -pthread -lcurl -lboost_system
INCLUDEPATH += /usr/include/cppconn/include

FORMS +=
