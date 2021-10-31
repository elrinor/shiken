#include "ScanUploader.h"
#include <QFile>
#include <QTextStream>
#include <QNetworkReply>
#include <QScopedPointer>
#include <arx/ScopeExit.h>
#include <arx/Foreach.h>
#include <shiken/utility/Log.h>
#include <shiken/Shiken.h>
#include <shiken/network/ConnectionManager.h>
#include <shiken/dao/DataAccessDriver.h>
#include <shiken/dao/SettingsDao.h>

namespace shiken {
  void ScanUploader::operator()() {
    foreach(Scan scan, mScans) {
      SHIKEN_LOG_MESSAGE("Sending " << scan.fileName());

      if(!QFile::exists(scan.fileName())) {
        SHIKEN_LOG_MESSAGE("File \"" << scan.fileName() << "\" not found.");
        Q_EMIT scanNotFound(scan);
        Q_EMIT advanced(1);
        continue;
      }

      QScopedPointer<QNetworkReply> reply(ctx()->connectionManager()->post(
        mUrl, 
        PostData().
          addField("loginStr",      ctx()->model()->settingsDao()->login()).
          addField("password",      ctx()->model()->settingsDao()->password()).
          addField("quizId",        mQuiz.isLoadedFromXml() ? "-1" : QString::number(mQuiz.quizId())).
          addField("quizGuid",      mQuiz.guidToCompress()). /* TODO: guid in other messages. */
          addField("guidMode",      QString::number(mQuiz.guidMode())).
          addFile("file", scan.fileName(), "image/jpeg")
      ));
      if(reply->error() != QNetworkReply::NoError) {
        criticalConnectionError(reply.data());
        return;
      }

      QString response = QString::fromUtf8(reply->readAll());
      QTextStream responseStream(&response);
      QString status = responseStream.readLine().trimmed();
      QString message = responseStream.readLine().trimmed();

      if(status != "OK") {
        SHIKEN_LOG_MESSAGE("Failed with message " << message);
        Q_EMIT critical(message);
        return;
      }

      SHIKEN_LOG_MESSAGE("Emitting OK for " << scan.fileName());
      Q_EMIT scanUploaded(scan);
      Q_EMIT advanced(1);

      if(canceled())
        return;
    }

  }

} // namespace shiken
