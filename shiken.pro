TEMPLATE  = app
CONFIG   += qt warn_on
QT       += xml sql network

include(3rdparty/arxlib/include/arx/ext/VigraQt.pri)
include(3rdparty/quazip/quazip.pri)

SOURCES += \
    src/shiken_main.cpp \
    src/shiken/actors/QuizListGetter.cpp \
    src/shiken/actors/ScanListGetter.cpp \
    src/shiken/actors/ScanRecognizer.cpp \
    src/shiken/actors/ScanUploader.cpp \
    src/shiken/actors/TestFormGenerator.cpp \
    src/shiken/actors/UpdateDownloader.cpp \
    src/shiken/actors/UserListGetter.cpp \
    src/shiken/actors/VersionGetter.cpp \
    src/shiken/actors/Worker.cpp \
    src/shiken/actors/WorkerRunner.cpp \
    src/shiken/dao/DataAccessDriver.cpp \
    src/shiken/dao/PageDao.cpp \
    src/shiken/dao/QuizDao.cpp \
    src/shiken/dao/ScanDao.cpp \
    src/shiken/dao/SettingsDao.cpp \
    src/shiken/dao/UserDao.cpp \
    src/shiken/network/ConnectionManager.cpp \
    src/shiken/network/ConnectionTester.cpp \
    src/shiken/Shiken.cpp \
    src/shiken/ui/ChooseQuizDialog.cpp \
    src/shiken/ui/ImageDialog.cpp \
    src/shiken/ui/LoginDialog.cpp \
    src/shiken/ui/LoginWidget.cpp \
    src/shiken/ui/MainWidget.cpp \
    src/shiken/ui/PrintConditionsDialog.cpp \
    src/shiken/ui/PrintWarningDialog.cpp \
    src/shiken/ui/ProxySettingsDialog.cpp \
    src/shiken/ui/QuizWidget.cpp \
    src/shiken/ui/StartDialog.cpp \
    src/shiken/utility/Log.cpp \
    src/shiken/utility/SqlQueryModel.cpp \

win32:SOURCES += \
    src/shiken/actors/GsPrinter.cpp \

HEADERS += \
    src/config.h \
    src/shiken/actors/FormGeneratorBase.h \
    src/shiken/actors/FormPrinter.h \
    src/shiken/actors/QuizListGetter.h \
    src/shiken/actors/ScanListGetter.h \
    src/shiken/actors/ScanRecognizer.h \
    src/shiken/actors/ScanUploader.h \
    src/shiken/actors/TestFormGenerator.h \
    src/shiken/actors/UpdateDownloader.h \
    src/shiken/actors/UserListGetter.h \
    src/shiken/actors/VersionGetter.h \
    src/shiken/actors/Worker.h \
    src/shiken/actors/WorkerRunner.h \
    src/shiken/config.h \
    src/shiken/dao/Dao.h \
    src/shiken/dao/DataAccessDriver.h \
    src/shiken/dao/PageDao.h \
    src/shiken/dao/QuizDao.h \
    src/shiken/dao/ScanDao.h \
    src/shiken/dao/SettingsDao.h \
    src/shiken/dao/UserDao.h \
    src/shiken/entities/Page.h \
    src/shiken/entities/Quiz.h \
    src/shiken/entities/Scan.h \
    src/shiken/entities/User.h \
    src/shiken/entities/UsersReply.h \
    src/shiken/network/ConnectionInfo.h \
    src/shiken/network/ConnectionManager.h \
    src/shiken/network/ConnectionTester.h \
    src/shiken/network/PostData.h \
    src/shiken/network/ProxyAuthenticationRequester.h \
    src/shiken/network/ProxyDescription.h \
    src/shiken/parsers/DateTimeParser.h \
    src/shiken/parsers/NamespaceOverrideParser.h \
    src/shiken/parsers/QuizListReplyParser.h \
    src/shiken/parsers/ScanListReplyParser.h \
    src/shiken/Shiken.h \
    src/shiken/ui/ChooseQuizDialog.h \
    src/shiken/ui/ImageDialog.h \
    src/shiken/ui/LoginDialog.h \
    src/shiken/ui/LoginWidget.h \
    src/shiken/ui/MainWidget.h \
    src/shiken/ui/PrintConditionsDialog.h \
    src/shiken/ui/PrintWarningDialog.h \
    src/shiken/ui/ProxySettingsDialog.h \
    src/shiken/ui/QuizWidget.h \
    src/shiken/ui/StartDialog.h \
    src/shiken/utility/BarcodeProcessor.h \
    src/shiken/utility/GuidCompressor.h \
    src/shiken/utility/Log.h \
    src/shiken/utility/SqlQueryModel.h \

win32:HEADERS += \
    src/shiken/actors/GsPrinter.h \

FORMS += \
    src/shiken/ui/LoginWidget.ui \
    src/shiken/ui/LoginDialog.ui \
    src/shiken/ui/MainWidget.ui \
    src/shiken/ui/MainWidgetSingleUser.ui \
    src/shiken/ui/StartDialog.ui \
    src/shiken/ui/StartDialogSingleUser.ui \
    src/shiken/ui/ChooseQuizDialog.ui \
    src/shiken/ui/PrintConditionsDialog.ui \
    src/shiken/ui/PrintWarningDialog.ui \
    src/shiken/ui/PrintWarningDialogSingleUser.ui \
    src/shiken/ui/ProxySettingsDialog.ui \

XSD_FILES = \
    src/shiken/parsers/DateTime.xsd \
    src/shiken/parsers/QuizListReply.xsd \
    src/shiken/parsers/ScanListReply.xsd \
    src/shiken/parsers/UserListReply.xsd \

XSD_MAPS = \
    src/shiken/parsers/DateTime.map \
    src/shiken/parsers/QuizListReply.map \
    src/shiken/parsers/ScanListReply.map \
    src/shiken/parsers/UserListReply.map \

RESOURCES += \
    src/res/shiken.qrc \

win32:RESOURCES += \
    src/res/shiken_win32.qrc \

INCLUDEPATH += \
    src \
    3rdparty/ghostscript \
    3rdparty/eigen \
    3rdparty/vigra/include \
    3rdparty/acvlib/include \
    3rdparty/xsde/libxsde \
    3rdparty/zlib \

QMAKE_LIBDIR += \
    3rdparty/xsde/libxsde/xsde \

UI_DIR    = bin/temp/ui
MOC_DIR   = bin/temp/moc
RCC_DIR   = bin/temp/rcc
TARGET    = generator

contains(CONFIG, static) {
    QTPLUGIN += qsqlite
    DEFINES  += SHIKEN_QT_STATIC_PLUGINS
}

win32 {
    DEFINES += _CRT_SECURE_NO_WARNINGS
}

CONFIG(debug, debug|release) {
    CONFIG              += console
    win32 {
        DESTDIR         = bin/debug
        OBJECTS_DIR     = bin/debug
        contains(QMAKE_CXXFLAGS_DEBUG, -MDd): LIBS += xsde-mdd.lib
        contains(QMAKE_CXXFLAGS_DEBUG, -MTd): LIBS += xsde-mtd.lib
    }
}

CONFIG(release, debug|release) {
    CONFIG              -= console
    DEFINES             += NDEBUG
    win32 {
        DESTDIR         = bin/release
        OBJECTS_DIR     = bin/release
        QMAKE_POST_LINK = upx -9 -q $$DESTDIR/$$join(TARGET, "", "", ".exe")
        contains(QMAKE_CXXFLAGS_RELEASE, -MD):  LIBS += xsde-md.lib
        contains(QMAKE_CXXFLAGS_RELEASE, -MT):  LIBS += xsde-mt.lib
    }
}

#win32:RC_FILE    = src/res/shiken.rc
win32:LIBS += Crypt32.lib

unix:QMAKE_CXXFLAGS += -std=c++0x

# Set up xsde compiler
for(XSD_MAP, XSD_MAPS):XSD_MAPFLAGS += --type-map $${XSD_MAP}
xsde.name = Generating code from ${QMAKE_FILE_IN}
xsde.input = XSD_FILES
xsde.output = $${MOC_DIR}/xsd_${QMAKE_FILE_BASE}.cpp
xsde.commands = 3rdparty/xsde/bin/xsde cxx-parser --no-iostream --output-dir \"$${MOC_DIR}\" --cxx-regex \"/^(.*).xsd/xsd_\\1.cpp/\" --hxx-regex \"/^(.*).xsd/xsd_\\1.h/\" $${XSD_MAPFLAGS} ${QMAKE_FILE_IN}
xsde.CONFIG += target_predeps
xsde.depends = $${XSD_MAPS}
xsde.variable_out = GENERATED_SOURCES
QMAKE_EXTRA_COMPILERS += xsde
