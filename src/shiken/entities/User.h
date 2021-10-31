#ifndef SHIKEN_USER_H
#define SHIKEN_USER_H

#include <shiken/config.h>
#include <cassert>
#include <QString>
#include <QMetaType>
#include <arx/StaticBlock.h>

namespace shiken {
// -------------------------------------------------------------------------- //
// User
// -------------------------------------------------------------------------- //
  /**
   * User is an entity class that represents a single student.
   *
   * Each user belongs to some Quiz. 
   */
  class User {
  public:
    User(): mUserId(0), mQuizId(0), mUnknown(false) {}

    User(int userId, int quizId, const QString &login, const QString &guid, const QString &compressedGuid, bool unknown):
      mUserId(userId), mQuizId(quizId), mLogin(login), mGuid(guid), mCompressedGuid(compressedGuid), mUnknown(unknown)
    {
      assert(!login.isNull());
      assert(!compressedGuid.isNull());
    };

    bool isNull() const {
      return mCompressedGuid.isNull();
    }

    int userId() const {
      return mUserId;
    }

    void setUserId(int userId) {
      mUserId = userId;
    }

    int quizId() const {
      return mQuizId;
    }

    void setQuizId(int quizId) {
      mQuizId = quizId;
    }

    const QString &login() const {
      return mLogin;
    }

    void setLogin(const QString &login) {
      mLogin = login;
    }

    const QString &guid() const {
      return mGuid;
    }

    void setGuid(const QString &guid) {
      mGuid = guid;
    }

    const QString &compressedGuid() const {
      return mCompressedGuid;
    }

    void setCompressedGuid(const QString &compressedGuid) {
      mCompressedGuid = compressedGuid;
    }

    bool isUnknown() const {
      return mUnknown;
    }

    void setUnknown(bool unknown) {
      mUnknown = unknown;
    }

  private:
    int mUserId;
    int mQuizId;
    QString mLogin;
    QString mGuid;
    QString mCompressedGuid;
    bool mUnknown;
  };

} // namespace shiken

Q_DECLARE_METATYPE(::shiken::User);

ARX_STATIC_BLOCK(SHIKEN_USER) {
  qRegisterMetaType< ::shiken::User>("User");
}

#endif // SHIKEN_USER_H

