#ifndef SHIKEN_LOG_H
#define SHIKEN_LOG_H

#include <shiken/config.h>

#ifndef SHIKEN_NO_LOGGING
#  include <boost/preprocessor/cat.hpp>
#  include <QTextStream>
#  include <QFile>
#  include <QReadWriteLock>
#  include <QList>

class QStringList;
class QDateTime;
class QUrl;

namespace shiken {
// -------------------------------------------------------------------------- //
// LogStream
// -------------------------------------------------------------------------- //
  /**
   * LogStream extends QTextStream providing new stream operations useful for
   * logging, such as List printing.
   *
   * QDebug cannot be used for such purpose because it doesn't provide any
   * methods for encoding manipulation. On Windows, it's impossible to
   * make QDebug write UTF-8.
   */
  class LogStream: public QTextStream {
  public:
    template<class T>
    LogStream &operator<<(const T &value) {
      static_cast<QTextStream &>(*this) << value;
      
      return *this;
    }

    template<class T>
    LogStream &operator<<(const QList<T> &list) {
      *this << "[";
      for(int i = 0; i < list.size(); i++) {
        if(i != 0)
          *this << ",";
        *this << list[i];
      }
      *this << "]";

      return *this;
    }

    LogStream &operator<<(const QUrl &url);

    LogStream &operator<<(const QStringList &list);

    LogStream &operator<<(const QDateTime &dateTime);

  };


// -------------------------------------------------------------------------- //
// Log
// -------------------------------------------------------------------------- //
  /**
   * Log singleton class.
   */
  class Log {
  public:
    /**
     * @returns                        Actual log stream.
     */
    LogStream &stream() {
      return mStream;
    }
    
    /**
     * @returns                        Lock associated with the stream.
     */
    QReadWriteLock *lock() {
      return &mLock;
    }

    /**
     * @returns                        Singleton instance.
     */
    static Log &instance() {
      return sInstance;
    }

  protected:
    Log();

  private:
    static void qtMsgHandler(QtMsgType msgType, const char* msg);

    static Log sInstance;

    QReadWriteLock mLock;
    QFile mFile;
    LogStream mStream;
  };

} // namespace shiken

#endif // SHIKEN_NO_LOGGING

/**
 * @def SHIKEN_LOG_MESSAGE(WHAT)
 *
 * Writes the given message to the application log if logging is enabled.
 * Evaluates to no-op otherwise.
 *
 * @param WHAT                         Logging expression.
 */
#ifndef SHIKEN_NO_LOGGING
#  define SHIKEN_LOG_MESSAGE(WHAT) {                                            \
    QWriteLocker BOOST_PP_CAT(locker, __LINE__)(::shiken::Log::instance().lock()); \
    ::shiken::Log::instance().stream() << WHAT << "\n" << flush;                \
  }
#else 
#  define SHIKEN_LOG_MESSAGE(WHAT)
#endif

#endif // SHIKEN_LOG_H
