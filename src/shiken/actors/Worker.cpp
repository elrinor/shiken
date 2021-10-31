#include "Worker.h"
#include <QNetworkProxy>
#include <QNetworkReply>
#include <shiken/utility/Log.h>

namespace shiken {
  void WorkerBase::criticalConnectionError(QNetworkReply *reply) {
    QString additionalInfo;

    /* Get the proxy that's actually used. */
    QNetworkProxy proxy = reply->manager()->proxy();
    if(proxy.type() == QNetworkProxy::DefaultProxy) {
      if(reply->manager()->proxyFactory() != NULL) {
        QList<QNetworkProxy> proxies = reply->manager()->proxyFactory()->queryProxy(reply->url());
        if(!proxies.empty())
          proxy = proxies[0];
        else
          proxy = QNetworkProxy(QNetworkProxy::NoProxy);
      } else {
        proxy = QNetworkProxy::applicationProxy();
      }
    }

    if(proxy.type() == QNetworkProxy::NoProxy)
      additionalInfo = QS("Прокси-сервер не используется");
    else
      additionalInfo = QS("Используется прокси-сервер %1:%2").arg(proxy.hostName()).arg(proxy.port());
    
    QString reason;
    if(reply->error() != QNetworkReply::NoError)
      reason = QS("Причина: %1.").arg(reply->errorString());
    
    SHIKEN_LOG_MESSAGE(
      QString("Critical connection error %1 (%2) while using proxy server %3:%4").
        arg(reply->error()).
        arg(reply->errorString()).
        arg(proxy.hostName()).
        arg(proxy.port())
    );

    emit critical(QS("Невозможно установить соединение с сервером. %1").arg(reason), additionalInfo);
  }

  void WorkerBase::criticalInvalidReplyFormat() {
    emit critical(QS("Неправильный формат ответа от сервера."));
  }

} // namespace shiken
