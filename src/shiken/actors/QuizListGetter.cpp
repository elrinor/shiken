#include "QuizListGetter.h"
#include <QNetworkReply>
#include <QScopedPointer>
#include <QXmlStreamReader>
#include <arx/Foreach.h>
#include <arx/ext/qt/XmlMappings.h>
#include <shiken/utility/Log.h>
#include <shiken/network/ConnectionManager.h>
#include <shiken/dao/DataAccessDriver.h>
#include <shiken/dao/SettingsDao.h>
#include <shiken/parsers/QuizListReplyParser.h>
#include <shiken/parsers/DateTimeParser.h>
#include <shiken/Shiken.h>

namespace shiken {

    class QuizListGetterPrivate {
    public:
        QUrl url;

        xml_schema::string_pimpl stringParser;
        xml_schema::int_pimpl intParser;
        DateTimeParser dateTimeParser;
        QuizParser quizParser;
        QuizListReplyParser quizListReplyParser;

        void init() {
            dateTimeParser.parsers(intParser, intParser, intParser, intParser, intParser, intParser, intParser, intParser);
            quizParser.parsers(intParser, intParser, stringParser, dateTimeParser, intParser, dateTimeParser, stringParser, stringParser, stringParser, stringParser);
            quizListReplyParser.parsers(intParser, stringParser, quizParser);
        }
    };

    QuizListGetter::QuizListGetter(Shiken *ctx, const QUrl &url):
        Worker<QList<Quiz> >(ctx, 1), 
        d(new QuizListGetterPrivate()) 
    {
        d->url = url;
        d->init();
    }

    QuizListGetter::~QuizListGetter() {
        return;
    }

    QList<Quiz> QuizListGetter::operator() () {
        SHIKEN_LOG_MESSAGE("Getting quiz list.");

        QList<Quiz> result;

        QScopedPointer<QNetworkReply> reply(ctx()->connectionManager()->post(
            d->url,
            PostData().
                addField("loginStr", ctx()->model()->settingsDao()->login()).
                addField("password", ctx()->model()->settingsDao()->password())
        ));

        if(reply->error() != QNetworkReply::NoError) {
            criticalConnectionError(reply.data());
            return result;
        }

        QByteArray response = reply->readAll();

        SHIKEN_LOG_MESSAGE("Got quiz list reply: " << response);

        /* Parse. */
        try {
            xml_schema::document_pimpl document(d->quizListReplyParser, "quizzes");
            d->quizListReplyParser.pre();
            document.parse(response.constData(), response.size(), true);
            d->quizListReplyParser.post_QuizListReply();
        } catch(const xml_schema::parser_exception &e) {
            SHIKEN_LOG_MESSAGE(e.line() << ":" << e.column() << ": Error: " << e.text());
            criticalInvalidReplyFormat();
            return result;
        }

        if(d->quizListReplyParser.hasError()) {
            SHIKEN_LOG_MESSAGE("Server error: " << d->quizListReplyParser.errorMessage());
            Q_EMIT critical(d->quizListReplyParser.errorMessage());
            return result;
        }

        result = d->quizListReplyParser.quizzes();

        foreach(Quiz &quiz, result) {
            quiz.setLoadedFromXml(false);
            quiz.fixup();
        }

        for(int i = result.size() - 1; i >= 0; i--) {
            if(result[i].guid().isEmpty()) {
                SHIKEN_LOG_MESSAGE("Ignoring quiz with empty GUID: " << result[i].title());

                result.removeAt(i);
            }
        }

        SHIKEN_LOG_MESSAGE("Got quiz list OK.");
        Q_EMIT advanced(1);
        return result;
    }

} // namespace shiken
