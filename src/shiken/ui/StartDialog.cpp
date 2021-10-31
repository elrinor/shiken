#include "StartDialog.h"
#include <boost/range/algorithm/find_if.hpp>
#include <QFileDialog>
#include <QMessageBox>
#include <QDomDocument>
#include <arx/Foreach.h>
#include <arx/ScopeExit.h>
#include <quazip.h>
#include <quazipfile.h>
#include <shiken/actors/WorkerRunner.h>
#include <shiken/actors/QuizListGetter.h>
#include <shiken/dao/DataAccessDriver.h>
#include <shiken/dao/SettingsDao.h>
#include <shiken/dao/QuizDao.h>
#include <shiken/dao/UserDao.h>
#include <shiken/utility/GuidCompressor.h>
#include <shiken/utility/Log.h>
#include <shiken/Shiken.h>
#include "ui_StartDialog.h"
#include "ui_StartDialogSingleUser.h"
#include "ProxySettingsDialog.h"

namespace shiken {
  StartDialog::StartDialog(Shiken *ctx, QWidget *parent): 
    QDialog(parent), 
    mUi(new Ui::StartDialog()), 
    mSingleUi(new Ui::StartDialogSingleUser()),
    mDummy(new QDialog(this)),
    mCtx(ctx)
  {
    if(mCtx->model()->settingsDao()->isSingleUser()) {
      initUi(mSingleUi.data(), mUi.data());
    } else {
      initUi(mUi.data(), mSingleUi.data());
    }

    mVersionLabel->setText(mVersionLabel->text().arg(SHIKEN_VERSION));

    /* User cannot press "Ok" unless he fills login field. */
    mUi->okButton->setDisabled(true);

    mLoginWidget->setLogin(mCtx->model()->settingsDao()->login());
    mLoginWidget->setPassword(mCtx->model()->settingsDao()->password());
  }

  template<class ActualUi, class DummyUi>
  void StartDialog::initUi(ActualUi *actualUi, DummyUi *dummyUi) {
    actualUi->setupUi(this);
    dummyUi->setupUi(mDummy.data());

    mVersionLabel = actualUi->versionLabel;
    mLoginWidget = actualUi->loginWidget;
  }

  StartDialog::~StartDialog() {
    return;
  }

  void StartDialog::on_openXmlButton_clicked() {
    assert(!mCtx->model()->settingsDao()->isSingleUser());

    QString targetPath = QFileDialog::getOpenFileName(this, QS("Выберите XML-файл..."), ".", QS("XML-файлы (*.xml *.zip)")); 
    if(targetPath.isEmpty())
      return;

    /* Load raw xml data. */
    QByteArray data;
    if(targetPath.endsWith(".zip")) {
      QuaZip quaZip(targetPath);
      quaZip.open(QuaZip::mdUnzip);
      quaZip.goToFirstFile();

      QuaZipFile zipFile(&quaZip);
      zipFile.open(QIODevice::ReadOnly);
      data = zipFile.readAll();
    } else {
      QFile xmlFile(targetPath);
      xmlFile.open(QIODevice::ReadOnly);
      data = xmlFile.readAll();
    }

    /* Load quiz. */
    UsersReply usersReply = mCtx->parseQuizXml(data, true);
    if(usersReply.isNull() || usersReply.users().size() == 0) {
      SHIKEN_LOG_MESSAGE("parseQuizXml() failed");
      QMessageBox::critical(this, QS("Ошибка"), QS("Неправильный формат XML-файла.")); /* TODO: Дополнительная информация доступна в лог-файле %1. */
      return;
    }

    /* Store quiz. */
    mCtx->storeQuizData(usersReply);

    /* Mark loaded quiz as current quiz. */
    mCtx->model()->settingsDao()->setQuizId(usersReply.quiz().quizId());

    accept();
  }
  
  void StartDialog::on_okButton_clicked() {
    assert(mLoginWidget->isFilled());

    setCursor(Qt::WaitCursor);
    setDisabled(true);

    ARX_SCOPE_EXIT(&) {
      this->unsetCursor();
      this->setDisabled(false);
    };

    mCtx->model()->settingsDao()->setLogin(mLoginWidget->login());
    mCtx->model()->settingsDao()->setPassword(mLoginWidget->password());
    
    QuizListGetter quizGetter(mCtx, QUrl(mCtx->model()->settingsDao()->targetUrl() + SHIKEN_QUIZZES_COMMAND));
    QList<Quiz> quizzes = runWorker(
      quizGetter, 
      this,
      "", 
      QS("Ошибка при соединении с сервером: %1\n\nВ случае, если ошибка будет повторяться, скачайте xml с описанием мероприятия и используйте его.")
    );

    if(quizzes.size() > 0) {
      /* Remove all quizzes from local database that were not loaded from XML. */
      mCtx->model()->quizDao()->removeByLoadedFromXml(false);

      /* Quizzes loaded from XML are prioritized, so we don't do updates here
       * if insert fails. */
      foreach(Quiz quiz, quizzes)
        mCtx->model()->quizDao()->insert(quiz);

      accept();
    }

    /* TODO: we should tell user about empty quizzes list. */
  }

  void StartDialog::on_proxySettingsLabel_linkActivated() {
    ProxySettingsDialog::query(mCtx, this);
  }

  
} // namespace shiken
