#ifndef SHIKEN_TEST_FORM_GENERATOR_H
#define SHIKEN_TEST_FORM_GENERATOR_H

#include <shiken/config.h>
#include <QObject>
#include <QPrinter>
#include <QList>
#include <shiken/entities/Quiz.h>
#include <shiken/entities/User.h>
#include <shiken/entities/Page.h>
#include "FormGeneratorBase.h"

namespace shiken {
// -------------------------------------------------------------------------- //
// TestFormGenerator
// -------------------------------------------------------------------------- //
  /**
   * This class performs generation of answer sheets.
   */
  class TestFormGenerator: public FormGeneratorBase<QPrinter::Portrait> {
    Q_OBJECT;
  public:
    TestFormGenerator(const Quiz &quiz, const QList<User> &users, int pagesForStudent): 
      FormGeneratorBase<QPrinter::Portrait>(users.size() * pagesForStudent), mQuiz(quiz), mUsers(users), mPagesForStudent(pagesForStudent) {}

  Q_SIGNALS:
    void pagePrinted(Page page);

  protected:
    virtual void drawPage(QPainter& painter, int pageNumber);

  private:
    const Quiz mQuiz;
    const QList<User> mUsers;
    int mPagesForStudent;
    QDateTime mLastPrintTime;
  };

} // namespace shiken

#endif // SHIKEN_TEST_FORM_GENERATOR_H
