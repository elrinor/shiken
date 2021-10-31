TEMPLATE  = app
CONFIG   += qt warn_on
QT       += webkit

include($(ARXDIR)/arx/Arx.pri)

SOURCES += \
  src/htmlconv.cpp \

HEADERS += \
  src/config.h \
  src/htmlconv/HtmlConv.h \

FORMS += \

RESOURCES += \

INCLUDEPATH += \

UI_DIR    = src/ui
MOC_DIR   = bin/temp/moc
RCC_DIR   = bin/temp/rcc
TARGET    = htmlconv

contains(CONFIG, static) {
  QTPLUGIN += qsqlite
  DEFINES  += HTMLCONV_QT_STATIC_PLUGINS
}

win32 {
  DEFINES += _CRT_SECURE_NO_WARNINGS
}

CONFIG(debug, debug|release) {
  CONFIG           += console
  win32 {
    DESTDIR         = bin/debug
    OBJECTS_DIR     = bin/debug
  }
}

CONFIG(release, debug|release) {
  CONFIG           -= console
  DEFINES          += NDEBUG
  win32 {
    DESTDIR         = bin/release
    OBJECTS_DIR     = bin/release
    #QMAKE_POST_LINK = upx -9 -q $$DESTDIR/$$join(TARGET, "", "", ".exe")
  }
}

win32:LIBS += Crypt32.lib
unix:QMAKE_CXXFLAGS += -std=c++0x
