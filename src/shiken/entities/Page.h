#ifndef SHIKEN_PAGE_H
#define SHIKEN_PAGE_H

#include <shiken/config.h>
#include <cassert>
#include <boost/preprocessor/stringize.hpp>
#include <QString>
#include <QDateTime>
#include <QMetaType>
#include <arx/StaticBlock.h>

namespace shiken {
// -------------------------------------------------------------------------- //
// Page
// -------------------------------------------------------------------------- //
  /**
   * Page is an entity class that represents a single answer sheet that was
   * printed by shiken.
   *
   * Each Page corresponds to some User. Correspondence is defined by 
   * compressedGuid and quizId.
   */
  class Page {
  public:
    Page(): mPageId(0), mUserId(0) {}

    Page(int pageId, int userId, const QDateTime &printTime):
      mPageId(pageId), mUserId(userId), mPrintTime(printTime)
    {
      assert(userId >= 0);
      assert(!printTime.isNull());
    }

    bool isNull() const {
      return mPrintTime.isNull();
    }

    int pageId() const {
      return mPageId;
    }

    void setPageId(int pageId) {
      mPageId = pageId;
    }

    int userId() const {
      return mUserId;
    }

    void setUserId(int userId) {
      mUserId = userId;
    }

    const QDateTime &printTime() const {
      return mPrintTime;
    }

    void setPrintTime(const QDateTime &time) {
      mPrintTime = time;
    }

  private:
    int mPageId;
    int mUserId;
    QDateTime mPrintTime;
  };

} // namespace shiken

Q_DECLARE_METATYPE(::shiken::Page);

ARX_STATIC_BLOCK(SHIKEN_PAGE) {
  qRegisterMetaType< ::shiken::Page>("Page");
}

#endif // SHIKEN_PAGE_H
