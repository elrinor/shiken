#include "DataAccessDriver.h"
#include <cassert>
#include <QSqlQuery>
#include "QuizDao.h"
#include "SettingsDao.h"
#include "UserDao.h"
#include "ScanDao.h"
#include "PageDao.h"

namespace shiken {
  DataAccessDriver::DataAccessDriver(const QString &databaseName): 
    mQuizDao(NULL), mSettingsDao(NULL), mUserDao(NULL), mPageDao(NULL), mScanDao(NULL), mSignalsBlockLevel(0) 
  {
    assert(!QSqlDatabase::contains(SHIKEN_SQL_CONNECTION_NAME));

    mConnection = QSqlDatabase::addDatabase("QSQLITE", SHIKEN_SQL_CONNECTION_NAME);
    mConnection.setDatabaseName(databaseName);
    mConnection.open();

    QSqlQuery query(mConnection);
    query.exec(" \
      PRAGMA synchronous = OFF \
    "); /* This speeds up inserts significantly. */
    query.exec(" \
      CREATE TABLE IF NOT EXISTS quizzes( \
        quiz_id INTEGER PRIMARY KEY AUTOINCREMENT, \
        disc_id INTEGER, \
        disc_str TEXT, \
        start_time TEXT, \
        duration_minutes INTEGER, \
        submit_end_time TEXT, \
        title TEXT NOT NULL, \
        description TEXT, \
        guid TEXT, \
        qsg_guid TEXT, \
        prj_guid TEXT, \
        is_loaded_from_xml INTEGER \
      ) \
    ");
    query.exec(" \
      CREATE INDEX IF NOT EXISTS guid_index ON quizzes(guid) \
    ");
    query.exec(" \
      CREATE TABLE IF NOT EXISTS settings( \
        key TEXT PRIMARY KEY, \
        value TEXT \
      ) \
    ");
    query.exec(" \
      CREATE TABLE IF NOT EXISTS users ( \
        user_id INTEGER PRIMARY KEY AUTOINCREMENT, \
        compressed_guid TEXT NOT NULL, \
        quiz_id INTEGER NOT NULL, \
        guid TEXT, \
        login TEXT NOT NULL, \
        is_unknown INTEGER, \
        UNIQUE(compressed_guid, quiz_id), \
        FOREIGN KEY(quiz_id) REFERENCES quizzes(quiz_id) \
      ) \
    ");
    query.exec(" \
      CREATE INDEX IF NOT EXISTS login_index ON users(login) \
    ");
    query.exec(" \
      CREATE TABLE IF NOT EXISTS pages ( \
        page_id INTEGER PRIMARY KEY AUTOINCREMENT, \
        user_id INTEGER NOT NULL, \
        print_time TEXT NOT NULL, \
        FOREIGN KEY(user_id) REFERENCES users(user_id), \
        UNIQUE(user_id, print_time) \
      ) \
    ");
    query.exec(" \
      CREATE TABLE IF NOT EXISTS scans ( \
        file_name TEXT NOT NULL, \
        quiz_id INTEGER NOT NULL, \
        hash TEXT NOT NULL, \
        state INTEGER NOT NULL, \
        page_id INTEGER, \
        UNIQUE(file_name, quiz_id), \
        PRIMARY KEY(hash, quiz_id), \
        FOREIGN KEY(page_id) REFERENCES pages(page_id), \
        FOREIGN KEY(quiz_id) REFERENCES quizzes(quiz_id) \
      ) \
    ");


    mQuizDao = new QuizDao(this, mConnection);
    mSettingsDao = new SettingsDao(this, mConnection);
    mUserDao = new UserDao(this, mConnection);
    mPageDao = new PageDao(this, mConnection);
    mScanDao = new ScanDao(this, mConnection);
  }

  DataAccessDriver::~DataAccessDriver() {
    delete mQuizDao;
    delete mSettingsDao;
    delete mUserDao;
    delete mPageDao;
    delete mScanDao;

    mConnection.close();
    QSqlDatabase::removeDatabase(SHIKEN_SQL_CONNECTION_NAME);
  }

  bool DataAccessDriver::signalsBlocked() {
    return mSignalsBlockLevel > 0;
  }

  void DataAccessDriver::blockSignals() {
    mSignalsBlockLevel++;
  }

  void DataAccessDriver::unblockSignals() {
    assert(mSignalsBlockLevel > 0);

    mSignalsBlockLevel--;

    if(mSignalsBlockLevel == 0) {
      mQuizDao->signalsUnblocked();
      mSettingsDao->signalsUnblocked();
      mUserDao->signalsUnblocked();
      mPageDao->signalsUnblocked();
      mScanDao->signalsUnblocked();
    }
  }


} // namespace shiken
