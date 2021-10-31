#include "QuizDao.h"
#include <cassert>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

namespace shiken {
  namespace {
    void fillQuery(QSqlQuery &query, const Quiz &quiz, int quizId) {
      query.bindValue(":quiz_id",            quizId);
      query.bindValue(":disc_id",            quiz.discId());
      query.bindValue(":disc_str",           quiz.discStr());
      query.bindValue(":start_time",         quiz.startTime().toString(SHIKEN_DATE_TIME_FORMAT));
      query.bindValue(":duration_minutes",   quiz.durationMinutes());
      query.bindValue(":submit_end_time",    quiz.submitEndTime().toString(SHIKEN_DATE_TIME_FORMAT));
      query.bindValue(":title",              quiz.title());
      query.bindValue(":description",        quiz.description());
      query.bindValue(":guid",               quiz.guid());
      query.bindValue(":qsg_guid",           quiz.qsgGuid());
      query.bindValue(":prj_guid",           quiz.prjGuid());
      query.bindValue(":is_loaded_from_xml", static_cast<int>(quiz.isLoadedFromXml()));
    }

    Quiz makeQuiz(const QSqlRecord &record) {
      Quiz quiz;
      quiz.setQuizId         (record.value("quiz_id"           ).toInt());
      quiz.setDiscId         (record.value("disc_id"           ).toInt());
      quiz.setDiscStr        (record.value("disc_str"          ).toString());
      quiz.setStartTime      (QDateTime::fromString(record.value("start_time"      ).toString(), SHIKEN_DATE_TIME_FORMAT));
      quiz.setDurationMinutes(record.value("duration_minutes"  ).toInt());
      quiz.setSubmitEndTime  (QDateTime::fromString(record.value("submit_end_time" ).toString(), SHIKEN_DATE_TIME_FORMAT));
      quiz.setTitle          (record.value("title"             ).toString());
      quiz.setDescription    (record.value("description"       ).toString());
      quiz.setGuid           (record.value("guid"              ).toString());
      quiz.setQsgGuid        (record.value("qsg_guid"          ).toString());
      quiz.setPrjGuid        (record.value("prj_guid"          ).toString());
      quiz.setLoadedFromXml  (record.value("is_loaded_from_xml").toBool());
      return quiz;
    }

  } // namespace `anonymous-namespace`

  bool QuizDao::insert(Quiz &quiz) {    
    int quizId = quiz.quizId() > 0 ? quiz.quizId() : 1000000 + qrand() * RAND_MAX + qrand();

    QSqlQuery query(connection());
    query.prepare(" \
      INSERT INTO quizzes(quiz_id, disc_id, disc_str, start_time, duration_minutes, submit_end_time, title, description, guid, qsg_guid, prj_guid, is_loaded_from_xml) \
      VALUES(:quizId, :disc_id, :disc_str, :start_time, :duration_minutes, :submit_end_time, :title, :description, :guid, :qsg_guid, :prj_guid, :is_loaded_from_xml) \
    ");
    fillQuery(query, quiz, quizId);

    bool success = query.exec();
    if(success) {
      quiz.setQuizId(quizId);
      emitChanged();
    }
    return success;
  }

  bool QuizDao::update(const Quiz &quiz) {
    assert(quiz.quizId() > 0);

    QSqlQuery query(connection());
    query.prepare(" \
      UPDATE quizzes SET disc_id = :disc_id, disc_str = :disc_str, start_time = :start_time, duration_minutes = :duration_minutes, submit_end_time = :submit_end_time, \
      title = :title, description = :description, guid = :guid, qsg_guid = :qsg_guid, prj_guid = :prj_guid, is_loaded_from_xml = :is_loaded_from_xml WHERE quiz_id = :quiz_id \
    ");
    fillQuery(query, quiz, quiz.quizId());

    bool success = query.exec();
    if(success)
      emitChanged();
    return success;
  }

  bool QuizDao::remove(const Quiz &quiz) {
    QSqlQuery query(connection()); 
    query.prepare("DELETE FROM quizzes WHERE quiz_id = :quiz_id");
    query.bindValue(":quiz_id", quiz.quizId());

    bool success = query.exec();
    if(success)
      emitChanged();
    return success;
  }

  bool QuizDao::removeByLoadedFromXml(bool isLoadedFromXml) {
    QSqlQuery query(connection()); 
    query.prepare("DELETE FROM quizzes WHERE is_loaded_from_xml = :is_loaded_from_xml");
    query.bindValue(":is_loaded_from_xml", static_cast<int>(isLoadedFromXml));

    bool success = query.exec();
    if(success)
      emitChanged();
    return success;
  }

  QList<Quiz> QuizDao::selectAll() {
    QSqlQuery query("SELECT * FROM quizzes", connection());

    QList<Quiz> result;
    while(query.next())
      result.push_back(makeQuiz(query.record()));
    return result;
  }

  Quiz QuizDao::selectById(int quizId) {
    QSqlQuery query(connection());
    query.prepare("SELECT * FROM quizzes WHERE quiz_id = :quiz_id");
    query.bindValue(":quiz_id", quizId);
    query.exec();

    Quiz result;
    if(query.next())
      result = makeQuiz(query.record());
    return result;
  }

  Quiz QuizDao::selectByGuid(const QString &guid) {
    QSqlQuery query(connection());
    query.prepare("SELECT * FROM quizzes WHERE guid = :guid");
    query.bindValue(":guid", guid);
    query.exec();

    Quiz result;
    if(query.next())
      result = makeQuiz(query.record());
    return result;
  }

} // namespace shiken
