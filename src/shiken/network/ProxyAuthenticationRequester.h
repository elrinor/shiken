#ifndef SHIKEN_PROXY_AUTHENTICATION_REQUESTER_H
#define SHIKEN_PROXY_AUTHENTICATION_REQUESTER_H

#include <shiken/config.h>
#include <QObject>

class QAuthenticator;
class QNetworkProxy;

namespace shiken {
// -------------------------------------------------------------------------- //
// ProxyAuthenticationRequester
// -------------------------------------------------------------------------- //
  /**
   * This is a helper class used for cross-thread slot execution.
   *
   * @see shiken::ConnectionManager
   */
  class ProxyAuthenticationRequester: public QObject {
    Q_OBJECT;
  public:
    /**
     * Emits the proxyAuthenticationRequired signal.
     */
    void sendProxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator) {
      Q_EMIT proxyAuthenticationRequired(proxy, authenticator);
    }

  signals:
    void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);
  };


} // namespace shiken

#endif // SHIKEN_PROXY_AUTHENTICATION_REQUESTER_H
