#ifndef SHIKEN_QUIZ_LIST_REPLY_PARSER_H
#define SHIKEN_QUIZ_LIST_REPLY_PARSER_H

#include <shiken/config.h>
#include <shiken/entities/Quiz.h>
#include "xsd_QuizListReply.h"

namespace shiken {

    class QuizListReplyParser: public xsd::shiken::QuizListReply_pskel {
    public:
        bool hasError() const {
            return m_tag == errorMessage_tag;
        }

        const QString &errorMessage() const {
            return m_errorMessage;
        }

        const QList<Quiz> &quizzes() const {
            return m_quizzes;
        }

    public:
        virtual void pre() override {
            m_tag = errorMessage_tag;
            m_errorMessage = QString();
            m_quizzes.clear();
        }

        virtual void choice_arm(choice_arm_tag tag) override {
            m_tag = tag;
        }

        virtual void errorMessage(const std::string &errorMessage) override {
            m_errorMessage = QString::fromUtf8(errorMessage.c_str());
        }

        virtual void quiz(const Quiz &quiz) override {
            m_quizzes.push_back(quiz);
        }

        virtual void post_QuizListReply() override {
            return;
        }

    private:
        choice_arm_tag m_tag;
        QString m_errorMessage;
        QList<Quiz> m_quizzes;
    };


    class QuizParser: public xsd::shiken::Quiz_pskel {
    public:
        virtual void pre() override {
            m_quiz = Quiz();
        }

        virtual void discId(int /*discId*/) override {
            return;
        }

        virtual void quizId(int quizId) override {
            m_quiz.setQuizId(quizId);
        }

        virtual void discStr(const std::string &discStr) override {
            m_quiz.setDiscStr(QString::fromUtf8(discStr.c_str()));
        }

        virtual void startTime(const QDateTime &startTime) override {
            m_quiz.setStartTime(startTime);
        }

        virtual void durationMin(int durationMin) override {
            m_quiz.setDurationMinutes(durationMin);
        }

        virtual void submitEndTime(const QDateTime &submitEndTime) override {
            m_quiz.setSubmitEndTime(submitEndTime);
        }

        virtual void title(const std::string &title) override {
            m_quiz.setTitle(QString::fromUtf8(title.c_str()));
        }

        virtual void description(const std::string &description) override {
            m_quiz.setDescription(QString::fromUtf8(description.c_str()));
        }

        virtual void guid(const std::string &guid) override {
            m_quiz.setGuid(QString::fromUtf8(guid.c_str()));
        }

        virtual void qsgguid(const std::string &qsgguid) override {
            m_quiz.setQsgGuid(QString::fromUtf8(qsgguid.c_str()));
        }

        virtual Quiz post_Quiz() override {
            return m_quiz;
        }

    private:
        Quiz m_quiz;
    };

} // namespace shiken

#endif // SHIKEN_QUIZ_LIST_REPLY_PARSER_H
