#ifndef SHIKEN_CONNECTION_MANAGER_H
#define SHIKEN_CONNECTION_MANAGER_H

#include <shiken/config.h>
#include <QObject>
#include <QSharedPointer>
#include <QReadWriteLock>
#include <QThreadStorage>
#include <QSet>
#include <QUrl>
#include "PostData.h"

class QNetworkReply;
class QNetworkAccessManager;
class QNetworkProxy;
class QAuthenticator;

namespace shiken {
  class ConnectionInfo;
  class Shiken;

// -------------------------------------------------------------------------- //
// ConnectionManager
// -------------------------------------------------------------------------- //
  /**
   * This class manages internet connections and offers convenient methods
   * for adding new proxies and sending HTTP post requests.
   */
  class ConnectionManager: public QObject {
    Q_OBJECT;
  public:
    /**
     * Constructor.
     *
     * Starts proxy tests for empty proxy (direct connection) and system-wide 
     * default proxy (if any).
     *
     * @param ctx                      Application context.
     */
    ConnectionManager(Shiken *ctx);

    ~ConnectionManager();

    /**
     * Posts the given request to the given URL.
     *
     * @note This function is thread-safe.
     *
     * @param url                      Target URL.
     * @param postData                 Data to post.
     * @param waitForFullReply         If true, this function will run an 
     *                                 internal event loop until the reply is 
     *                                 downloaded. If false, it will return
     *                                 immediately.
     * @returns                        Network reply object. Deleting this
     *                                 object is a caller's responsibility.
     */
    QNetworkReply *post(const QUrl &url, const PostData &postData, bool waitForFullReply = true);

    /**
     * @note This function is thread-safe.
     *
     * @returns                        Network access manager for the current
     *                                 thread.
     */
    QNetworkAccessManager *networkAccessManager();

    /**
     * Adds proxy for connectivity testing.
     *
     * @note This function is thread-safe.
     *
     * @param proxy                    Proxy to add.
     */
    void testProxy(const QNetworkProxy &proxy);

    /**
     * Shows a proxy login/password request dialog for a given proxy and
     * saves the given login/password in settings.
     *
     * The given proxy object is not modified.
     *
     * @note This function is thread-safe.
     *
     * @param proxy                    Proxy to get login/password for.
     */
    void requestProxyAuthentication(const QNetworkProxy &proxy);

  protected slots:
    /**
     * Displays proxy authentication dialog if it is needed and fills
     * the given authenticator correspondingly.
     */
    void onProxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);

  private:
    /** Application context. */
    Shiken *mCtx;

    /** State shared between ConnectionManager, ConnectionTester's, and 
     * proxy factories. Testers may still use it after manager destruction,
     * that's why it wrapped in a shared pointer. */
    QSharedPointer<ConnectionInfo> mInfo;

    /** Storage for per-thread network access managers. */
    QThreadStorage<QNetworkAccessManager *> mNetworkAccessManagerStorage;
  };

} // namespace shiken

#endif // SHIKEN_CONNECTION_MANAGER_H
