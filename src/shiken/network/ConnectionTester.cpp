#include "ConnectionTester.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QScopedPointer>
#include <QEventLoop>
#include <QWriteLocker>
#include <shiken/dao/DataAccessDriver.h>
#include <shiken/dao/SettingsDao.h>
#include <shiken/utility/Log.h>
#include <shiken/Shiken.h>
#include "ConnectionInfo.h"

namespace shiken {

  void ConnectionTester::start() {
    /* Increase active tester count first. */
    {
      QWriteLocker writeLocker(&mInfo->lock);

      mInfo->testersRunning++;
    }

    SHIKEN_LOG_MESSAGE(QString("Testing proxy server %1:%2").arg(mProxy.hostName()).arg(mProxy.port()));

    /* Send HTTP GET request. */
    QNetworkAccessManager http;
    http.setProxy(mProxy);
    QScopedPointer<QNetworkReply> reply(http.get(QNetworkRequest(mCtx->model()->settingsDao()->targetUrl())));

    /* Wait for reply. */
    QEventLoop eventLoop;
    connect(reply.data(), SIGNAL(finished()), &eventLoop, SLOT(quit()));
    if(!reply->isFinished())
      eventLoop.exec();
    
    /* Write results & wake up waiting threads. */
    {
      QWriteLocker writeLocker(&mInfo->lock);
      bool hasConnection = 
        reply->error() == QNetworkReply::NoError ||
        reply->error() == QNetworkReply::ProxyAuthenticationRequiredError;

      /* TODO: maybe also ContentOperationNotPermittedError? */

      if(hasConnection) {
        mCtx->model()->settingsDao()->setProxy(mProxy);
        mInfo->hasConnection = true;

        if(reply->error() == QNetworkReply::ProxyAuthenticationRequiredError)
          mCtx->model()->settingsDao()->setProxyAuthenticationRequired(mProxy, true);
      }

      mInfo->testersRunning--;

      if(hasConnection || (!hasConnection && mInfo->testersRunning == 0))
        mInfo->waitCondition.wakeAll();

      SHIKEN_LOG_MESSAGE(
        QString("Proxy server %1:%2 tested: hasConnection = %3, error = %4 (%5)").
          arg(mProxy.hostName()).
          arg(mProxy.port()).
          arg(hasConnection).
          arg(reply->error()).
          arg(reply->errorString())
      );
    }

    Q_EMIT finished();
  }


} // namespace shiken
