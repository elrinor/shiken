#ifndef SHIKEN_CONNECTION_TESTER_H
#define SHIKEN_CONNECTION_TESTER_H

#include <shiken/config.h>
#include <QObject>
#include <QNetworkProxy>
#include <QSharedPointer>
#include "ConnectionInfo.h"

namespace shiken {
  class Shiken;
  class ConnectionInfo;

// -------------------------------------------------------------------------- //
// ConnectionTester
// -------------------------------------------------------------------------- //
  /**
   * ConnectionTester class tests whether internet connection through the given 
   * proxy is available.
   */
  class ConnectionTester: public QObject {
    Q_OBJECT;
  public:
    /**
     * Constructor.
     * 
     * @param ctx                      Application context.
     * @param info                     Shared connection state.
     * @param proxy                    Proxy to test.
     */
    ConnectionTester(Shiken *ctx, const QSharedPointer<ConnectionInfo> &info, const QNetworkProxy &proxy): 
      mCtx(ctx), mInfo(info), mProxy(proxy) {}

  public Q_SLOTS:
    /**
     * Starts the connection testing.
     */
    void start();

  Q_SIGNALS:
    /**
     * This signal is emitted when connection testing is finished.
     */
    void finished();

  private:
    Shiken *mCtx;
    QSharedPointer<ConnectionInfo> mInfo;
    QNetworkProxy mProxy;
  };

} // namespace shiken

#endif // SHIKEN_CONNECTION_TESTER_H
