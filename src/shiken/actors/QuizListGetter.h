#ifndef SHIKEN_QUIZ_LIST_GETTER_H
#define SHIKEN_QUIZ_LIST_GETTER_H

#include <shiken/config.h>
#include <QList>
#include <QUrl>
#include <QScopedPointer>
#include <shiken/entities/Quiz.h>
#include "Worker.h"

namespace shiken {

    class Shiken;
    class QuizListGetterPrivate;

    /**
     * This worker gets a list of all quizzes from the server.
     */
    class QuizListGetter: public Worker<QList<Quiz> > {
        Q_OBJECT;

    public:
        QuizListGetter(Shiken *ctx, const QUrl &url);

        virtual ~QuizListGetter();

        QList<Quiz> operator()();

    private:
        QScopedPointer<QuizListGetterPrivate> d;
    };

} // namespace shiken

#endif // SHIKEN_QUIZ_LIST_GETTER_H
