#include "SettingsDao.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDataStream>
#include <QVariant>
#include <arx/Foreach.h>
#include <arx/ext/qt/Range.h>
#include <shiken/utility/Log.h>

namespace shiken {
  QString SettingsDao::value(const QString &key) {
    QSqlQuery query(connection());
    query.prepare("SELECT value FROM settings WHERE key = :key");
    query.bindValue(":key", key);
    query.exec();
    
    QString result;
    if(query.next())
      result = query.value(0).toString();
    return result;
  }

  QString SettingsDao::value(const QString &key, const QString &defaultValue) {
    QString result = value(key);
    if(!result.isNull())
      return result;
    
    setValue(key, defaultValue);
    return defaultValue;
  }

  bool SettingsDao::setValue(const QString &key, const QString &value) {
    QSqlQuery query(connection());
    
    query.prepare("INSERT INTO settings(key, value) VALUES(:key, :value)");
    query.bindValue(":key", key);
    query.bindValue(":value", value);
    if(query.exec())
      return true;

    query.prepare("UPDATE settings SET value = :value WHERE key = :key");
    query.bindValue(":key", key);
    query.bindValue(":value", value);
    return query.exec();
  }

  QHash<QString, QString> SettingsDao::dump() {
    QSqlQuery query(connection());
    query.prepare(QString("SELECT * FROM settings WHERE key != '%1'").arg(SHIKEN_DB_VERSION_KEY));
    query.exec();

    QHash<QString, QString> result;
    while(query.next())
      result.insert(query.record().value("key").toString(), query.record().value("value").toString());
    return result;
  }

  void SettingsDao::import(const QHash<QString, QString> &dump) {
    QWriteLocker locker(&mLock);

    map_foreach(const QString &key, const QString &value, dump)
      setValue(key, value);

    sync();
  }

  QNetworkProxy SettingsDao::proxy() const {
    QReadLocker locker(&mLock);

    return QNetworkProxy(
      mProxyDesc.type(), 
      mProxyDesc.address(), 
      mProxyDesc.port(),
      mProxyInfo.value(mProxyDesc).login,
      mProxyInfo.value(mProxyDesc).password
    );
  }

  void SettingsDao::setProxy(const QNetworkProxy &proxy) {
    SHIKEN_LOG_MESSAGE(QString("setProxy(%1:%2)").arg(proxy.hostName()).arg(proxy.port()));

    QWriteLocker locker(&mLock);

    mProxyDesc = proxy;

    if(!proxy.user().isEmpty()) /* Proxy password may be empty. */
      setProxyLoginPassword(proxy, proxy.user(), proxy.password());
  }

  QNetworkProxy SettingsDao::userProxy() const {
    QReadLocker locker(&mLock);

    return QNetworkProxy(
      mUserProxyDesc.type(), 
      mUserProxyDesc.address(), 
      mUserProxyDesc.port(),
      mProxyInfo.value(mUserProxyDesc).login,
      mProxyInfo.value(mUserProxyDesc).password
    );
  }

  void SettingsDao::setUserProxy(const QNetworkProxy &userProxy) {
    QWriteLocker locker(&mLock);

    mUserProxyDesc = userProxy;
      
    setValue(SHIKEN_USER_PROXY_TYPE_KEY,     QString::number(userProxy.type()));
    setValue(SHIKEN_USER_PROXY_ADDRESS_KEY,  userProxy.hostName());
    setValue(SHIKEN_USER_PROXY_PORT_KEY,     QString::number(userProxy.port()));

    if(!userProxy.user().isEmpty()) /* Password may be empty. */
      setProxyLoginPassword(mUserProxyDesc, userProxy.user(), userProxy.password());
  }
    

  void SettingsDao::ensureProxyPresent(const ProxyDescription &desc) {
    assert(mLock.tryLockForWrite() == false);

    if(!mProxyInfo.contains(desc)) {
      mProxyInfo[desc] = ProxyInfo();
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << mProxyInfo.keys();
      setValue(SHIKEN_PROXIES_KEY, buffer.toBase64());
    }
  }
  
  QString SettingsDao::proxyValue(const QString &key, const ProxyDescription &desc) {
    return value(key + "_" + desc.toString());
  }

  void SettingsDao::setProxyValue(const QString &key, const ProxyDescription &desc, const QString &value) {
    setValue(key + "_" + desc.toString(), value);
  }

  void SettingsDao::sync() {
    mTargetUrl          = value(SHIKEN_TARGET_URL_KEY,            SHIKEN_DEFAULT_TARGET_URL);
    mHelpUrl            = value(SHIKEN_HELP_URL_KEY,              SHIKEN_DEFAULT_HELP_URL);
    mBinaryUrl          = value(SHIKEN_BINARY_URL_KEY,            SHIKEN_DEFAULT_BINARY_URL);
    mLogin              = value(SHIKEN_LOGIN_KEY);
    mPageCount          = value(SHIKEN_PAGE_COUNT_KEY,            QString::number(SHIKEN_DEFAULT_PAGE_COUNT)).toInt();
    mDbVersion          = value(SHIKEN_DB_VERSION_KEY,            SHIKEN_DB_VERSION);
    mMaxKeyImageWidth   = value(SHIKEN_MAX_KEY_IMAGE_WIDTH_KEY,   QString::number(SHIKEN_DEFAULT_MAX_KEY_IMAGE_WIDTH)).toInt();
    mMaxKeyImageHeight  = value(SHIKEN_MAX_KEY_IMAGE_HEIGHT_KEY,  QString::number(SHIKEN_DEFAULT_MAX_KEY_IMAGE_HEIGHT)).toInt();
    mMaxRansacError     = value(SHIKEN_MAX_RANSAC_ERROR_KEY,      QString::number(SHIKEN_DEFAULT_MAX_RANSAC_ERROR)).toDouble();
    mScansUpdateIntervalMsecs = value(SHIKEN_SCANS_UPDATE_INTERVAL_MSECS_KEY, QString::number(SHIKEN_DEFAULT_SCANS_UPDATE_INTERVAL_MSECS)).toInt();
    mUserProxyDesc      = 
      ProxyDescription(
        static_cast<QNetworkProxy::ProxyType>(value(SHIKEN_USER_PROXY_TYPE_KEY).toInt()),
        value(SHIKEN_USER_PROXY_ADDRESS_KEY),
        value(SHIKEN_USER_PROXY_PORT_KEY).toInt()
      );

    QList<ProxyDescription> proxyDescriptions;
    QDataStream dataStream(QByteArray::fromBase64(value(SHIKEN_PROXIES_KEY).toAscii()));
    dataStream >> proxyDescriptions;
    foreach(const ProxyDescription &desc, proxyDescriptions) {
      ProxyInfo &proxyInfo = mProxyInfo[desc];
      proxyInfo.login    = proxyValue(SHIKEN_PROXY_LOGIN_KEY,    desc);
      proxyInfo.password = proxyValue(SHIKEN_PROXY_PASSWORD_KEY, desc);
    }
  }

  SettingsDao::SettingsDao(DataAccessDriver *dataAccessDriver, QSqlDatabase connection): Dao(dataAccessDriver, connection) {
    sync();

    mQuizId = 0;
    mSingleUser = static_cast<bool>(SHIKEN_SINGLE_USER);
  }


} // namespace shiken
