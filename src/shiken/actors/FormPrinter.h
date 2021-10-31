#ifndef SHIKEN_FORM_PRINTER_H
#define SHIKEN_FORM_PRINTER_H

#include <shiken/config.h>
#include <QObject>
#include <QPrinter>
#include <QPainter>
#include "Worker.h"

namespace shiken {
  class Shiken;

// -------------------------------------------------------------------------- //
// FormPrintWorker
// -------------------------------------------------------------------------- //
  /**
   * Base class for print workers.
   *
   * Each print worker prints one or more pages.
   */
  class FormPrintWorker: public QObject {
    Q_OBJECT;
  public:
    FormPrintWorker(int numPages): mNumPages(numPages) {}

    virtual void operator() (QPrinter&, QPainter&) = 0;
    
    int numPages() const {
      return mNumPages;
    }

  signals:
    void advanced(int amount);

  private:
    int mNumPages;
  };


// -------------------------------------------------------------------------- //
// FormPrinter
// -------------------------------------------------------------------------- //
  /**
   * Class that performs printing.
   *
   * It takes one or more print workers and executes them consequently.
   */
  class FormPrinter: public Worker<void> {
    Q_OBJECT;
  public:
    FormPrinter(Shiken *ctx, QPrinter& printer): 
      Worker<void>(ctx, -1), mPrinter(printer), mTotalPages(0) {}

    FormPrinter& operator()(FormPrintWorker* worker) {
      connect(worker, SIGNAL(advanced(int)), this, SIGNAL(advanced(int)));
      worker->setParent(this);
      mWorkers.append(worker);
      mTotalPages += worker->numPages();

      return *this;
    }

    virtual void operator()() OVERRIDE {
      Q_EMIT started(mTotalPages);

      /* Init printer. */
      mPrinter.setOrientation(QPrinter::Portrait);
      mPrinter.setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);

      /* Set up painter & transformation. */
      QPainter painter;
      painter.begin(&mPrinter);

      /* Draw! */
      for(int i = 0; i < mWorkers.size(); i++ ) {
        if(i != 0)
          mPrinter.newPage();
        painter.resetTransform();
        mWorkers[i]->operator() (mPrinter, painter);
      }

      /* Clean up. */
      painter.end();
      Q_EMIT finished();
    }

  private:
    QList<FormPrintWorker*> mWorkers;
    QPrinter& mPrinter;
    int mTotalPages;
  };

} // namespace shiken

#endif // SHIKEN_FORM_PRINTER_H
