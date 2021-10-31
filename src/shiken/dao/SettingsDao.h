#ifndef SHIKEN_SETTINGS_DAO_H
#define SHIKEN_SETTINGS_DAO_H

#include <shiken/config.h>
#include <cassert>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QHash>
#include <QString>
#include <shiken/network/ProxyDescription.h>
#include "Dao.h"

namespace shiken {
// -------------------------------------------------------------------------- //
// SettingsDao
// -------------------------------------------------------------------------- //
  /**
   * Data access object for application-wide settings.
   *
   * @note This class is thread-safe.
   */
  class SettingsDao: public Dao {
  public:
    /**
     * @returns                        Url of the web service.
     */
    QString targetUrl() const {
      QReadLocker locker(&mLock);

      return mTargetUrl;
    }

    /**
     * @returns                        Url of the documentation.
     */
    QString helpUrl() const {
      QReadLocker locker(&mLock);

      return mHelpUrl;
    }

    /**
     * @returns                        Url of the updated binary.
     */
    QString binaryUrl() const {
      QReadLocker locker(&mLock);

      return mBinaryUrl;
    }

    /**
     * @returns                        School login.
     */
    QString login() const {
      QReadLocker locker(&mLock);

      return mLogin;
    }

    /**
     * @param login                    New school login.
     */ 
    void setLogin(const QString &login) {
      QWriteLocker locker(&mLock);

      mLogin = login;
      setValue(SHIKEN_LOGIN_KEY, login);
    }

    /**
     * @returns                        School password.
     */
    QString password() const {
      QReadLocker locker(&mLock);

      return mPassword;
    }

    /**
     * @param password                 New school password.
     */
    void setPassword(const QString &password) {
      QWriteLocker locker(&mLock);

      mPassword = password;
      /* Not saved to database. */
    }

    /**
     * @returns                        Proxy to be used for all outgoing 
     *                                 connections.
     */
    QNetworkProxy proxy() const;

    /**
     * @param desc                     Description of the proxy to be used
     *                                 for all outgoing connections.
     */
    void setProxy(const QNetworkProxy &proxy);

    /**
     * @returns                        Proxy specified by the user.
     */
    QNetworkProxy userProxy() const;

    /**
     * @param userProxy                Proxy specified by user.
     */ 
    void setUserProxy(const QNetworkProxy &userProxy);
    
    /**
     * @param desc                     Proxy description.
     * @returns                        Whether the given proxy requires 
     *                                 authentication.
     */
    bool isProxyAuthenticationRequired(const ProxyDescription &desc) const {
      QReadLocker locker(&mLock);

      return mProxyInfo.value(desc).authenticationRequired;
    }

    /**
     * @param desc                     Proxy description.
     * @param authenticationRequired   Whether the given proxy requires 
     *                                 authentication.
     */
    void setProxyAuthenticationRequired(const ProxyDescription &desc, bool authenticationRequired) {
      QWriteLocker locker(&mLock);

      ensureProxyPresent(desc);

      mProxyInfo[desc].authenticationRequired = authenticationRequired;
    }

    /**
     * @param desc                     Proxy description.
     * @returns                        Login for the given proxy.
     */
    QString proxyLogin(const ProxyDescription &desc) const {
      QReadLocker locker(&mLock);

      return mProxyInfo.value(desc).login;
    }

    /**
     * @param desc                     Proxy description.
     * @param proxyLogin               Login for the given proxy.
     */
    void setProxyLogin(const ProxyDescription &desc, const QString &proxyLogin) {
      QWriteLocker locker(&mLock);

      ensureProxyPresent(desc);

      mProxyInfo[desc].login = proxyLogin;
      setProxyValue(SHIKEN_PROXY_LOGIN_KEY, desc, proxyLogin);
    }

    /**
     * @param desc                     Proxy description.
     * @returns                        Password for the given proxy.
     */
    QString proxyPassword(const ProxyDescription &desc) const {
      QReadLocker locker(&mLock);

      return mProxyInfo[desc].password;
    }

    /**
     * @param desc                     Proxy description.
     * @param proxyPassword            Password for the given proxy.
     */
    void setProxyPassword(const ProxyDescription &desc, const QString &proxyPassword) {
      QWriteLocker locker(&mLock);

      ensureProxyPresent(desc);

      mProxyInfo[desc].password = proxyPassword;
      setProxyValue(SHIKEN_PROXY_PASSWORD_KEY, desc, proxyPassword);
    }

    /**
     * @returns                        Identifier of the current quiz.
     */
    int quizId() const {
      QReadLocker locker(&mLock);

      return mQuizId;
    }

    /**
     * @param quizId                   Identifier of the current quiz.
     */
    void setQuizId(int quizId) {
      QWriteLocker locker(&mLock);

      mQuizId = quizId;
    }

    /**
     * @returns                        Whether shiken is running in a 
     *                                 single-user mode.
     */
    bool isSingleUser() const {
      return mSingleUser;
    }

    /**
     * @param singleUser               Whether shiken is running in a 
     *                                 single-user mode.
     */
    void setSingleUser(bool singleUser) {
      mSingleUser = singleUser;
    }

    /**
     * @returns                        Version of the database.
     */
    QString dbVersion() const {
      QReadLocker locker(&mLock);

      return mDbVersion;
    }

    /**
     * @returns                        Default page count to be printed per
     *                                 student.
     */
    int pageCount() const {
      QReadLocker locker(&mLock);

      return mPageCount;
    }

    /**
     * @returns                        Maximal width of an image used for 
     *                                 keypoint extraction.
     */
    int maxKeyImageWidth() const {
      QReadLocker locker(&mLock);

      return mMaxKeyImageWidth;
    }

    /**
     * @returns                        Maximal height of an image used for 
     *                                 keypoint extraction.
     */
    int maxKeyImageHeight() const {
      QReadLocker locker(&mLock);

      return mMaxKeyImageHeight;
    }

    /**
     * @returns                        Maximal RANSAC error for match filtering.
     */
    double maxRansacError() const {
      QReadLocker locker(&mLock);

      return mMaxRansacError;
    }

    /**
     * @returns                        Interval in msecs between requests for
     *                                 scans on server. 
     */
    int scansUpdateIntervalMsecs() const {
      QReadLocker locker(&mLock);

      return mScansUpdateIntervalMsecs;
    }

    /**
     * @returns                        QHash containing all settings.
     */
    QHash<QString, QString> dump();

    /**
     * Imports previously exported settings.
     * 
     * @param dump                     Settings dump acquired with a call to
     *                                 dump().
     */
    void import(const QHash<QString, QString> &dump);

  protected:
    friend class DataAccessDriver;

    void sync();

    QString value(const QString &key);

    QString value(const QString &key, const QString &defaultValue);

    bool setValue(const QString &key, const QString &value);

    void ensureProxyPresent(const ProxyDescription &desc);

    QString proxyValue(const QString &key, const ProxyDescription &desc);

    void setProxyValue(const QString &key, const ProxyDescription &desc, const QString &value);

    SettingsDao(DataAccessDriver *dataAccessDriver, QSqlDatabase connection);

    void setProxyLoginPassword(const ProxyDescription &desc, const QString &login, const QString &password) {
      assert(mLock.tryLockForWrite() == false);

      ProxyInfo &userProxyInfo = mProxyInfo[desc];
      userProxyInfo.login = login;
      userProxyInfo.password = password;

      setProxyValue(SHIKEN_PROXY_LOGIN_KEY,    desc, login);
      setProxyValue(SHIKEN_PROXY_PASSWORD_KEY, desc, password);
    }

  private:
    struct ProxyInfo {
      ProxyInfo(): authenticationRequired(false) {}

      bool authenticationRequired;
      QString login;
      QString password;
    };

    mutable QReadWriteLock mLock;

    QString mTargetUrl, mHelpUrl, mBinaryUrl, mLogin, mPassword, mDbVersion;
    int mQuizId, mPageCount, mMaxKeyImageWidth, mMaxKeyImageHeight, mScansUpdateIntervalMsecs;
    double mMaxRansacError;
    ProxyDescription mUserProxyDesc, mProxyDesc;
    QHash<ProxyDescription, ProxyInfo> mProxyInfo;
    bool mSingleUser;
  };

} // namespace shiken

#endif // SHIKEN_SETTINGS_DAO_H
