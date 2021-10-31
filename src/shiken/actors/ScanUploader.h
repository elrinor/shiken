#ifndef SHIKEN_SCAN_UPLOADER_H
#define SHIKEN_SCAN_UPLOADER_H

#include <shiken/config.h>
#include <QUrl>
#include <QList>
#include <shiken/entities/Quiz.h>
#include <shiken/entities/Scan.h>
#include "Worker.h"

namespace shiken {
  class Shiken;

// -------------------------------------------------------------------------- //
// ScanUploader
// -------------------------------------------------------------------------- //
  /**
   * This worker uploads given scans to the server.
   */
  class ScanUploader: public Worker<void> {
    Q_OBJECT;
  public:
    ScanUploader(Shiken *ctx, const Quiz& quiz, const QList<Scan> &scans, const QUrl &url):
      Worker<void>(ctx, scans.size()), mQuiz(quiz), mScans(scans), mUrl(url) {}

    void operator()();

  Q_SIGNALS:
    void scanUploaded(Scan scan);
    void scanNotFound(Scan scan);

  private:
    const Quiz mQuiz;
    const QList<Scan> mScans;
    QUrl mUrl;
  };

} // namespace shiken

#endif // SHIKEN_SCAN_UPLOADER_H
