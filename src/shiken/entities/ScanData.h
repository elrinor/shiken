#ifndef SHIKEN_SCAN_DATA_H
#define SHIKEN_SCAN_DATA_H

#include <shiken/config.h>
#include <QString>
#include <QMetaType>
#include <QList>
#include <arx/StaticBlock.h>

namespace shiken {
// -------------------------------------------------------------------------- //
// ScanData
// -------------------------------------------------------------------------- //
  /**
   * This struct represents a deserialized description of a scan that was
   * received from server.
   */
  struct ScanData {
    int scanId;
    long long size;
    QString origName;
    QString contentType;
    int quizId;
    int groupId;
    QString status;
    QString barcode;
    QString hashcode;
    QString sha1;
    QString message;
    QString pageStamp;
    int version;
    int dpi;
    int width;
    int height;
    int viewportX;
    int viewportY;
    int viewportW;
    int viewportH;
    int userId;
  };

} // namespace shiken


Q_DECLARE_METATYPE(::shiken::ScanData);
Q_DECLARE_METATYPE(QList< ::shiken::ScanData>);

ARX_STATIC_BLOCK(SHIKEN_SCAN_DATA) {
  qRegisterMetaType< ::shiken::ScanData>("ScanData");
  qRegisterMetaType<QList< ::shiken::ScanData> >("QList<ScanData>");
}

#endif // SHIKEN_SCAN_DATA_H
