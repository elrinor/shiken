#ifndef HTMLCONV_HTML_CONV_H
#define HTMLCONV_HTML_CONV_H

#include "config.h"
#include <QObject>
#include <QWebPage>
#include <QWebFrame>

namespace htmlconv {
// -------------------------------------------------------------------------- //
// HtmlConv
// -------------------------------------------------------------------------- //
  /**
   * This class performs rendering and downloading of HTML pages.
   */
  class HtmlConv: public QObject {
    Q_OBJECT;
  public:
    /**
     * Constructor.
     * 
     * @param url                      Url to download HTML page from.
     * @param width                    Preferred width of the page's contents.
     */
    HtmlConv(const QUrl &url, int width): mWidth(width) {
      mPage.setPreferredContentsSize(QSize(mWidth, 1));
      mPage.mainFrame()->load(url);
      connect(mPage.networkAccessManager(), SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)), this, SLOT(sslErrors(QNetworkReply *, const QList<QSslError> &)));
      connect(&mPage, SIGNAL(loadFinished(bool)), this, SIGNAL(loadFinished()));
    }

    /**
     * @returns                        Size in pixels of the downloaded page's
     *                                 contents.
     */
    QSize contentsSize() const {
      return mPage.mainFrame()->contentsSize();
    }

    /**
     * Renders downloaded page into given painter.
     *
     * @param painter                  Painter to render the downloaded page 
     *                                 into.
     */
    void render(QPainter *painter) {
      mPage.setViewportSize(contentsSize());
      mPage.mainFrame()->render(painter);
    }

  Q_SIGNALS:
    /**
     * This signal is fired when page downloading finishes.
     */
    void loadFinished();

  private Q_SLOTS:
    /**
     * For debugging purposes.
     */
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors) {
      Q_UNUSED(reply);
      Q_UNUSED(errors);
      return;
    }

  private:
    int mWidth;
    QWebPage mPage;
  };

} // htmlconv

#endif // HTMLCONV_HTML_CONV_H
