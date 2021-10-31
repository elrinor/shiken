#ifndef SHIKEN_USER_DAO_H
#define SHIKEN_USER_DAO_H

#include <shiken/config.h>
#include <shiken/entities/User.h>
#include "Dao.h"

namespace shiken {
// -------------------------------------------------------------------------- //
// UserDao
// -------------------------------------------------------------------------- //
  /**
   * Data access object for users.
   */
  class UserDao: public Dao {
  public:
    bool insert(User &user);
    bool insert(QList<User> &users);
    bool update(const User &user);
    bool remove(const User &user);
    
    QList<User> selectByQuizId(int quizId);
    User selectByCompressedGuidAndQuizId(const QString &compressedGuid, int quizId);
    User selectByLoginAndQuizId(const QString &login, int quizId);

  protected:
    friend class DataAccessDriver;

    bool silentInsert(User &user);

    UserDao(DataAccessDriver *dataAccessDriver, QSqlDatabase connection): Dao(dataAccessDriver, connection) {}
  };

} // namespace shiken

#endif // SHIKEN_USER_DAO_H
