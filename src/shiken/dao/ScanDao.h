#ifndef SHIKEN_SCAN_DAO_H
#define SHIKEN_SCAN_DAO_H

#include <shiken/config.h>
#include <QSet>
#include <shiken/entities/Scan.h>
#include "Dao.h"

namespace shiken {
// -------------------------------------------------------------------------- //
// ScanDao
// -------------------------------------------------------------------------- //
  /**
   * Data access object for scans.
   */
  class ScanDao: public Dao {
  public:
    bool insert(const Scan &scan);
    bool update(const Scan &scan);
    bool remove(const Scan &scan);
    bool removeByQuizIdAndStates(int quizId, const QSet<Scan::State> &states);
    Scan selectByHashAndQuizId(const QString &hash, int quizId);
    Scan selectByFileNameAndQuizId(const QString &fileName, int quizId);
    QList<Scan> selectByPageId(int pageId);
    QList<Scan> selectByUserId(int userId);
    QList<Scan> selectByQuizIdAndStates(int quizId, const QSet<Scan::State> &states);

  protected:
    friend class DataAccessDriver;

    ScanDao(DataAccessDriver *dataAccessDriver, QSqlDatabase connection): Dao(dataAccessDriver, connection) {}
  };

} // namespace shiken

#endif // SHIKEN_SCAN_DAO_H
