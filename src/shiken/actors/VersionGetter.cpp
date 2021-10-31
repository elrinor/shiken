#include "VersionGetter.h"
#include <QDomNode>
#include <QNetworkReply>
#include <QScopedPointer>
#include <shiken/network/ConnectionManager.h>
#include <shiken/utility/Log.h>
#include <shiken/Shiken.h>

namespace shiken {
  int VersionGetter::operator()() {
    SHIKEN_LOG_MESSAGE("Getting version #.");

    int result = -1;

    QScopedPointer<QNetworkReply> reply(ctx()->connectionManager()->post(mUrl, PostData()));

    if(reply->error() != QNetworkReply::NoError) {
      criticalConnectionError(reply.data());
      return result;
    }

    QByteArray response = reply->readAll();
    QDomDocument doc;
    doc.setContent(response);

    QDomNode root = doc.firstChildElement("latestClientVersion");
    if(root.isNull()) {
      SHIKEN_LOG_MESSAGE("Invalid XML reply version: " << response);
      criticalInvalidReplyFormat();
      return result;
    }

    result = root.toElement().text().toInt();

    SHIKEN_LOG_MESSAGE("Got version # " << result);
    Q_EMIT advanced(1);

    return result;
  }

} // namespace shiken
