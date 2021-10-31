#ifndef SHIKEN_BARCODE_PROCESSOR_H
#define SHIKEN_BARCODE_PROCESSOR_H

#include <shiken/config.h>
#include <utility> /* For std::pair. */
#include <boost/preprocessor/stringize.hpp>
#include <QString>
#include <QDateTime>

namespace shiken {
// -------------------------------------------------------------------------- //
// BarcodeProcessor
// -------------------------------------------------------------------------- //
  class BarcodeProcessor {
  public:
    static std::pair<QString, QDateTime> decompose(const QString &barcode) {
      /* Get id. */
      bool idOk = true;
      int id = barcode.mid(0, 2).toInt(&idOk);
      if(!idOk || id != SHIKEN_TEST_BARCODE_ID)
        return std::pair<QString, QDateTime>();

      /* Get compressed guid. */
      QString compressedGuid = barcode.mid(2, 20);

      /* Get date & time. */
      QDateTime dateTime = QDateTime::fromString(barcode.mid(22, SHIKEN_BARCODE_DATE_TIME_FORMAT_LENGTH), SHIKEN_BARCODE_DATE_TIME_FORMAT);
      QDateTime now = QDateTime::currentDateTime();
      if(dateTime.date().year() / 100 != now.date().year() / 100)
        dateTime.setDate(QDate(now.date().year() - now.date().year() % 100 + dateTime.date().year() % 100, dateTime.date().month(), dateTime.date().day()));

      /* The rest of the string is not processed. It may have been used for checksum. */

      return std::make_pair(compressedGuid, dateTime);
    }

    static QString compose(const QString &compressedGuid, const QDateTime &printTime) {
      QString result;

      /* Add id. */
      result.append(BOOST_PP_STRINGIZE(SHIKEN_TEST_BARCODE_ID));

      /* Add compressed guid. */
      result.append(compressedGuid);

      /* Add date & time. */
      result.append(printTime.toString(SHIKEN_BARCODE_DATE_TIME_FORMAT));

      return result;
    }

  };

} // namespace shiken

#endif // SHIKEN_BARCODE_PROCESSOR_H
