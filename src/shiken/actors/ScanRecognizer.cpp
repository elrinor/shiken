#include "ScanRecognizer.h"
#include <cassert>
#include <exception>
#include <boost/scoped_array.hpp>
#include <QFile>
#include <QCryptographicHash>
#include <arx/ext/Vigra.h>
#include <arx/ext/Qt.h>
#include <acv/Extractor.h>
#include <acv/Matcher.h>
#include <acv/HomographyRansacModeller.h>
#include <acv/CollageRansacModeller.h>
#include <acv/Lma.h>
#include <acv/HomographyLmaModeller.h>
#include <acv/CollageLmaModeller.h>
#include <barcode/ItfRecognizer.h>
#include <shiken/Shiken.h>
#include <shiken/dao/DataAccessDriver.h>
#include <shiken/dao/SettingsDao.h>
#include <shiken/utility/Log.h>
#include <ImageUtils.h>
#include <Common.h>

namespace shiken {
  void ScanRecognizer::operator() () {
    SHIKEN_LOG_MESSAGE("Recognition started for file list");

    /* Load keypoint file. */
    QFile keysFile(":/test_form_header.ky");
    keysFile.open(QIODevice::ReadOnly);
    QByteArray rawKeys = keysFile.readAll();
    std::stringstream keyStream(std::string(rawKeys.constData(), rawKeys.size()));
    acv::Extract<> extract;
    keyStream >> extract;
    assert(extract.keypoints().size() >= MIN_KEYPOINTS_PER_IMAGE);

    SHIKEN_LOG_MESSAGE("Keypoint file loaded");

    /* Read code position. */
    QFile codePosFile(":/test_form_header_codepos.txt");
    codePosFile.open(QIODevice::ReadOnly);
    QByteArray rawCodePos = codePosFile.readAll();
    std::stringstream codePosStream(std::string(rawCodePos.constData(), rawCodePos.size()));
    int codeX, codeY, codeW, codeH;
    codePosStream >> codeX >> codeY >> codeW >> codeH;
    assert(codeX > 0 && codeW > 0 && codeY > 0 && codeH > 0 && codeX + codeW <= extract.width() && codeY + codeH <= extract.height());

    SHIKEN_LOG_MESSAGE("Code position read");

    /* Loop through all files. */
    foreach(Scan scan, mScans) {
      QString barcode;

      scan.setState(Scan::UNRECOGNIZED);

      try {
        QFile file(scan.fileName());
        if(!file.open(QIODevice::ReadOnly))
          continue;

        QCryptographicHash hasher(QCryptographicHash::Sha1);
        boost::scoped_array<char> buffer(new char[1024 * 1024]);
        while(!file.atEnd()) {
          qint64 length = file.read(buffer.get(), 1024 * 1024);
          hasher.addData(buffer.get(), static_cast<int>(length));
        }
        scan.setHash(QString(hasher.result().toHex()));

        /* Load input fImage. */
        SHIKEN_LOG_MESSAGE("Loading image " << scan.fileName());
        vigra::BImage srcImage, outImage;
        importImage(srcImage, scan.fileName().toStdWString());

        /* Match & warp. */
        match(
          srcImage, 
          vigra::Size2D(ctx()->model()->settingsDao()->maxKeyImageWidth(), ctx()->model()->settingsDao()->maxKeyImageHeight()), 
          extract, 
          outImage, 
          ctx()->model()->settingsDao()->maxRansacError(), 
          true
        );

        /* Recognize. */
        SHIKEN_LOG_MESSAGE("Recognizing");
        vigra::BImage codeImage(codeW, codeH);
        copyImage(srcImageRange(outImage, vigra::Rect2D(codeX, codeY, codeX + codeW, codeY + codeH)), destImage(codeImage));

        barcode::ItfCode code = barcode::ItfRecognizer(codeImage)(DEFAULT_MIN_ITERATIONS, DEFAULT_MAX_ITERATIONS);
        if(code.size() == 0)
          throw std::logic_error("Could not recognize barcode");

        barcode = QString::fromStdString(code.string());

        scan.setState(Scan::RECOGNIZED);
        SHIKEN_LOG_MESSAGE("Recognized barcode " << barcode);
      } catch (std::exception& e) {
        (void) e; /* To eliminate "Unused variable" warning when not using logging. */
        SHIKEN_LOG_MESSAGE("Exception " << QString::fromStdString(e.what()));
      } catch (...) {
        SHIKEN_LOG_MESSAGE("Unknown exception");
      }

      SHIKEN_LOG_MESSAGE("Notifying...");
      Q_EMIT scanRecognized(scan, barcode);
      Q_EMIT advanced(1);
    }
  }

} // namespace shiken
