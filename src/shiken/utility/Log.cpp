#include "Log.h"
#include <QDebug>
#include <QTextCodec>
#include <QStringList>
#include <QUrl>
#include <QDateTime>


#ifndef SHIKEN_NO_LOGGING

namespace shiken {
  LogStream &LogStream::operator<<(const QUrl &url) {
    *this << url.toString();

    return *this;
  }

  LogStream &LogStream::operator<<(const QStringList &list) {
    *this << static_cast<const QList<QString> &>(list);

    return *this;
  }

  LogStream &LogStream::operator<<(const QDateTime &dateTime) {
    *this << dateTime.toString();

    return *this;
  }

  Log::Log() {
    /* Open file. */
    mFile.setFileName(SHIKEN_LOG_NAME);
    mFile.open(QIODevice::WriteOnly | QIODevice::Append);

    /* Set up log stream. */
    mStream.setDevice(&mFile);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    if(codec == NULL)
      codec = QTextCodec::codecForLocale();
    mStream.setCodec(codec);

    /* Redirect Qt messages into log file. */
    qInstallMsgHandler(&qtMsgHandler);
  }

  void Log::qtMsgHandler(QtMsgType msgType, const char* msg) {
    QWriteLocker locker(Log::instance().lock());
    QTextStream &stream = Log::instance().stream();

    switch(msgType) {
    case QtDebugMsg:
      stream << "DEBUG: ";
      break;
    case QtWarningMsg:
      stream << "WARNING: ";
      break;
    case QtCriticalMsg:
      stream << "CRITICAL: ";
      break;
    case QtFatalMsg:
      stream << "FATAL: ";
      break;
    }

    stream << msg << "\n" << flush;
  }

  Log Log::sInstance;

} // namespace shiken

#endif // SHIKEN_NO_LOGGING
