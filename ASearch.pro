#-------------------------------------------------
#
# Project created by QtCreator 2011-02-26T12:08:02
#
#-------------------------------------------------

TARGET = maps/ASearch
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++0x

win32 {
QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++
}

SOURCES += \
    tinyxmlparser.cpp \
    tinyxmlerror.cpp \
    tinyxml.cpp \
    tinystr.cpp \
    xmllogger.cpp \
    isearch.cpp \
    mission.cpp \
    map.cpp \
    list.cpp \
    dijkstra.cpp \
    config.cpp \
    bfs.cpp \
    astar.cpp \
    asearch.cpp \
    jp_search.cpp \
    theta.cpp \
    environmentoptions.cpp

HEADERS += \
    tinyxml.h \
    tinystr.h \
    node.h \
    gl_const.h \
    xmllogger.h \
    isearch.h \
    mission.h \
    map.h \
    ilogger.h \
    list.h \
    dijkstra.h \
    config.h \
    bfs.h \
    astar.h \
    searchresult.h \
    jp_search.h \
    theta.h \
    environmentoptions.h \
    open.h
