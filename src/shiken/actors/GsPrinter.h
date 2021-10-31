#ifndef SHIKEN_GS_PRINTER_H
#define SHIKEN_GS_PRINTER_H

#include <shiken/config.h>
#include <QString>
#include "Worker.h"

namespace shiken {
// -------------------------------------------------------------------------- //
// GsPrinter
// -------------------------------------------------------------------------- //
  /**
   * This worker prints the given file using Ghostscript.
   * 
   * One of the reasonable questions is: why mess with Ghostscript if it's
   * possible to use QPrinter for printing? The reason for this is that
   * printing via QPrinter is WAY TOO SLOW. Generation a PDF and then printing
   * it via Ghostscript is much faster.
   */
  class GsPrinter: public Worker<void> {
    Q_OBJECT;
  public:
    /**
     * Constructor.
     *
     * @param ctx                      Application context.
     * @param fileName                 Name of the file to print.
     * @param printerName              Name of the printer to use.
     * @param parent                   Parent object.
     */
    GsPrinter(Shiken *ctx, QString fileName, QString printerName): 
      Worker<void>(ctx, 1), mFileName(fileName), mPrinterName(printerName) {}

    virtual void operator()() OVERRIDE;

  private:
    /** Name of the file to print. */
    QString mFileName;

    /** Name of the printer to use. */
    QString mPrinterName;
  };

} // namespace shiken

#endif // SHIKEN_GS_PRINTER_H
