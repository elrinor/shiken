#ifndef SHIKEN_DAO_H
#define SHIKEN_DAO_H

#include <shiken/config.h>
#include <QObject>
#include <QSqlDatabase>
#include "DataAccessDriver.h"

namespace shiken {
// -------------------------------------------------------------------------- //
// Dao
// -------------------------------------------------------------------------- //
  /**
   * Base class for all data access objects.
   */
  class Dao: public QObject {
    Q_OBJECT;
  Q_SIGNALS:
    /** 
     * This signal is emitted every time the underlying table is changed.
     */
    void changed();

  protected:
    Dao(DataAccessDriver *dataAccessDriver, const QSqlDatabase &connection): 
      mDataAccessDriver(dataAccessDriver), mConnection(connection), mChanged(false) {}

    /**
     * @returns                        Database connection that is used by this
     *                                 data access object.
     */
    QSqlDatabase &connection() {
      return mConnection;
    }

    /**
     * This method is to be called when underlying table is changed.
     *
     * It will emit changed signal if needed.
     */
    void emitChanged() {
      if(mDataAccessDriver->signalsBlocked()) {
        mChanged = true;
      } else {
        Q_EMIT changed();
      }
    }

    /**
     * This method is to be called when signals are unblocked.
     *
     * It will emit changed signal if needed.
     */
    void signalsUnblocked() {
      if(mChanged) {
        Q_EMIT changed();
        mChanged = false;
      }
    }

  private:
    DataAccessDriver *mDataAccessDriver;
    QSqlDatabase mConnection;
    bool mChanged;
  };

} // namespace shiken

#endif // SHIKEN_DAO_H
