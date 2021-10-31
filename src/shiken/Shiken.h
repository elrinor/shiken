#ifndef SHIKEN_SHIKEN_H
#define SHIKEN_SHIKEN_H

#include <shiken/config.h>
#include <QObject>
#include <QScopedPointer>
#include <shiken/entities/Quiz.h>
#include <shiken/entities/UsersReply.h>

class QNetworkProxy;
class QAuthenticator;

namespace shiken {
  class DataAccessDriver;
  class ConnectionManager;

// -------------------------------------------------------------------------- //
// Shiken
// -------------------------------------------------------------------------- //
  /**
   * Application context class.
   *
   * Also contains some business logic.
   */
  class Shiken: private QObject {
    Q_OBJECT;
  public:
    /**
     * @param model                    Data manager object.
     */
    Shiken(DataAccessDriver *model);

    ~Shiken();

    /**
     * @returns                        Global data manager object.
     */
    DataAccessDriver *model() const {
      return mModel;
    }

    /**
     * @returns                        Global connection manager object.
     */
    ConnectionManager *connectionManager() const {
      return mConnectionManager.data();
    }

    /**
     * Stores quiz description in a database.
     *
     * @param usersReply               Parsed quiz description.
     * @returns                        Actual quiz from the database.
     */
    Quiz storeQuizData(const UsersReply &usersReply);

    /**
     * Parses xml quiz description.
     * 
     * @param data                     Raw xml data.
     * @param fromFile                 Whether the data was supplied by the user from a file.
     * @returns                        Parsed quiz description.
     */
    UsersReply parseQuizXml(const QByteArray &data, bool fromFile);

  private:
    QScopedPointer<ConnectionManager> mConnectionManager;
    DataAccessDriver *mModel;
  };

} // namespace shiken

#endif // SHIKEN_SHIKEN_H
