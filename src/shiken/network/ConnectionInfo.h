#ifndef SHIKEN_CONNECTION_INFO_H
#define SHIKEN_CONNECTION_INFO_H

#include <shiken/config.h>
#include <QReadWriteLock>
#include <QWaitCondition>

namespace shiken {
  class ConnectionTester;

// -------------------------------------------------------------------------- //
// ConnectionInfo
// -------------------------------------------------------------------------- //
  /**
   * State that is shared between ConnectionManager and ConnectionTester's.
   */
  class ConnectionInfo {
  public:
    ConnectionInfo(): 
      testersRunning(0),
      hasConnection(false)
    {}

    /** Lock for access serialization. */
    QReadWriteLock lock;

    /** Number of ConnectionTester's currently running. */
    int testersRunning;

    /** Wait condition used for waking up connection threads that are waiting
     * for the results of connectivity testing. */
    QWaitCondition waitCondition;

    /** True if an internet connection was found by one of the 
     * ConnectionTesters, false otherwise. */
    bool hasConnection;
  };

} // namespace shiken

#endif // SHIKEN_CONNECTION_INFO_H
