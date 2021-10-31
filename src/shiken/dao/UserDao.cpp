#include "UserDao.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <arx/Foreach.h>

namespace shiken {
  namespace {
    void fillQuery(QSqlQuery &query, const User &user, bool useUserId) {
      if(useUserId)
        query.bindValue(":user_id", user.userId());
      query.bindValue(":quiz_id", user.quizId());
      query.bindValue(":login", user.login());
      query.bindValue(":guid", user.guid());
      query.bindValue(":compressed_guid", QString("_" + user.compressedGuid())); 
      query.bindValue(":is_unknown", static_cast<int>(user.isUnknown()));
      
      /* "_" hack is needed to make SQLite compare compressed guids as strings.  
       * Comparing them as numbers fails for no apparent reason. Probably
       * a bug/quirk of SQLite. */
    }

    User makeUser(const QSqlRecord &record) {
      User user;
      user.setUserId        (record.value("user_id").toInt());
      user.setQuizId        (record.value("quiz_id").toInt());
      user.setLogin         (record.value("login").toString());
      user.setGuid          (record.value("guid").toString());
      user.setCompressedGuid(record.value("compressed_guid").toString().mid(1));
      user.setUnknown       (record.value("is_unknown").toBool());
      return user;
    }

  } // namespace `anonymous-namespace`

  bool UserDao::silentInsert(User &user) {
    QSqlQuery query(connection());
    query.prepare("INSERT INTO users (quiz_id, login, guid, compressed_guid, is_unknown) VALUES(:quiz_id, :login, :guid, :compressed_guid, :is_unknown)");
    fillQuery(query, user, true);

    bool success = query.exec();
    if(success)
      user.setUserId(query.lastInsertId().toInt());
    return success;
  }

  bool UserDao::insert(User &user) {
    bool success = silentInsert(user);
    if(success)
      emitChanged();
    return success;
  }

  bool UserDao::insert(QList<User> &users) {
    if(users.empty())
      return true;

    bool success = true;
    foreach(User &user, users)
      success &= silentInsert(user);

    emitChanged(); /* Always emit. */
    return success;
  }

  bool UserDao::update(const User &user) {
    QSqlQuery query(connection());
    query.prepare("UPDATE users SET quiz_id = :quiz_id, login = :login, compressed_guid = :compressed_guid, is_unknown = :is_unknown, guid = :guid WHERE user_id = :user_id");
    fillQuery(query, user, true);

    bool success = query.exec();
    if(success)
      emitChanged();
    return success;
  }

  bool UserDao::remove(const User &user) {
    QSqlQuery query(connection()); 
    query.prepare("DELETE FROM users WHERE user_id = :user_id");
    query.bindValue(":user_id", user.userId());

    bool success = query.exec();
    if(success)
      emitChanged();
    return success;
  }

  QList<User> UserDao::selectByQuizId(int quizId) {
    QSqlQuery query("SELECT * FROM users WHERE quiz_id = :quiz_id", connection());
    query.bindValue(":quiz_id", quizId);

    QList<User> result;
    while(query.next())
      result.push_back(makeUser(query.record()));
    return result;
  }

  User UserDao::selectByCompressedGuidAndQuizId(const QString &compressedGuid, int quizId) {
    QSqlQuery query(connection());
    query.prepare("SELECT * FROM users WHERE compressed_guid = :compressed_guid AND quiz_id = :quiz_id");
    query.bindValue(":compressed_guid", QString("_" + compressedGuid));
    query.bindValue(":quiz_id", quizId);
    query.exec();

    User result;
    if(query.next())
      result = makeUser(query.record());
    return result;
  }

  User UserDao::selectByLoginAndQuizId(const QString &login, int quizId) {
    QSqlQuery query(connection());
    query.prepare("SELECT * FROM users WHERE login = :login AND quiz_id = :quiz_id");
    query.bindValue(":login", login);
    query.bindValue(":quiz_id", quizId);
    query.exec();

    User result;
    if(query.next())
      result = makeUser(query.record());
    return result;
  }

} // namespace shiken
