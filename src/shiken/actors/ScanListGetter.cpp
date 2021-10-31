#include "ScanListGetter.h"
#include <QNetworkReply>
#include <QScopedPointer>
#include <arx/Foreach.h>
#include <shiken/network/ConnectionManager.h>
#include <shiken/dao/DataAccessDriver.h>
#include <shiken/dao/SettingsDao.h>
#include <shiken/parsers/ScanListReplyParser.h>
#include <shiken/utility/Log.h>
#include <shiken/Shiken.h>

namespace shiken {
    class ScanListGetterPrivate {
    public:
        QUrl url;
        Quiz quiz;

        xml_schema::string_pimpl stringParser;
        xml_schema::int_pimpl intParser;
        xml_schema::long_pimpl longParser;
        ScanReplyParser scanReplyParser;
        ScanListReplyParser scanListReplyParser;

        void init() {
            scanReplyParser.parsers(
                intParser, longParser, stringParser, stringParser, intParser, intParser, stringParser, intParser, intParser, 
                stringParser, stringParser, stringParser, stringParser, stringParser, 
                intParser, intParser, intParser, intParser, intParser, intParser,intParser, intParser, intParser
            );
            scanListReplyParser.parsers(stringParser, scanReplyParser);
        }
    };

    ScanListGetter::ScanListGetter(Shiken *ctx, const QUrl &url, const Quiz &quiz):
        Worker<QList<ExtendedScan> >(ctx, 1),
        d(new ScanListGetterPrivate())
    {
        d->url = url;
        d->quiz = quiz;
        d->init();
    }

    ScanListGetter::~ScanListGetter() {
        return;
    }

    QList<ExtendedScan> ScanListGetter::operator()() {
        SHIKEN_LOG_MESSAGE("Getting scan list.");

        QList<ExtendedScan> result;

        QScopedPointer<QNetworkReply> reply(ctx()->connectionManager()->post(
            d->url,
            PostData().
                addField("loginStr", ctx()->model()->settingsDao()->login()).
                addField("password", ctx()->model()->settingsDao()->password()).
                addField("quizId",   d->quiz.isLoadedFromXml() ? "-1" : QString::number(d->quiz.quizId())).
                addField("quizGuid", d->quiz.guid())
        ));

        if(reply->error() != QNetworkReply::NoError) {
            criticalConnectionError(reply.data());
            return result;
        }

        QByteArray response = reply->readAll();

        SHIKEN_LOG_MESSAGE("Got scan list reply: " << response);

        /* Parse. */
        try {
            xml_schema::document_pimpl document(d->scanListReplyParser, "scans");
            d->scanListReplyParser.pre();
            document.parse(response.constData(), response.size(), true);
            d->scanListReplyParser.post_ScanListReply();
        } catch(const xml_schema::parser_exception &e) {
            SHIKEN_LOG_MESSAGE(e.line() << ":" << e.column() << ": Error: " << e.text());
            criticalInvalidReplyFormat();
            return result;
        }

        if(d->scanListReplyParser.hasError()) {
            SHIKEN_LOG_MESSAGE("Server error: " << d->scanListReplyParser.errorMessage());
            Q_EMIT critical(d->scanListReplyParser.errorMessage());
            return result;
        }

        result = d->scanListReplyParser.scans();

        /* Set quiz id for the returned scans. */
        foreach(ExtendedScan &scan, result)
            scan.setQuizId(d->quiz.quizId());

        SHIKEN_LOG_MESSAGE("Got scan list.");
        Q_EMIT advanced(1);

        return result;
    }

} // namespace shiken
