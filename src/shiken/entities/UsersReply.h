#ifndef SHIKEN_USERS_REPLY_H
#define SHIKEN_USERS_REPLY_H

#include <shiken/config.h>
#include <QList>
#include "Quiz.h"
#include "User.h"

namespace shiken {
// -------------------------------------------------------------------------- //
// UsersReply
// -------------------------------------------------------------------------- //
  class UsersReply {
  public:
    UsersReply() {}

    UsersReply(const Quiz &quiz, const QList<User> &users): mQuiz(quiz), mUsers(users) {}

    const Quiz &quiz() const {
      return mQuiz;
    }

    const QList<User> users() const {
      return mUsers;
    }

    const User &user(int index) const {
      return mUsers[index];
    }

    bool isNull() const {
      return mQuiz.isNull();
    }

  private:
    Quiz mQuiz;
    QList<User> mUsers;
  };

} // namespace shiken

#endif // SHIKEN_USERS_REPLY_H
