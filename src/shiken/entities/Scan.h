#ifndef SHIKEN_SCAN_H
#define SHIKEN_SCAN_H

#include <shiken/config.h>
#include <cassert>
#include <QDateTime>
#include <QMetaType>
#include <arx/Utility.h> /* For unreachable. */
#include <arx/StaticBlock.h>

namespace shiken {

  /**
   * Scan is an entity class that represents a single scan.
   *
   * If scan is recognized, then it corresponds to some Page. Correspondence 
   * is defined by pageId.
   */
  class Scan {
  public:
    enum State {
      UNPROCESSED = 0,
      RECOGNIZED = 1,
      UNRECOGNIZED = 2,
      UPLOADED = 3,
      SERVER_RECOGNIZED = 4,
      SERVER_FAILED = 5,
      SERVER_WRONG = 6,
      SERVER_MANUAL = 7,
      STATE_COUNT = 8,
      INVALID = -1
    };

    static QString stateString(State state) {
      switch(state) {
        case UNPROCESSED:        return QS("В очереди");
        case RECOGNIZED:         return QS("Распознан");
        case UNRECOGNIZED:       return QS("Ошибка при распознавании");
        case UPLOADED:           return QS("Отправлен на сервер");
        /* It was decided that the following states must be 
         * indistinguishable from the user's point of view. */
#if 0
        case SERVER_RECOGNIZED:  return QS("Распознан на сервере");
        case SERVER_FAILED:      return QS("Ошибка при распознании на сервере");
        case SERVER_WRONG:       return QS("Неправильно распознан на сервере");
        case SERVER_MANUAL:      return QS("Вручную распознан на сервере");
#else
        case SERVER_RECOGNIZED:
        case SERVER_FAILED:
        case SERVER_WRONG:
        case SERVER_MANUAL:
          return QS("На сервере");
#endif
        default:
          unreachable();
      }
    }

    Scan(): mQuizId(0), mState(INVALID), mPageId(0) {}

    Scan(int quizId, const QString &fileName, const QString &hash, State state, int pageId):
      mQuizId(quizId), mFileName(fileName), mHash(hash), mState(state), mPageId(pageId)
    {
      assert(!fileName.isNull());
      assert(state >= 0 && state < STATE_COUNT);
    }

    bool isNull() const {
      return mHash.isNull();
    }

    int quizId() const {
      return mQuizId;
    }

    void setQuizId(int quizId) {
      mQuizId = quizId;
    }

    const QString &fileName() const {
      return mFileName;
    }

    void setFileName(const QString &fileName) {
      mFileName = fileName;
    }

    const QString &hash() const {
      return mHash;
    }

    void setHash(const QString &hash) { 
      mHash = hash;
    }

    State state() const {
      return mState;
    }

    QString stateString() const {
      return stateString(mState);
    }

    void setState(State state) {
      assert(state >= 0 && state < STATE_COUNT);

      mState = state;
    }

    int pageId() const {
      return mPageId;
    }

    void setPageId(int pageId) {
      mPageId = pageId;
    }

  private:
    int mQuizId;
    QString mFileName;
    QString mHash;
    State mState;
    int mPageId;
  };


  /**
   * Extended scan is a convenience class that contains some additional fields
   * that can be used to identify its corresponding user and page.
   * 
   * It is used as a type that server scan replies are deserialized into.
   */
  class ExtendedScan: public Scan {
  public:
      ExtendedScan() {}

      const QString &compressedGuid() const {
          return m_compressedGuid;
      }

      void setCompressedGuid(const QString &compressedGuid) {
          m_compressedGuid = compressedGuid;
      }

      const QDateTime &printTime() const {
          return m_printTime;
      }

      void setPrintTime(const QDateTime &printTime) {
          m_printTime = printTime;
      }

  private:
      QString m_compressedGuid;
      QDateTime m_printTime;
  };

} // namespace shiken

Q_DECLARE_METATYPE(::shiken::Scan);
Q_DECLARE_METATYPE(::shiken::ExtendedScan);

ARX_STATIC_BLOCK(SHIKEN_SCAN) {
  qRegisterMetaType< ::shiken::Scan>("Scan");
  qRegisterMetaType< ::shiken::ExtendedScan>("ExtendedScan");
}

#endif // SHIKEN_SCAN_H
