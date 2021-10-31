#ifndef SHIKEN_UPDATE_DOWNLOADER_H
#define SHIKEN_UPDATE_DOWNLOADER_H

#include <shiken/config.h>
#include <QByteArray>
#include <QUrl>
#include "Worker.h"

namespace shiken {
  class Shiken;

// -------------------------------------------------------------------------- //
// UpdateDownloader
// -------------------------------------------------------------------------- //
  /**
   * This worker downloads binary of the latest shiken version from the server.
   */
  class UpdateDownloader: public Worker<QByteArray> {
    Q_OBJECT;
  public:
    UpdateDownloader(Shiken *ctx, const QUrl &url):
      Worker<QByteArray>(ctx, -1), mUrl(url), mStartedEmitted(false), mLastProgress(0) {}

    virtual QByteArray operator()() OVERRIDE;

  protected Q_SLOTS:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

  private:
    QUrl mUrl;
    bool mStartedEmitted;
    qint64 mLastProgress;
  };

} // namespace shiken

#endif // SHIKEN_UPDATE_DOWNLOADER_H
