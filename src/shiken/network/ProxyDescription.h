#ifndef SHIKEN_PROXY_DESCRIPTION_H
#define SHIKEN_PROXY_DESCRIPTION_H

#include <shiken/config.h>
#include <QNetworkProxy>
#include <QString>
#include <QHash>
#include <QDataStream>

namespace shiken {
// -------------------------------------------------------------------------- //
// ProxyDescription
// -------------------------------------------------------------------------- //
  /**
   * Description of a proxy server.
   */
  class ProxyDescription {
  public:
    /**
     * Default constructor.
     *
     * Constructs an empty proxy server description (i.e. direct internet
     * connection).
     */
    ProxyDescription(): mType(QNetworkProxy::NoProxy), mPort(0) {}

    /**
     * Constructor.
     *
     * @param type                     Type of the proxy server.
     * @param address                  Address of the proxy server.
     * @param port                     Port of the proxy server.
     */
    ProxyDescription(QNetworkProxy::ProxyType type, const QString &address, int port):
      mType(type), mAddress(address), mPort(port) {}

    /**
     * Constructor.
     *
     * @param proxy                    QNetworkProxy object to construct this 
     *                                 proxy description from.
     */
    ProxyDescription(const QNetworkProxy &proxy): mType(proxy.type()), mAddress(proxy.hostName()), mPort(proxy.port()) {}

    /**
     * @returns                        Type of the proxy server.
     */
    QNetworkProxy::ProxyType type() const {
      return mType;
    }

    /**
     * @returns                        Address of the proxy server.
     */
    const QString &address() const {
      return mAddress;
    }

    /**
     * @returns                        Port of the proxy server.
     */
    int port() const {
      return mPort;
    }

    friend uint qHash(const ProxyDescription &other) {
      return other.mType ^ qHash(other.mAddress) ^ other.mPort;
    }

    bool operator==(const ProxyDescription &other) const {
      return mType == other.mType && mAddress == other.mAddress && mPort == other.mPort;
    }

    QString toString() const {
      return mAddress + ":" + QString::number(mPort) + "(" + QString::number(mType) + ")";
    }

    friend QDataStream &operator<<(QDataStream &stream, const ProxyDescription &other) {
      return stream << static_cast<int>(other.mType) << other.mAddress << other.mPort;
    }

    friend QDataStream &operator>>(QDataStream &stream, ProxyDescription &other) {
      int type;
      stream >> type >> other.mAddress >> other.mPort;
      other.mType = static_cast<QNetworkProxy::ProxyType>(type);
      return stream;
    }

  private:
    QNetworkProxy::ProxyType mType;
    QString mAddress;
    int mPort;
  };

} // namespace shiken

#endif // SHIKEN_PROXY_DESCRIPTION_H
