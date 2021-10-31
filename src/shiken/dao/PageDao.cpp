#include "PageDao.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

namespace shiken {
  namespace {
    void fillQuery(QSqlQuery &query, const Page &page, bool usePageId) {
      if(usePageId)
        query.bindValue(":page_id",       page.pageId());
      query.bindValue(":user_id",         page.userId());
      query.bindValue(":print_time",      page.printTime().toString(SHIKEN_DATE_TIME_FORMAT));
    }

    Page makePage(const QSqlRecord &record) {
      Page page;
      page.setPageId        (record.value("page_id").toInt());
      page.setUserId        (record.value("user_id").toInt());
      page.setPrintTime     (QDateTime::fromString(record.value("print_time").toString(), SHIKEN_DATE_TIME_FORMAT));
      return page;
    }

  } // namespace `anonymous-namespace`

  bool PageDao::insert(Page &page) {
    QSqlQuery query(connection());
    query.prepare("INSERT INTO pages(user_id, print_time) VALUES(:user_id, :print_time)");
    fillQuery(query, page, false);

    bool success = query.exec();
    if(success) {
      page.setPageId(query.lastInsertId().toInt());
      emitChanged();
    }
    return success;
  }

  bool PageDao::update(const Page &page) {
    QSqlQuery query(connection());
    query.prepare("UPDATE pages SET user_id = :user_id, print_time = :print_time WHERE page_id = :page_id");
    fillQuery(query, page, true);

    bool success = query.exec();
    if(success)
      emitChanged();
    return success;
  }

  QList<Page> PageDao::selectByUserId(int userId) {
    QSqlQuery query(connection());
    query.prepare("SELECT * FROM pages WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);
    query.exec();

    QList<Page> result;
    while(query.next())
      result.push_back(makePage(query.record()));
    return result;
  }
  
  Page PageDao::selectByUserIdAndPrintTime(int userId, const QDateTime &printTime) {
    QSqlQuery query(connection());
    query.prepare("SELECT * FROM pages WHERE user_id = :user_id AND print_time = :print_time");
    query.bindValue(":user_id", userId);
    query.bindValue(":print_time", printTime.toString(SHIKEN_DATE_TIME_FORMAT));
    query.exec();

    Page result;
    if(query.next())
      result = makePage(query.record());
    return result;
  }

} // namespace shiken
