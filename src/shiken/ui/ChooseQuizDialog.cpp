#include "ChooseQuizDialog.h"
#include <QMessageBox>
#include <arx/Foreach.h>
#include <arx/ScopeExit.h>
#include <shiken/actors/UserListGetter.h>
#include <shiken/actors/WorkerRunner.h>
#include <shiken/dao/DataAccessDriver.h>
#include <shiken/dao/QuizDao.h>
#include <shiken/dao/SettingsDao.h>
#include <shiken/dao/QuizDao.h>
#include <shiken/dao/UserDao.h>
#include <shiken/entities/Quiz.h>
#include <shiken/entities/User.h>
#include <shiken/Shiken.h>
#include "ui_ChooseQuizDialog.h"
#include "QuizWidget.h"

namespace shiken {
  ChooseQuizDialog::ChooseQuizDialog(Shiken *ctx, QWidget *parent):
    QDialog(parent),
    mUi(new Ui::ChooseQuizDialog()),
    mCtx(ctx),
    mBackClicked(false)
  {
    mUi->setupUi(this);

    QList<Quiz> quizzes = mCtx->model()->quizDao()->selectAll();

    qSort(quizzes.begin(), quizzes.end(), [](const Quiz &l, const Quiz &r) { return l.startTime() > r.startTime(); });
    foreach(Quiz &quiz, quizzes) {
      QuizWidget *quizWidget = new QuizWidget(quiz);
      mQuizWidgets.push_back(quizWidget);
      connect(quizWidget, SIGNAL(clicked()), this, SLOT(at_quizWidget_clicked()));
      mUi->quizzesLayout->addWidget(quizWidget);

      if(quiz.quizId() == mCtx->model()->settingsDao()->quizId())
        quizWidget->setSelected(true);
    }
  }

  ChooseQuizDialog::~ChooseQuizDialog() {
    return;
  }

  void ChooseQuizDialog::on_backButton_clicked() {
    mBackClicked = true;
    mCtx->model()->settingsDao()->setQuizId(0);
    accept();
  }

  void ChooseQuizDialog::at_quizWidget_clicked() {
    QuizWidget *quizWidget = dynamic_cast<QuizWidget *>(sender());
    
    assert(quizWidget != NULL);

    foreach(QuizWidget *otherQuizWidget, mQuizWidgets)
      otherQuizWidget->setSelected(otherQuizWidget == quizWidget);
    
    mCtx->model()->settingsDao()->setQuizId(quizWidget->quiz().quizId());
    mUi->okButton->setEnabled(true);
  }

  void ChooseQuizDialog::on_okButton_clicked() {
    setCursor(Qt::WaitCursor);
    setDisabled(true);

    ARX_SCOPE_EXIT(&) {
      this->unsetCursor();
      this->setDisabled(false);
    };

    Quiz quiz = mCtx->model()->quizDao()->selectById(mCtx->model()->settingsDao()->quizId());

    UserListGetter userGetter(mCtx, QUrl(mCtx->model()->settingsDao()->targetUrl() + SHIKEN_USERS_COMMAND), quiz);
    UsersReply usersReply = runWorker(userGetter, this, "", QS("Ошибка: %1"));

    if(userGetter.succeeded()) {
      if(usersReply.users().empty()) {
        QMessageBox::critical(this, QS("Ошибка"), QS("Список пользователей для этого мероприятия пуст. \nВыберите другое мероприятие."));
        return;
      }

      if(mCtx->model()->settingsDao()->isSingleUser()) {
        if(usersReply.users().size() != 1 || usersReply.user(0).login() != mCtx->model()->settingsDao()->login()) {
          QMessageBox::critical(this, QS("Ошибка"), QS("Неправильный формат ответа от сервера."));
          return;
        }
      }

      mCtx->storeQuizData(usersReply);
      accept();
    }
  }

} // namespace shiken
