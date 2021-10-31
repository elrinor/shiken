#ifndef SHIKEN_QUIZ_DAO_H
#define SHIKEN_QUIZ_DAO_H

#include <shiken/config.h>
#include <shiken/entities/Quiz.h>
#include "Dao.h"

namespace shiken {
// -------------------------------------------------------------------------- //
// QuizDao
// -------------------------------------------------------------------------- //
  /**
   * Data access object for quizzes.
   */
  class QuizDao: public Dao {
  public:
    bool insert(Quiz &quiz);
    bool update(const Quiz &quiz);
    bool remove(const Quiz &quiz);
    bool removeByLoadedFromXml(bool isLoadedFromXml);
    QList<Quiz> selectAll();
    Quiz selectById(int quizId);
    Quiz selectByGuid(const QString &guid);

  protected:
    friend class DataAccessDriver;

    QuizDao(DataAccessDriver *dataAccessDriver, QSqlDatabase connection): Dao(dataAccessDriver, connection) {}
  };

} // namespace shiken

#endif // SHIKEN_QUIZ_DAO_H
