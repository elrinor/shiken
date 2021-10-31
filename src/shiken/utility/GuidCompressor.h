#ifndef SHIKEN_GUID_COMPRESSOR_H
#define SHIKEN_GUID_COMPRESSOR_H

#include <shiken/config.h>
#include <QString>
#include <QCryptographicHash>

namespace shiken {
// -------------------------------------------------------------------------- //
// GuidCompressor
// -------------------------------------------------------------------------- //
  /**
   * This class performs GUID compression. Compressed guids are used for
   * user identification.
   */
  class GuidCompressor {
  public:
    GuidCompressor() {}

    /**
     * @param quizGuid                 Guid of a quiz.
     * @param userGuid                 Guid of a user.
     * @returns                        Compressed guid, a string of length 20
     *                                 consisting of decimal digits.
     */
    QString operator()(QString quizGuid, QString userGuid) {
      QByteArray hashedGuids = QCryptographicHash::hash(QString(quizGuid + userGuid).toLower().toAscii(), QCryptographicHash::Md5);
      assert(hashedGuids.size() == 16);

      quint64* hash64 = reinterpret_cast<quint64*>(hashedGuids.data());
      QString result = QString("%1").arg(hash64[0] ^ hash64[1], 20, 10, QChar('0'));

      assert(result.size() == 20);

      return result;
    }

  };

} // namespace shiken

#endif // SHIKEN_GUID_COMPRESSOR_H
