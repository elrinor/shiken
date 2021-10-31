#include "UserListGetter.h"
#include <QDomNode>
#include <QNetworkReply>
#include <QScopedPointer>
#include <shiken/utility/Log.h>
#include <shiken/utility/GuidCompressor.h>
#include <shiken/network/ConnectionManager.h>
#include <shiken/dao/DataAccessDriver.h>
#include <shiken/dao/SettingsDao.h>
#include <shiken/Shiken.h>

namespace shiken {
  UsersReply UserListGetter::operator()() {
    SHIKEN_LOG_MESSAGE("Getting user list.");

    UsersReply result;

    QScopedPointer<QNetworkReply> reply(ctx()->connectionManager()->post(
      mUrl,
      PostData().
        addField("loginStr", ctx()->model()->settingsDao()->login()).
        addField("password", ctx()->model()->settingsDao()->password()).
        addField("quizId",   mQuiz.isLoadedFromXml() ? "-1" : QString::number(mQuiz.quizId())).
        addField("quizGuid", mQuiz.guid())
    ));
        
    if(reply->error() != QNetworkReply::NoError) {
      criticalConnectionError(reply.data());
      return result;
    }

    QByteArray response = reply->readAll();
    QDomDocument doc;
    doc.setContent(response);

    QDomNode root = doc.firstChildElement("examlist");
    QDomElement error = root.firstChildElement("errorMessage");
    if(!error.isNull()) {
      SHIKEN_LOG_MESSAGE("Server error: " << error.text());
      Q_EMIT critical(error.text());
      return result;
    }

    result = ctx()->parseQuizXml(response, false);
    if(result.isNull()) {
      SHIKEN_LOG_MESSAGE("Invalid XML reply quiz list: " << response);
      criticalInvalidReplyFormat();
      return result;
    }

    SHIKEN_LOG_MESSAGE("Got user list.");
    Q_EMIT advanced(1);

    return result;
  }

} // namespace shiken
