#ifndef SHIKEN_DATA_ACCESS_DRIVER_H
#define SHIKEN_DATA_ACCESS_DRIVER_H

#include <shiken/config.h>
#include <boost/noncopyable.hpp>
#include <QString>
#include <QSqlDatabase>

namespace shiken {
  class QuizDao;
  class SettingsDao;
  class UserDao;
  class ScanDao;
  class PageDao;

// -------------------------------------------------------------------------- //
// DataAccessDriver
// -------------------------------------------------------------------------- //
  /**
   * DataAccessDriver class performs database manipulation and gives access
   * to Dao classes.
   */
  class DataAccessDriver: public boost::noncopyable {
  public:
    DataAccessDriver(const QString &databaseName);

    ~DataAccessDriver();

    QuizDao *quizDao() const {
      return mQuizDao;
    }

    SettingsDao *settingsDao() const {
      return mSettingsDao;
    }

    UserDao *userDao() const {
      return mUserDao;
    }

    PageDao *pageDao() const {
      return mPageDao;
    }

    ScanDao *scanDao() const {
      return mScanDao;
    }

    QSqlDatabase connection() const {
      return mConnection;
    }

    /**
     * @returns                        Whether signals from all Dao objects
     *                                 are currently blocked.
     */
    bool signalsBlocked();

    /**
     * Blocks signals from all Dao objects.
     */
    void blockSignals();

    /**
     * Unblocks signals from all Dao objects. If there are any signals that 
     * were triggered when blocked, they will be fired.
     */
    void unblockSignals();

  private:
    QSqlDatabase mConnection;
    QuizDao *mQuizDao;
    SettingsDao *mSettingsDao;
    UserDao *mUserDao;
    PageDao *mPageDao;
    ScanDao *mScanDao;
    int mSignalsBlockLevel;
  };

} // namespace shiken

#endif // SHIKEN_DATA_ACCESS_DRIVER_H
