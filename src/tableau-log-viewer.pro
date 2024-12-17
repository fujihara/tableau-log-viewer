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

VERSION = 1.5.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
INCLUDEPATH += $$PWD/../third-party/ads/include

# qmake will process all libraries linked to by the application and find their meta-information
CONFIG += link_prl

macx {
    # Config for mac
    ICON = ../resources/images/tlv.icns
    CONFIG += app_bundle
    # Third-party libs
    CONFIG(debug, debug|release) {
        LIBS += -L$$PWD/../third-party/ads/lib/mac -lqtadvanceddocking_debug
    } else {
        LIBS += -L$$PWD/../third-party/ads/lib/mac -lqtadvanceddocking
    }
}
else {
    # Config for windows
    RC_ICONS += ../resources/images/tlv.ico
    CONFIG += c++17
    CONFIG += x86_64
    CONFIG += windows
    QMAKE_TARGET_COMPANY = "Tableau Software"
    QMAKE_TARGET_DESCRIPTION = "Tableau Log Viewer"
    QMAKE_TARGET_COPYRIGHT = "Copyright 2024 Tableau Software"
    # Third-party libs
    CONFIG(debug, debug|release) {
        LIBS += -L$$PWD/../third-party/ads/lib/win32 -lqtadvanceddockingd
    } else {
        LIBS += -L$$PWD/../third-party/ads/lib/win32 -lqtadvanceddocking
    }
}
