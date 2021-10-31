#ifndef SHIKEN_FORM_GENERATOR_BASE_H
#define SHIKEN_FORM_GENERATOR_BASE_H

#include <shiken/config.h>
#include <QObject>
#include <QPrinter>
#include <QPainter>
#include <QFontMetrics>
#include <barcode/ItfCode.h>
#include "FormPrinter.h"

namespace shiken {
// -------------------------------------------------------------------------- //
// FormGeneratorBase
// -------------------------------------------------------------------------- //
  /**
   * This is a base class for all form generators.
   *
   * It defines several useful generic functions and enums, as well as
   * allows derived classes to draw both in portrait and in landscape 
   * orientation without bothering about transformations.
   */
  template<QPrinter::Orientation orientation = QPrinter::Portrait>
  class FormGeneratorBase: public FormPrintWorker {
  public:
    FormGeneratorBase(int numPages): FormPrintWorker(numPages), mLAnchor(":/anchor_ls.png"), mRAnchor(":/anchor_rs.png"), mMAnchor(":/anchor_ms.png"), mNAnchor(":/anchor_ns.png") {}

    virtual void operator() (QPrinter& printer, QPainter& painter) {
      /* Get paper size & compute margins. */
      QRectF pageRect = printer.pageRect(QPrinter::Millimeter);
      qreal hMargin = (pageRect.width() - PORTRAIT_WIDTH / 1000.0) / 2;
      qreal vMargin = (pageRect.height() - PORTRAIT_HEIGHT / 1000.0) / 2;

      /* Set up transformation. */
      painter.scale(printer.resolution() / 25400.0, printer.resolution() / 25400.0); /* Convert to microns. */
      painter.translate(hMargin * 1000, vMargin * 1000); /* Add margins. */
      if(orientation == QPrinter::Landscape) { /* Rotate if needed. */
        painter.translate(0, PORTRAIT_HEIGHT);
        painter.setTransform(QTransform(0, -1, 1, 0, 0, 0), true);
      }

      /* Draw! */
      for(int i = 0; i < numPages(); i++) {
        if(i != 0)
          printer.newPage();

        drawPage(painter, i);

        Q_EMIT advanced(1);
      }
    }

  protected:
    /** Declaring this one as virtual will save us a lot of pain. */
    virtual void drawPage(QPainter& painter, int pageNum) = 0;

    /** Page size in microns. */
    enum {
      PORTRAIT_WIDTH = 190000,
      PORTRAIT_HEIGHT = 260000,

      WIDTH  = orientation == QPrinter::Portrait ? PORTRAIT_WIDTH : PORTRAIT_HEIGHT,
      HEIGHT = orientation == QPrinter::Portrait ? PORTRAIT_HEIGHT : PORTRAIT_WIDTH
    };

    const QImage& lAnchor() const {
      return mLAnchor;
    }

    const QImage& rAnchor() const {
      return mRAnchor;
    }

    const QImage& nAnchor() const {
      return mNAnchor;
    }

    const QImage& mAnchor() const {
      return mMAnchor;
    }

    void drawCells(QPainter& painter, int x, int y, int w, int h, int step, const QPen& borderPen, const QPen& oddLinePen, const QPen& evenLinePen) {
      bool isOdd = true;
      for(int iy = y + step; iy < y + h; iy += step, isOdd = !isOdd) {
        painter.setPen(isOdd ? oddLinePen : evenLinePen);
        painter.drawLine(x, iy, x + w, iy);
      }

      isOdd = true;
      for(int ix = x + step; ix < x + w; ix += step, isOdd = !isOdd) {
        painter.setPen(isOdd ? oddLinePen : evenLinePen);
        painter.drawLine(ix, y, ix, y + h);
      }

      painter.setPen(borderPen);
      painter.drawRect(x, y, w, h);
    }

    void drawSymmetricAnchors(QPainter& painter, int y, int sideSkip) {
      painter.fillRect(               sideSkip, y, 5000, 5000, QColor(0, 0, 0));
      painter.fillRect(WIDTH - sideSkip - 5000, y, 5000, 5000, QColor(0, 0, 0));
    }

    void drawBarcode(QPainter& painter, const std::vector<barcode::ItfBar>& bars, int x, int y, int w, int h) {
      /* Calculate optimal bar width. */
      int barCount = 20; /* Spaces on the sides. */
      foreach(barcode::ItfBar bar, bars)
        barCount += bar.isThick() ? 3 : 1;
      int barWidth = w / barCount;
      int barCodeWidth = barWidth * barCount;

      /* Draw barcode. */
      int ix = x + (w - barCodeWidth) / 2  + 10 * barWidth;
      foreach(barcode::ItfBar bar, bars) {
        int thickness = barWidth * (bar.isThick() ? 3 : 1);
        if(bar.isBlack())
          painter.fillRect(ix, y, thickness, h, QColor(0, 0, 0));
        ix += thickness;
      }
    }

    void drawCenteredText(QPainter& painter, const QString& text, int y, bool yIsBottom, const QFont& font, int xLeft = 0, int xRight = WIDTH) {
      int height = QFontMetrics(font).height();

      painter.setFont(font);
      painter.drawText(xLeft, y - (yIsBottom ? height : 0), xRight, height, Qt::AlignHCenter | (yIsBottom ? Qt::AlignBottom : Qt::AlignTop), text);
    }

    void drawHeader(QPainter& painter, const QString& name, const QString& value, const QFont& nameFont, const QFont& valueFont, int y, bool yIsBottom, int col, int colCount, float nameRelWidth, int leftSkip, int rightSkip, bool multiLine = false) {
      int  nameHeight = QFontMetrics( nameFont).height();
      int valueHeight = QFontMetrics(valueFont).height();

      int colWidth = (WIDTH - leftSkip - rightSkip) / colCount;
      int nameWidth = static_cast<int>(colWidth * nameRelWidth);
      int xName = leftSkip + colWidth * col;
      int xValue = xName + nameWidth;

      painter.setFont(nameFont);
      painter.drawText(xName, y - (yIsBottom ? nameHeight : 0), nameWidth, nameHeight, Qt::AlignLeft | (yIsBottom ? Qt::AlignBottom : Qt::AlignTop) | Qt::TextDontClip, name);

      painter.setFont(valueFont);
      painter.drawText(xValue, y - (yIsBottom ? valueHeight : 0), colWidth - nameWidth, valueHeight, Qt::AlignLeft | (yIsBottom ? Qt::AlignBottom : Qt::AlignTop) | (multiLine ? Qt::TextWordWrap : 0) | Qt::TextDontClip, value);
    }

    void drawHeader(QPainter& painter, const QString& name, const QString& value, const QFont& nameFont, const QFont& valueFont, int y, bool yIsBottom, int col, int colCount, float nameRelWidth, int sideSkip, bool multiLine = false) {
      drawHeader(painter, name, value, nameFont, valueFont, y, yIsBottom, col, colCount, nameRelWidth, sideSkip, sideSkip, multiLine);
    }

  private:
    QImage mLAnchor, mRAnchor, mMAnchor, mNAnchor;
  };

} // namespace shiken

#endif // SHIKEN_FORM_GENERATOR_BASE_H
