#ifndef SHIKEN_PAGE_DAO_H
#define SHIKEN_PAGE_DAO_H

#include <shiken/config.h>
#include <shiken/entities/Page.h>
#include "Dao.h"

namespace shiken {
// -------------------------------------------------------------------------- //
// PageDao
// -------------------------------------------------------------------------- //
  /**
   * Data access objects for pages.
   */
  class PageDao: public Dao {
  public:
    bool insert(Page &page);
    bool update(const Page &page);
    QList<Page> selectByUserId(int userId);
    Page selectByUserIdAndPrintTime(int userId, const QDateTime &printTime);

  protected:
    friend class DataAccessDriver;

    PageDao(DataAccessDriver *dataAccessDriver, QSqlDatabase connection): Dao(dataAccessDriver, connection) {}
  };

} // namespace shiken

#endif // SHIKEN_PAGE_DAO_H
