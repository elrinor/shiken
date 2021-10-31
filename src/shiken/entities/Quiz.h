#ifndef SHIKEN_QUIZ_REPLY_H
#define SHIKEN_QUIZ_REPLY_H

#include <shiken/config.h>
#include <cassert>
#include <QString>
#include <QDateTime>
#include <QMetaType>
#include <arx/StaticBlock.h>

namespace shiken {
// -------------------------------------------------------------------------- //
// Quiz
// -------------------------------------------------------------------------- //
  /**
   * Quiz is an entity class that represents a single quiz.
   */ 
  class Quiz {
  public:
    Quiz(): mQuizId(0), mDiscId(0), mDurationMinutes(0), mLoadedFromXml(false) {}

    bool isNull() const {
      return mTitle.isNull();
    }

    int quizId() const { 
      return mQuizId; 
    }

    void setQuizId(int quizId) { 
      assert(quizId >= 0);

      mQuizId = quizId; 
    }

    int discId() const { 
      return mDiscId; 
    }

    void setDiscId(int discId) { 
      mDiscId = discId; 
    }

    const QString &discStr() const { 
      return mDiscStr; 
    }

    void setDiscStr(const QString &discStr) { 
      mDiscStr = discStr; 
    }

    const QDateTime &startTime() const { 
      return mStartTime; 
    }

    void setStartTime(const QDateTime &startTime) { 
      mStartTime = startTime; 
    }

    int durationMinutes() const {
      return mDurationMinutes;
    }

    void setDurationMinutes(int durationMinutes) {
      mDurationMinutes = durationMinutes;
    }

    const QDateTime &submitEndTime() const {
      return mSubmitEndTime;
    }

    void setSubmitEndTime(const QDateTime &submitEndTime) {
      mSubmitEndTime = submitEndTime;
    }

    const QString &title() const { 
      return mTitle; 
    }

    void setTitle(const QString &title) { 
      mTitle = title; 
    }

    const QString &description() const { 
      return mDescription; 
    }

    void setDescription(const QString &description) { 
      mDescription = description; 
    }

    const QString &guid() const { 
      return mGuid; 
    }

    void setGuid(const QString &guid) { 
      mGuid = guid; 
    }

    const QString &qsgGuid() const { 
      return mQsgGuid; 
    }

    void setQsgGuid(const QString &qsgGuid) { 
      mQsgGuid = qsgGuid; 
    }

    const QString &prjGuid() const { 
      return mPrjGuid; 
    }

    void setPrjGuid(const QString &prjGuid) { 
      mPrjGuid = prjGuid; 
    }

    bool isLoadedFromXml() const {
      return mLoadedFromXml;
    }

    void setLoadedFromXml(bool loadedFromXml) {
      mLoadedFromXml = loadedFromXml;
    }

    const QString &guidToCompress() const {
      if(mQsgGuid.isEmpty())
        return mGuid;
      else
        return mQsgGuid;
    }

    int guidMode() const {
      if(mQsgGuid.isEmpty())
        return 1;
      else
        return 2;
    }

    /**
     * Fixes up a quiz instance after deserialization.
     */
    void fixup() {
      if(mGuid.isEmpty())
        mGuid = mQsgGuid;
    }

  private:
    friend class QuizXml;

    int mQuizId;
    int mDiscId;
    QString mDiscStr;
    QDateTime mStartTime;
    int mDurationMinutes;
    QDateTime mSubmitEndTime;
    QString mTitle;
    QString mDescription;
    QString mGuid;
    QString mQsgGuid; 
    QString mPrjGuid;
    bool mLoadedFromXml;
  };

} // namespace shiken

Q_DECLARE_METATYPE(::shiken::Quiz);

ARX_STATIC_BLOCK(SHIKEN_QUIZ) {
  qRegisterMetaType< ::shiken::Quiz>("Quiz");
}

#endif // SHIKEN_QUIZ_REPLY_H
