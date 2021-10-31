TEMPLATE  = app
CONFIG   += qt warn_on console

include(3rdparty/arxlib/include/arx/ext/VigraQt.pri)

SOURCES += \
  src/kpextract.cpp \

HEADERS += \
  src/config.h \

FORMS += \

RESOURCES += \

INCLUDEPATH +=  \
  src \
  3rdparty/vigra/include \
  3rdparty/acvlib/include \

UI_DIR    = src/ui
MOC_DIR   = bin/temp/moc
RCC_DIR   = bin/temp/rcc
TARGET    = kpextract

CONFIG(debug, debug|release) {
  win32 {
    DESTDIR         = bin/debug
    OBJECTS_DIR     = bin/debug
  }
}

CONFIG(release, debug|release) {
  DEFINES          += NDEBUG
  win32 {
    DESTDIR         = bin/release
    OBJECTS_DIR     = bin/release
  }
}

unix:LIBS += -lboost_program_options
unix:QMAKE_CXXFLAGS += -std=c++0x
