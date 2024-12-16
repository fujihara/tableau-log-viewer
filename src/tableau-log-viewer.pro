QT       += core gui
QT       += network
QT       += webenginewidgets
QT       += widgets
QT       += concurrent

TARGET = "tlv"
TEMPLATE = app

FORMS       = \
    filtertab.ui \
    finddlg.ui \
    highlightdlg.ui \
    logtab.ui \
    mainwindow.ui \
    optionsdlg.ui \
    savefilterdialog.ui \
    searchtab.ui \
    valuedlg.ui 

HEADERS     = \
    colorlibrary.h \
    column.h \
    filesearcher.h \
    filtertab.h \
    finddlg.h \
    highlightdlg.h \
    highlightoptions.h \
    logtab.h \
    mainwindow.h \
    options.h \
    optionsdlg.h \
    pathhelper.h \
    processevent.h \
    savefilterdialog.h \
    searchopt.h \
    searchtab.h \
    singleton.h \
    statusbar.h \
    tokenizer.h \
    treeitem.h \
    treemodel.h \
    valuedlg.h \
    zoomabletreeview.h \
    themeutils.h \
    theme.h \
    qjsonutils.h

SOURCES     = \
    colorlibrary.cpp \
    filesearcher.cpp \
    filtertab.cpp \
    finddlg.cpp \
    highlightdlg.cpp \
    highlightoptions.cpp \
    logtab.cpp \
    main.cpp \
    mainwindow.cpp \
    options.cpp \
    optionsdlg.cpp \
    pathhelper.cpp \
    processevent.cpp \
    savefilterdialog.cpp \
    searchopt.cpp \
    searchtab.cpp \
    statusbar.cpp \
    tokenizer.cpp \
    treeitem.cpp \
    treemodel.cpp \
    valuedlg.cpp \
    zoomabletreeview.cpp \
    themeutils.cpp \
    theme.cpp \
    qjsonutils.cpp

RESOURCES   = resources.qrc

win32:RC_ICONS += ../resources/images/tlv.ico

ICON = ../resources/images/tlv.icns

CONFIG += c++17
CONFIG += x86_64 

QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

VERSION = 1.5.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

INCLUDEPATH += $$PWD/../third-party/ads/include

mac{
    LIBS += -L$$PWD/../third-party/ads/lib/ -lqtadvanceddocking
}
else:win32 {

}




