#include "ConnectionManager.h"
#include <cassert>
#include <QReadLocker>
#include <QWriteLocker>
#include <QNetworkProxy>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QEventLoop>
#include <QMetaType>
#include <QAuthenticator>
#include <QApplication>
#include <QThread>
#include <arx/Foreach.h>
#include <arx/StaticBlock.h>
#include <shiken/dao/DataAccessDriver.h>
#include <shiken/dao/SettingsDao.h>
#include <shiken/ui/LoginDialog.h>
#include <shiken/utility/Log.h>
#include <shiken/Shiken.h>
#include "ConnectionInfo.h"
#include "ConnectionTester.h"
#include "ProxyAuthenticationRequester.h"

ARX_STATIC_BLOCK(SHIKEN_CPP) {
  qRegisterMetaType<QNetworkProxy>();
  qRegisterMetaType<QAuthenticator *>();
}

Q_DECLARE_METATYPE(QNetworkProxy);
Q_DECLARE_METATYPE(QAuthenticator *);

namespace shiken {
  namespace {
    /** 
     * Proxy factory class that takes proxy from the global settings.
     * 
     * If internet connection is not available, it will wait for 
     * ConnectionTester's to finish.
     */
    class ShikenNetworkProxyFactory: public QNetworkProxyFactory {
    public:
      ShikenNetworkProxyFactory(Shiken *ctx, const QSharedPointer<ConnectionInfo> &info): mCtx(ctx), mInfo(info) {}

      virtual QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery & /*query*/) OVERRIDE {
        QList<QNetworkProxy> result;

        bool hasConnection;
        {
          QReadLocker readLocker(&mInfo->lock);
          if(!mInfo->hasConnection && mInfo->testersRunning > 0)
            mInfo->waitCondition.wait(&mInfo->lock);

          assert(mInfo->hasConnection || mInfo->testersRunning == 0);

          hasConnection = mInfo->hasConnection;
        }

        if(hasConnection) {
          QNetworkProxy proxy;
          while(true) {
            proxy = mCtx->model()->settingsDao()->proxy();

            if(!mCtx->model()->settingsDao()->isProxyAuthenticationRequired(proxy) || !proxy.user().isEmpty())
              break;
            
            /* Proxy has already been tested and requires authentication. 
             * Show authentication dialog. */
            mCtx->connectionManager()->requestProxyAuthentication(proxy);
          }

          result.push_back(proxy);
        } else {
          /* Use user-specified proxy if there seems to be no internet 
           * connection. */
          result.push_back(mCtx->model()->settingsDao()->userProxy());
        }
        return result;
      }

    private:
      Shiken *mCtx;
      QSharedPointer<ConnectionInfo> mInfo;
    };

  } // namespace `anonymous-namespace`


  ConnectionManager::ConnectionManager(Shiken *ctx): 
    mCtx(ctx), 
    mInfo(new ConnectionInfo())
  {
    assert(thread() == QApplication::instance()->thread()); /* ConnectionManager object must be created in the main thread. */

    SettingsDao *settings = mCtx->model()->settingsDao();

    /* Test direct connection. */
    testProxy(QNetworkProxy(QNetworkProxy::NoProxy));

    /* Test all system proxies. */
    foreach(const QNetworkProxy &proxy, QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(QUrl(settings->targetUrl()))))
      if(proxy.type() != QNetworkProxy::DefaultProxy && proxy.type() != QNetworkProxy::NoProxy)
        testProxy(proxy);

    /* Test user proxy, if any. */
    QNetworkProxy userProxy = settings->userProxy();
    if(userProxy.type() != QNetworkProxy::NoProxy)
      testProxy(userProxy);
  }

  void ConnectionManager::testProxy(const QNetworkProxy &proxy) {
    QThread *testerThread = new QThread();
    
    ConnectionTester *tester = new ConnectionTester(mCtx, mInfo, proxy);
    tester->moveToThread(testerThread);
    
    connect(testerThread, SIGNAL(started()), tester, SLOT(start()));
    connect(tester, SIGNAL(finished()), tester, SLOT(deleteLater()));
    connect(tester, SIGNAL(destroyed()), testerThread, SLOT(quit()));
    connect(testerThread, SIGNAL(finished()), testerThread, SLOT(deleteLater()));

    testerThread->start();
  }

  ConnectionManager::~ConnectionManager() {
    return;
  }

  QNetworkAccessManager *ConnectionManager::networkAccessManager() {
    QNetworkAccessManager *result = mNetworkAccessManagerStorage.localData();
    if(result == NULL) {
      result = new QNetworkAccessManager();

      result->setProxyFactory(new ShikenNetworkProxyFactory(mCtx, mInfo));
      connect(
        result, 
        SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), 
        this, 
        SLOT(onProxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)),
        QThread::currentThread() == thread() ? Qt::DirectConnection : Qt::BlockingQueuedConnection
      );

      /* QThreadStorage takes ownership of the result. It will be deleted upon
       * thread termination (NOT upon destruction of QThreadStorage). */
      mNetworkAccessManagerStorage.setLocalData(result);
    }

    return result;
  }

  QNetworkReply *ConnectionManager::post(const QUrl &url, const PostData &postData, bool waitForFullReply) {
    QByteArray crlf = "\r\n";

    QByteArray boundary = "---------------------------" + QByteArray::number(qrand()) + QByteArray::number(qrand()) + QByteArray::number(qrand());
    
    QString startBoundary = "--" + boundary + crlf;
    QString midBoundary = crlf + "--" + boundary + crlf;
    QString endBoundary = crlf + "--" + boundary + "--" + crlf;
    
    QByteArray buffer;
    int count = 0;

    foreach(const PostData::Field &field, postData.fields()) {
      buffer.append(count == 0 ? startBoundary : midBoundary);
      buffer.append("Content-Disposition: form-data; name=\"").append(field.key()).append("\"").append(crlf);
      buffer.append("Content-Transfer-Encoding: 8bit").append(crlf);
      buffer.append(crlf);
      buffer.append(field.value().toUtf8());

      count++;
    }

    SHIKEN_LOG_MESSAGE("Posting data (files skipped) to " << url);
    SHIKEN_LOG_MESSAGE(buffer);

    foreach(const PostData::File &file, postData.files()) {
      buffer.append(count == 0 ? startBoundary : midBoundary);

      buffer.append("Content-Disposition: form-data; name=\"").append(file.key()).append("\"; filename=\"").append(file.fileName()).append("\"").append(crlf);
      buffer.append("Content-Type: ").append(file.mime()).append(crlf).append(crlf);
      buffer.append(file.data());  

      count++;
    }

    buffer.append(endBoundary.toAscii());

    QNetworkRequest request;
    request.setRawHeader("Host", url.host().toAscii());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + boundary);
    request.setHeader(QNetworkRequest::ContentLengthHeader, buffer.size());
    request.setUrl(url);

    QNetworkReply *reply = networkAccessManager()->post(request, buffer);

    if(waitForFullReply) {
      QEventLoop eventLoop;
      connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
      if(!reply->isFinished())
        eventLoop.exec();
    }

    return reply;
  }

  void ConnectionManager::requestProxyAuthentication(const QNetworkProxy &proxy) {
    /* Qt documentation says:
     *
     * QObject and all of its subclasses are not thread-safe. This includes the 
     * entire event delivery system.
     *
     * So, to run a slot in a GUI thread we need to first construct an
     * object in the current thread, connect it to the slot in question,
     * and then emit the corresponding signal.
     *
     * QObject::connect is thread-safe, so everything we do here is also
     * thread-safe. */
    ProxyAuthenticationRequester requester;

    connect(
      &requester,
      SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)),
      this,
      SLOT(onProxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)),
      requester.thread() == thread() ? Qt::DirectConnection : Qt::BlockingQueuedConnection
    );

    QAuthenticator authenticator;
    requester.sendProxyAuthenticationRequired(proxy, &authenticator);
  }

  void ConnectionManager::onProxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator) {
    /* This signal must be executed in the object's thread only (which is the 
     * GUI thread, by the way) because we are creating dialogs here. */
    assert(QThread::currentThread() == thread()); 

    SettingsDao *settings = mCtx->model()->settingsDao();
    QString proxyLogin = settings->proxyLogin(proxy);
    QString proxyPassword = settings->proxyPassword(proxy);

    if(proxyLogin.isEmpty() || proxyPassword.isEmpty() || (proxyLogin == authenticator->user() && proxyPassword == authenticator->password())) {
      LoginDialog dlg(QS("Внимание!!!\nВведите логин и пароль для прокси-сервера %1:%2.").arg(proxy.hostName()).arg(proxy.port()));
      dlg.setLogin(mCtx->model()->settingsDao()->proxyLogin(proxy));
      if(dlg.exec() != QDialog::Accepted)
        return;

      proxyLogin = dlg.login();
      proxyPassword = dlg.password();
      settings->setProxyLogin(proxy, proxyLogin);
      settings->setProxyPassword(proxy, proxyPassword);
    }

    authenticator->setUser(proxyLogin);
    authenticator->setPassword(proxyPassword);
  }


} // namespace shiken
