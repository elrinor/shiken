DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

HEADERS += $$PWD/crypt.h $$PWD/ioapi.h $$PWD/quazip.h $$PWD/quazipfile.h $$PWD/quazipfileinfo.h $$PWD/quazipnewinfo.h $$PWD/unzip.h $$PWD/zip.h

SOURCES += $$PWD/ioapi.c $$PWD/quazip.cpp $$PWD/quazipfile.cpp $$PWD/quazipnewinfo.cpp $$PWD/unzip.c $$PWD/zip.c

unix:LIBS += -lz
