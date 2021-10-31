#include "ScanDao.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <arx/Foreach.h>

namespace shiken {
  namespace {
    void fillQuery(QSqlQuery &query, const Scan &scan) {
      query.bindValue(":quiz_id",   scan.quizId());
      query.bindValue(":file_name", scan.fileName());
      query.bindValue(":hash",      scan.hash());
      query.bindValue(":state",     scan.state());
      query.bindValue(":page_id",   scan.pageId() == 0 ? QVariant() : scan.pageId());
    }

    Scan makeScan(const QSqlRecord &record) {
      Scan scan;
      scan.setQuizId  (record.value("quiz_id").toInt());
      scan.setFileName(record.value("file_name").toString());
      scan.setHash    (record.value("hash"     ).toString());
      scan.setState   (static_cast<Scan::State>(record.value("state"    ).toInt()));
      scan.setPageId  (record.value("page_id"  ).toInt());
      return scan;
    }

    QString setString(const QSet<Scan::State> &states) {
      QString result;
      foreach(Scan::State state, states)
        result += (result.isEmpty() ? "" : ", ") + QString::number(state);
      return result;
    }

  } // namespace `anonymous-namespace`


  bool ScanDao::insert(const Scan &scan) {    
    QSqlQuery query(connection());
    query.prepare("INSERT INTO scans(quiz_id, file_name, hash, state, page_id) VALUES(:quiz_id, :file_name, :hash, :state, :page_id)");
    fillQuery(query, scan);

    bool success = query.exec();
    if(success)
      emitChanged();
    return success;
  }

  bool ScanDao::update(const Scan &scan) {
    QSqlQuery query(connection());
    query.prepare("UPDATE scans SET quiz_id = :quiz_id, file_name = :file_name, state = :state, page_id = :page_id WHERE hash = :hash");
    fillQuery(query, scan);

    bool success = query.exec();
    if(success)
      emitChanged();
    return success;
  }

  bool ScanDao::remove(const Scan &scan) {
    QSqlQuery query(connection()); 
    query.prepare("DELETE FROM scans WHERE hash = :hash");
    query.bindValue(":hash", scan.hash());

    bool success = query.exec();
    if(success)
      emitChanged();
    return success;
  }

  bool ScanDao::removeByQuizIdAndStates(int quizId, const QSet<Scan::State> &states) {
    QSqlQuery query(connection()); 
    query.prepare("DELETE FROM scans WHERE quiz_id = :quiz_id AND state IN (" + setString(states) + ")");
    query.bindValue(":quiz_id", quizId);

    bool success = query.exec();
    if(success)
      emitChanged();
    return success;
  }

  Scan ScanDao::selectByHashAndQuizId(const QString &hash, int quizId) {
    QSqlQuery query(connection());
    query.prepare("SELECT * FROM scans WHERE hash = :hash AND quiz_id = :quiz_id");
    query.bindValue(":hash", hash);
    query.bindValue(":quiz_id", quizId);
    query.exec();

    Scan result;
    if(query.next())
      result = makeScan(query.record());
    return result;
  }

  Scan ScanDao::selectByFileNameAndQuizId(const QString &fileName, int quizId) {
    QSqlQuery query(connection());
    query.prepare("SELECT * FROM scans WHERE file_name = :file_name AND quiz_id = :quiz_id");
    query.bindValue(":file_name", fileName);
    query.bindValue(":quiz_id", quizId);
    query.exec();

    Scan result;
    if(query.next())
      result = makeScan(query.record());
    return result;
  }

  QList<Scan> ScanDao::selectByPageId(int pageId) {
    QSqlQuery query(connection());
    query.prepare("SELECT * FROM scans WHERE page_id = :page_id");
    query.bindValue(":page_id", pageId);
    query.exec();

    QList<Scan> result;
    while(query.next())
      result.push_back(makeScan(query.record()));
    return result;
  }

  QList<Scan> ScanDao::selectByUserId(int userId) {
    QSqlQuery query(connection());
    query.prepare("SELECT * FROM scans NATURAL JOIN pages WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);
    query.exec();

    QList<Scan> result;
    while(query.next())
      result.push_back(makeScan(query.record()));
    return result;
  }

  QList<Scan> ScanDao::selectByQuizIdAndStates(int quizId, const QSet<Scan::State> &states) {
    QSqlQuery query(connection());
    query.prepare("SELECT * FROM scans WHERE quiz_id = :quiz_id AND state IN (" + setString(states) + ")");
    query.bindValue(":quiz_id", quizId);
    query.exec();

    QList<Scan> result;
    while(query.next())
      result.push_back(makeScan(query.record()));
    return result;
  }

} // namespace shiken
