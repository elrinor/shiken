#include "TestFormGenerator.h"
#include <barcode/ItfCode.h>
#include <shiken/utility/GuidCompressor.h>
#include <shiken/utility/BarcodeProcessor.h>

namespace shiken {

  void TestFormGenerator::drawPage(QPainter& painter, int pageNumber) {
    User user = mUsers[pageNumber / mPagesForStudent];

    /* Draw cells. */
    QBrush grayBrush(QColor(192, 192, 192));
    drawCells(painter, 0, 40000, WIDTH, HEIGHT - 50000, 5000, QPen(QBrush(Qt::black), 300), QPen(grayBrush, 100, Qt::DotLine), QPen(grayBrush, 200, Qt::DotLine));

    /* Write "variant". */
    painter.setFont(QFont("Arial", 4000));
    painter.drawText(1000, 40000, 100000, 6000, Qt::AlignBottom | Qt::AlignLeft, QS("Вариант:"));

    /* Write how-to. */
    painter.setFont(QFont("Arial", 2000));
    painter.drawText(0, 35000, WIDTH, 5000, Qt::AlignBottom | Qt::AlignLeft, QS("Поле для записи развернутого ответа. Делать какие-либо отметки за границами поля запрещается."));

    /* Draw anchors. */
    drawSymmetricAnchors(painter, 0, 0);
    drawSymmetricAnchors(painter, 30000, 0);
    drawSymmetricAnchors(painter, HEIGHT - 5000, 0);

    /* Draw barcode anchors. */
    painter.drawImage(QRect(7500, 20000, 15000, 15000), lAnchor());
    painter.drawImage(QRect(WIDTH - 22500, 20000, 15000, 15000), rAnchor());

    /* Draw barcode. */
    QDateTime newPrintTime = QDateTime::currentDateTime();
    if(!mLastPrintTime.isNull() && mLastPrintTime >= newPrintTime)
      newPrintTime = mLastPrintTime.addMSecs(1);
    
    Page page = Page(-1, user.userId(), newPrintTime);

    QString code = BarcodeProcessor::compose(GuidCompressor()(mQuiz.guidToCompress(), user.guid()), newPrintTime);
    barcode::ItfCode itfCode(code.toStdString());
    itfCode.addMod10CheckSum();
    drawBarcode(painter, itfCode.bars(), 22500, 20000, WIDTH - 45000, 12000);

    /* Write barcode text. */
    drawCenteredText(painter, code, 32000, false, QFont("Arial", 2000));

    /* Write header. */
    drawCenteredText(painter, QS("БЛАНК ОТВЕТОВ"),                    0, false, QFont("Arial", 4000, QFont::Bold));
    drawCenteredText(painter, QS("Копирование бланков запрещено"), 6000, false, QFont("Arial", 2000, QFont::Bold));

    /* Write login & other info. */
    QFont nameFont("Arial", 3000, QFont::Bold), valueFont("Arial", 3000);
    drawHeader(painter, QS("Логин:"),       user.login(),                             nameFont, valueFont, 10000, false, 0, 2, 0.4f, 7500);
    drawHeader(painter, QS("Предмет:"),     mQuiz.discStr(),                          nameFont, valueFont, 15000, false, 0, 2, 0.4f, 7500);

    if(mQuiz.startTime().isNull())
      drawHeader(painter, QS("Дата печати:"), QDateTime::currentDateTime().toString("dd.MM.yyyy"), nameFont, valueFont, 10000, false, 1, 2, 0.4f, 7500);
    else
      drawHeader(painter, QS("Дата работы:"), mQuiz.startTime().toString("dd.MM.yyyy"),            nameFont, valueFont, 10000, false, 1, 2, 0.4f, 7500);
    drawHeader(painter, QS("Работа:"), mQuiz.isLoadedFromXml() ? mQuiz.title() : QString::number(mQuiz.quizId()), nameFont, valueFont, 15000, false, 1, 2, 0.4f, 7500, false);

    emit pagePrinted(page);
  }

} // namespace shiken