#include "UpdateDownloader.h"
#include <QNetworkReply>
#include <QScopedPointer>
#include <QEventLoop>
#include <arx/ScopeExit.h>
#include <shiken/network/ConnectionManager.h>
#include <shiken/utility/Log.h>
#include <shiken/Shiken.h>

namespace shiken {
  QByteArray UpdateDownloader::operator()() {
    ARX_SCOPE_EXIT(&) { Q_EMIT this->finished(); };

    SHIKEN_LOG_MESSAGE("Getting new version.");

    QByteArray result;

    QScopedPointer<QNetworkReply> reply(ctx()->connectionManager()->post(mUrl, PostData(), false));

    QEventLoop eventLoop;
    connect(reply.data(), SIGNAL(finished()), &eventLoop, SLOT(quit()));
    connect(reply.data(), SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));
    if(!reply->isFinished())
      eventLoop.exec();

    if(reply->error() != QNetworkReply::NoError) {
      criticalConnectionError(reply.data());
      return result;
    }

    result = reply->readAll();

    SHIKEN_LOG_MESSAGE("Got new version.");

    return result;
  }

  void UpdateDownloader::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if(bytesTotal > 0) {
      if(!mStartedEmitted) {
        Q_EMIT started(static_cast<int>(bytesTotal));
        mStartedEmitted = true;
      }

      Q_EMIT advanced(bytesReceived - mLastProgress);
      mLastProgress = bytesReceived;
    } else {
      if(!mStartedEmitted) {
        Q_EMIT started(100);
        mStartedEmitted = true;
      }

      Q_EMIT advanced(1);
    }
  }

} // namespace shiken
