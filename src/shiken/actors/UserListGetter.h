#ifndef SHIKEN_USER_LIST_GETTER_H
#define SHIKEN_USER_LIST_GETTER_H

#include <shiken/config.h>
#include <QUrl>
#include <QList>
#include <shiken/entities/UsersReply.h>
#include "Worker.h"

namespace shiken {
  class Shiken;

// -------------------------------------------------------------------------- //
// UserListGetter
// -------------------------------------------------------------------------- //
  /**
   * This worker gets a list of students for the given quiz from the server.
   */
  class UserListGetter: public Worker<UsersReply> {
    Q_OBJECT;
  public:
    UserListGetter(Shiken *ctx, const QUrl &url, const Quiz &quiz):
      Worker<UsersReply>(ctx, 1), mUrl(url), mQuiz(quiz) {}

    virtual UsersReply operator()() OVERRIDE;

  private:
    QUrl mUrl;
    Quiz mQuiz;
  };

} // namespace shiken

#endif // SHIKEN_USER_LIST_GETTER_H

