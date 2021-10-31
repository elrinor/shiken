#ifndef SHIKEN_SCAN_RECOGNIZER_H
#define SHIKEN_SCAN_RECOGNIZER_H

#include <shiken/config.h>
#include <QList>
#include <shiken/entities/Scan.h>
#include <shiken/entities/Page.h>
#include "Worker.h"

namespace shiken {
  class Shiken;

// -------------------------------------------------------------------------- //
// ScanRecognizer
// -------------------------------------------------------------------------- //
  /**
   * This worker recognizes given scans.
   */
  class ScanRecognizer: public Worker<void> {
    Q_OBJECT;
  public:
    ScanRecognizer(Shiken *ctx, const QList<Scan>& scans): 
      Worker<void>(ctx, scans.size()), mScans(scans) {}
    
    virtual void operator()() OVERRIDE;

  Q_SIGNALS:
    void scanRecognized(Scan scan, QString barcode);

  private:
    const QList<Scan> mScans;
  };
  
} // namespace shiken

#endif // SHIKEN_SCAN_RECOGNIZER_H
