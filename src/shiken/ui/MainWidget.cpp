#include "MainWidget.h"
#include <cassert>
#include <boost/tuple/tuple.hpp>
#include <QPrintDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QTemporaryFile>
#include <QProcess>
#include <arx/Foreach.h>
#include <shiken/dao/DataAccessDriver.h>
#include <shiken/dao/SettingsDao.h>
#include <shiken/dao/UserDao.h>
#include <shiken/dao/PageDao.h>
#include <shiken/dao/ScanDao.h>
#include <shiken/dao/QuizDao.h>
#include <shiken/actors/ScanRecognizer.h>
#include <shiken/actors/ScanListGetter.h>
#include <shiken/actors/ScanUploader.h>
#include <shiken/actors/VersionGetter.h>
#include <shiken/actors/UpdateDownloader.h>
#include <shiken/actors/FormPrinter.h>
#include <shiken/actors/TestFormGenerator.h>
#include <shiken/actors/WorkerRunner.h>
#include <shiken/utility/SqlQueryModel.h>
#include <shiken/entities/ScanData.h>
#include <shiken/utility/BarcodeProcessor.h>
#include <shiken/utility/Log.h>
#include <shiken/Shiken.h>
#include "ui_MainWidget.h"
#include "ui_MainWidgetSingleUser.h"
#include "LoginDialog.h"
#include "ImageDialog.h"
#include "PrintConditionsDialog.h"
#include "PrintWarningDialog.h"
#include "ProxySettingsDialog.h"

#ifdef Q_WS_WIN
#   include <shiken/actors/GsPrinter.h>
#endif

namespace shiken {
  MainWidget::MainWidget(Shiken *ctx): 
    mUi(new Ui::MainWidget()), 
    mSingleUi(new Ui::MainWidgetSingleUser()),
    mDummy(new QMainWindow(this)),
    mCtx(ctx), 
    mInvalidUserButton(0), 
    mDuplicateScanButton(0) 
  {
    if(mCtx->model()->settingsDao()->isSingleUser()) {
      initUi(mSingleUi.data(), mUi.data());
    } else {
      initUi(mUi.data(), mSingleUi.data());
    }
    
    mPagesPerStudentSpinBox->setValue(mCtx->model()->settingsDao()->pageCount());
    mSendAddressLabel->setText(mCtx->model()->settingsDao()->targetUrl());

    /* Initialize views and models. */
    QString quizIdString = QString::number(mCtx->model()->settingsDao()->quizId());

    /* User view & model. */
    mUserModel = new SqlQueryModel(this);
    connect(mCtx->model()->userDao(), SIGNAL(changed()), mUserModel, SLOT(update()));
    connect(mCtx->model()->pageDao(), SIGNAL(changed()), mUserModel, SLOT(update()));
    connect(mUserModel, SIGNAL(updated()), this, SLOT(updateUserModel()));
    mUserModel->setQuery(QString(" \
      SELECT NULL, users.compressed_guid, users.login, COUNT(pages.page_id) \
      FROM users NATURAL LEFT JOIN pages \
      WHERE users.is_unknown = 0 AND users.quiz_id = " + quizIdString + "\
      GROUP BY users.compressed_guid \
      ORDER BY users.login \
    "), mCtx->model()->connection());
    mUserModel->setHeaderData(0, Qt::Horizontal, "");
    mUserModel->setHeaderData(1, Qt::Horizontal, QS("Хеш"));
    mUserModel->setHeaderData(2, Qt::Horizontal, QS("Логин"));
    mUserModel->setHeaderData(3, Qt::Horizontal, QS("Бланков напечатано"));
    
    mUi->userView->setModel(mUserModel);
    mUi->userView->hideColumn(1);
    for(int i = 0; i < mUserModel->columnCount(); i++)
      mUi->userView->resizeColumnToContents(i);

    mUi->userView->installEventFilter(this);

    /* User scans view & model. */
    mUserScanModel = new SqlQueryModel(this);
    connect(mCtx->model()->userDao(), SIGNAL(changed()), mUserScanModel, SLOT(update()));
    connect(mCtx->model()->pageDao(), SIGNAL(changed()), mUserScanModel, SLOT(update()));
    connect(mCtx->model()->scanDao(), SIGNAL(changed()), mUserScanModel, SLOT(update()));
    connect(mUserScanModel, SIGNAL(updated()), this, SLOT(updateUserScanModel()));
    mUserScanModel->setQuery(QString(" \
      SELECT users.login, COUNT(pages.page_id), COUNT(scans.page_id) \
      FROM users NATURAL LEFT JOIN pages NATURAL LEFT JOIN scans \
      WHERE users.quiz_id = " + quizIdString + "\
      GROUP BY users.compressed_guid \
      ORDER BY users.login \
    "), mCtx->model()->connection());
    mUserScanModel->setHeaderData(0, Qt::Horizontal, QS("Логин"));
    mUserScanModel->setHeaderData(1, Qt::Horizontal, QS("Бланков напечатано"));
    mUserScanModel->setHeaderData(2, Qt::Horizontal, QS("Бланков загружено"));

    mUi->userScanView->setModel(mUserScanModel);
    for(int i = 0; i < mUserScanModel->columnCount(); i++)
      mUi->userScanView->resizeColumnToContents(i);

    /* Scan view & model. */
    mScanModel = new SqlQueryModel(this);
    connect(mCtx->model()->scanDao(), SIGNAL(changed()), mScanModel, SLOT(update()));
    connect(mScanModel, SIGNAL(updated()), this, SLOT(updateScanModel()));
    /* Note: for some unknown reason using users.login instead of just login
     * in this query leads to an error. */
    mScanModel->setQuery(QString(" \
      SELECT scans.file_name, login, scans.state \
      FROM scans NATURAL LEFT JOIN (pages NATURAL JOIN users) \
      WHERE scans.quiz_id = " + quizIdString + " \
      ORDER BY state, scans.file_name \
    "), mCtx->model()->connection());
    mScanModel->setHeaderData(0, Qt::Horizontal, QS("Файл"));
    mScanModel->setHeaderData(1, Qt::Horizontal, QS("Логин"));
    mScanModel->setHeaderData(2, Qt::Horizontal, QS("Состояние"));

    mScanView->setModel(mScanModel);
    for(int i = 0; i < mScanModel->columnCount(); i++)
      mScanView->resizeColumnToContents(i);
    
    mScanView->installEventFilter(this);

    /* Single-user UI. */
    mSingleUi->loginEdit1->setText(mCtx->model()->settingsDao()->login());
    mSingleUi->loginEdit2->setText(mCtx->model()->settingsDao()->login());
    connect(mCtx->model()->pageDao(), SIGNAL(changed()), this, SLOT(updatePageCount()));
    connect(mCtx->model()->scanDao(), SIGNAL(changed()), this, SLOT(updateScanCount()));
    updatePageCount();
    updateScanCount();
    if(mCtx->model()->settingsDao()->isSingleUser()) {
      mSingleUser = mCtx->model()->userDao()->selectByLoginAndQuizId(mCtx->model()->settingsDao()->login(), mCtx->model()->settingsDao()->quizId());

      assert(!mSingleUser.isNull());

      mSelectedCompressedGuids.insert(mSingleUser.compressedGuid());
    }

    /* Set up timer for getting scans. */
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(getScans()));
    mTimer->start(mCtx->model()->settingsDao()->scansUpdateIntervalMsecs());
    QTimer::singleShot(0, this, SLOT(getScans()));

    /* Check for updates after getting back into the event loop. */
    QTimer::singleShot(0, this, SLOT(checkUpdates()));

    setWindowTitle(windowTitle().arg(SHIKEN_VERSION));
  }

  template<class ActualUi, class DummyUi>
  void MainWidget::initUi(ActualUi *actualUi, DummyUi *dummyUi) {
    actualUi->setupUi(this);
    dummyUi->setupUi(mDummy.data());

    mPagesPerStudentSpinBox = actualUi->pagesPerStudentSpinBox;
    mSendAddressLabel = actualUi->sendAddressLabel;
    mScanView = actualUi->scanView;
  }

  MainWidget::~MainWidget() {
    return;
  }

  void MainWidget::checkUpdates() {
    SHIKEN_LOG_MESSAGE("checkUpdates()");

    /* Get current version number. */
    VersionGetter versionGetter(mCtx, QUrl(mCtx->model()->settingsDao()->targetUrl() + SHIKEN_VERSION_COMMAND));
    int version = runWorker(versionGetter);
    bool hasUpdate = version > SHIKEN_INT_VERSION;
    SHIKEN_LOG_MESSAGE("version == " << version << "; hasUpdate == " << hasUpdate);
    if(!hasUpdate) 
      return;

    QMessageBox::information(this, QS(""), QS("Эта версия программы устарела. \nПрограмма будет автоматически обновлена."));

    /* Download new binary. */
    UpdateDownloader updateDownloader(mCtx, mCtx->model()->settingsDao()->binaryUrl());
    QByteArray newBinary = runWorker(updateDownloader, this, QS("Выполняется обновление программы..."));
    if(!updateDownloader.succeeded()) {
      SHIKEN_LOG_MESSAGE("Update download failed");
      QMessageBox::critical(this, QS("Ошибка"), QS("При загрузке обновления произошла ошибка. Будет использована старая версия программы."));
      return;
    }
    SHIKEN_LOG_MESSAGE("Update downloaded, size == " << newBinary.size());

    /* Write new binary into a temporary file. */ 
    QTemporaryFile newBinaryFile;
    newBinaryFile.open();
    qint64 written = newBinaryFile.write(newBinary);
    newBinaryFile.close();
    SHIKEN_LOG_MESSAGE("Written " << written << " bytes to " << newBinaryFile.fileName() << ". Error == " << newBinaryFile.errorString());
    Q_UNUSED(written);

#ifdef Q_WS_WIN
    QString restarterPath = QDir::temp().filePath("restarter_win32.exe");
    QString applicationPath = QDir::toNativeSeparators(QApplication::applicationFilePath());

    SHIKEN_LOG_MESSAGE("Unpacking restarter to " << restarterPath);

    /* Remove old restarter if it exists. */
    if(QFile::exists(restarterPath)) {
      SHIKEN_LOG_MESSAGE("Restarter file already exists, removing.");

      if(!QFile::remove(restarterPath)) {
        SHIKEN_LOG_MESSAGE("Failed to remove restarter file, setting permissions and trying again.");
        QFile::setPermissions(restarterPath, QFile::permissions(restarterPath) | QFile::WriteUser);
        bool removed = QFile::remove(restarterPath);

        SHIKEN_LOG_MESSAGE("Removed == " << removed);
        Q_UNUSED(removed);
      }
    }

    /* Spawn restarter binary & execute it. */
    if(QFile::copy(":/restarter_win32.exe", restarterPath)) {
      SHIKEN_LOG_MESSAGE("Restarter file copied");

      QStringList args = QStringList()
        << QString::number(QApplication::applicationPid())
        << QDir::toNativeSeparators(newBinaryFile.fileName())
        << applicationPath
        << applicationPath.left(applicationPath.size() - 4) + QString::number(version) + ".exe";

      SHIKEN_LOG_MESSAGE("running: " << restarterPath << " with args " << args);

      QProcess process;
      process.start(restarterPath, args);
      process.waitForFinished(-1);

      SHIKEN_LOG_MESSAGE("Restarter finished with exit code " << process.exitCode() << ", exit status " << process.exitStatus() << " and error " << process.error() << "(" << process.errorString() << ")");
    } else {
      SHIKEN_LOG_MESSAGE("Failed to copy restarter file.");
    }
#else
    /* TODO */
#endif

    SHIKEN_LOG_MESSAGE("Update failed.");

    /* Well, we should be already killed here. */
    QMessageBox::critical(this, QS("Ошибка"), QS("При обновлении произошла ошибка. Будет использована старая версия программы."));
  }

  void MainWidget::getScans() {
    int quizId = mCtx->model()->settingsDao()->quizId();

    ScanListGetter scanListGetter(
        mCtx, 
        QUrl(mCtx->model()->settingsDao()->targetUrl() + SHIKEN_SCANS_COMMAND), 
        mCtx->model()->quizDao()->selectById(quizId)
    );

    QList<ExtendedScan> scans = runWorker(scanListGetter);

    mCtx->model()->blockSignals();

    foreach(ExtendedScan &scan, scans) {
      Scan existingScan = mCtx->model()->scanDao()->selectByHashAndQuizId(scan.hash(), quizId);
      if(existingScan.state() == scan.state())
        continue;

      if(existingScan.isNull()) {
        User user = mCtx->model()->userDao()->selectByCompressedGuidAndQuizId(scan.compressedGuid(), quizId);
        if(user.isNull()) {
          user = User(-1, quizId, SHIKEN_UNKNOWN_LOGIN, QString(), scan.compressedGuid(), true);
          mCtx->model()->userDao()->insert(user);
        }
        assert(user.userId() != -1);

        Page page = mCtx->model()->pageDao()->selectByUserIdAndPrintTime(user.userId(), scan.printTime());
        if(page.isNull()) {
          page = Page(-1, user.userId(), scan.printTime());
          mCtx->model()->pageDao()->insert(page);
        }
        assert(page.pageId() != -1);

        scan.setPageId(page.pageId());
        mCtx->model()->scanDao()->insert(scan);
      } else {
        mCtx->model()->scanDao()->update(scan);
      }
    }

    mCtx->model()->unblockSignals();
  }

  void MainWidget::on_selectAllButton_clicked() {
    for(int row = 0; row < mUserModel->rowCount(); row++) {
      mSelectedCompressedGuids.insert(mUserModel->index(row, 1).data().toString());
      mUserModel->setData(mUserModel->index(row, 0), Qt::Checked, Qt::CheckStateRole);
    }
  }

  void MainWidget::on_clearSelectionButton_clicked() {
    mSelectedCompressedGuids.clear();
    for(int row = 0; row < mUserModel->rowCount(); row++)
      mUserModel->setData(mUserModel->index(row, 0), Qt::Unchecked, Qt::CheckStateRole);
  }

  void MainWidget::on_userView_clicked(QModelIndex idx) {
    if(!idx.isValid())
      return;

    if(idx.column() == 0) {
      Qt::CheckState newState = static_cast<Qt::CheckState>(mUserModel->data(idx, Qt::CheckStateRole).toInt()) == Qt::Checked ? Qt::Unchecked : Qt::Checked;
      mUserModel->setData(idx, newState, Qt::CheckStateRole);

      QString compressedGuid = mUserModel->index(idx.row(), 1).data().toString();
      if(newState == Qt::Checked)
        mSelectedCompressedGuids.insert(compressedGuid);
      else
        mSelectedCompressedGuids.remove(compressedGuid);
    }
  }

#ifdef Q_WS_WIN
  void MainWidget::on_printButton_clicked() {
    SHIKEN_LOG_MESSAGE("on_printButton_clicked()");

    if(mSelectedCompressedGuids.empty()) {
      QMessageBox::critical(this, QS("Ошибка"), QS("Не выбрано ни одного ученика."));
      return;
    }

    PrintConditionsDialog conditionsDlg(QS("Подтвердите, пожалуста, что:"), this);
    if(conditionsDlg.exec() != QDialog::Accepted)
      return;

    PrintWarningDialog warningDlg(mCtx, this);
    if(warningDlg.exec() != QDialog::Accepted)
      return;

    QPrinter printer;

    QPrintDialog printDialog(&printer, this);
    if(printDialog.exec() != QDialog::Accepted)
      return;
    
    printer.setNumCopies(1);
    printer.setPrintRange(QPrinter::AllPages);
    printer.setFullPage(true); /* For crippled printer not to ruin everything. */
    printer.setPaperSize(QPrinter::A4);
      
    QString printerName = printer.printerName();
    printer.setOutputFormat(QPrinter::PdfFormat);

    /* Prepare a temporary file. We need to open it first to generate fileName. 
     * It will be removed automatically when tempFile object goes out of scope. */
    QTemporaryFile tempFile;
    tempFile.open();
    tempFile.close();

    printer.setOutputFileName(tempFile.fileName());
    print(printer);

    setEnabled(false);
    runWorker(GsPrinter(mCtx, tempFile.fileName(), printerName), this, "", QS("Ошибка при печати: %1"));
    setEnabled(true);
  }
#endif

  void MainWidget::on_printPdfButton_clicked() {
    SHIKEN_LOG_MESSAGE("on_printPdfButton_clicked()");

    if(mSelectedCompressedGuids.empty()) {
      QMessageBox::critical(this, QS("Ошибка"), QS("Не выбрано ни одного ученика."));
      return;
    }

    PrintConditionsDialog conditionsDlg(QS("При распечатке полученного pdf-файла убедитесь, пожалуйста, что:"), this);
    if(conditionsDlg.exec() != QDialog::Accepted)
      return;

    PrintWarningDialog warningDlg(mCtx, this);
    if(warningDlg.exec() != QDialog::Accepted)
      return;

    QString targetPath = QFileDialog::getSaveFileName(this, QS("Сохранить как PDF-файл..."), ".", QS("PDF-файлы (*.pdf)")); 
    if(targetPath.isEmpty())
      return;

    QPrinter printer;
    printer.setOutputFileName(targetPath);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::A4);

    print(printer);
  }

  void MainWidget::print(QPrinter& printer) {
    SHIKEN_LOG_MESSAGE("print(" << printer.printerName() << ")");

    QList<User> users;
    foreach(const QString &compressedGuid, mSelectedCompressedGuids)
      users.push_back(mCtx->model()->userDao()->selectByCompressedGuidAndQuizId(compressedGuid, mCtx->model()->settingsDao()->quizId()));
    qSort(users.begin(), users.end(), [](const User &l, const User &r) -> bool { return l.login() < r.login(); });

    FormPrinter formPrinter(mCtx, printer);

    TestFormGenerator* formGenerator = new TestFormGenerator(mCtx->model()->quizDao()->selectById(mCtx->model()->settingsDao()->quizId()), users, mPagesPerStudentSpinBox->value());
    connect(formGenerator, SIGNAL(pagePrinted(Page)), this, SLOT(pagePrinted(Page))); 
    formPrinter(formGenerator);

    /* We don't want the DAO to fire an update for each call of pagePrinted().
     * That's why we block signals for the time of page generation. */
    mCtx->model()->blockSignals();
    runWorker(formPrinter, this, QS("Выполняется генерация бланков..."), QS("Ошибка при генерации бланков: %1"));
    mCtx->model()->unblockSignals();
  }

  void MainWidget::pagePrinted(Page page) {
    SHIKEN_LOG_MESSAGE("pagePrinted(" << page.pageId() << ", " << page.userId() << ", " << page.printTime() << ")");

    assert(!page.isNull());

    mCtx->model()->pageDao()->insert(page);
  }

  void MainWidget::on_addScansButton_clicked() {
    SHIKEN_LOG_MESSAGE("on_addScansButton_clicked()");

    QStringList targets = QFileDialog::getOpenFileNames(this, QS("Открыть отсканированные бланки..."), ".", QS("Все файлы (*.*)")); 
    if(targets.isEmpty())
      return;

    QList<Scan> scans;
    foreach(QString fileName, targets) {
      Scan scan = mCtx->model()->scanDao()->selectByFileNameAndQuizId(fileName, mCtx->model()->settingsDao()->quizId());
      if(!scan.isNull())
        continue;
      scan.setQuizId(mCtx->model()->settingsDao()->quizId());
      scan.setFileName(fileName);
      scan.setState(Scan::UNPROCESSED);
      scans.push_back(scan);
    }

    mInvalidUserButton = 0;
    mDuplicateScanButton = 0;
    ScanRecognizer scanRecognizer(mCtx, scans);
    connect(&scanRecognizer, SIGNAL(scanRecognized(Scan, QString)), this, SLOT(scanRecognized(Scan, QString)), Qt::BlockingQueuedConnection);
    runWorker(scanRecognizer, this, QS("Выполняется обработка бланков..."), QS("Ошибка при обработке бланков: %1"));
  }

  void MainWidget::scanRecognized(Scan scan, QString barcode) {
    SHIKEN_LOG_MESSAGE("scanRecognized(" << scan.fileName() << ", " << barcode << ")");

    if(scan.hash().isNull())
      return;

    Scan existingScan = mCtx->model()->scanDao()->selectByHashAndQuizId(scan.hash(), scan.quizId());
    if(!existingScan.isNull()) {
      existingScan.setFileName(scan.fileName());
      mCtx->model()->scanDao()->update(existingScan);
      return;
    }

    if(!barcode.isNull()) {
      QString compressedGuid;
      QDateTime printTime;
      boost::tie(compressedGuid, printTime) = BarcodeProcessor::decompose(barcode);

      User user = mCtx->model()->userDao()->selectByCompressedGuidAndQuizId(compressedGuid, scan.quizId());
      if(user.isNull()) {
        int button = mInvalidUserButton != 0 ? mInvalidUserButton : QMessageBox::question(
          this,
          QS("Внимание"),
          QS("Согласно данным на компьютере, бланк \"%1\" относится к тесту, отличному от текущего. Вы уверены, что этот бланк необходимо отправить на сервер?")
            .arg(scan.fileName()),
          QMessageBox::Yes | QMessageBox::YesAll | QMessageBox::No | QMessageBox::NoAll,
          QMessageBox::No
        );

        if(button == QMessageBox::YesAll || button == QMessageBox::NoAll)
          mInvalidUserButton = button;

        if(button != QMessageBox::Yes && button != QMessageBox::YesAll)
          return;

        user = User(-1, scan.quizId(), SHIKEN_UNKNOWN_LOGIN, QString(), compressedGuid, true);
        mCtx->model()->userDao()->insert(user);
      }

      Page page = mCtx->model()->pageDao()->selectByUserIdAndPrintTime(user.userId(), printTime);
      if(page.isNull()) {
        page = Page(-1, user.userId(), printTime);
        mCtx->model()->pageDao()->insert(page);
        
        assert(page.pageId() != -1);
      }
      scan.setPageId(page.pageId());

      QList<Scan> scans = mCtx->model()->scanDao()->selectByPageId(page.pageId());
      if(scans.size() > 0) {
        int button = mDuplicateScanButton != 0 ? mDuplicateScanButton : QMessageBox::question(
          this,
          QS("Внимание"), 
          QS("Штрих-код бланка \"%1\" совпадает со штрих-кодом уже загруженного бланка \"%2\". Вы уверены, что этот бланк необходимо отправить на сервер?")
            .arg(scan.fileName()).arg(scans.front().fileName()),
          QMessageBox::Yes | QMessageBox::YesAll | QMessageBox::No | QMessageBox::NoAll,
          QMessageBox::No
        );

        if(button == QMessageBox::YesAll || button == QMessageBox::NoAll)
          mDuplicateScanButton = button;

        if(button != QMessageBox::Yes && button != QMessageBox::YesAll)
          return;
      }
    }

    mCtx->model()->scanDao()->insert(scan);
  }

  void MainWidget::on_sendScansButton_clicked() {
    SHIKEN_LOG_MESSAGE("on_sendScansButton_clicked()");

    QList<Scan> scans = mCtx->model()->scanDao()->selectByQuizIdAndStates(mCtx->model()->settingsDao()->quizId(), QSet<Scan::State>() << Scan::RECOGNIZED);

    if(scans.size() == 0) {
      QMessageBox::critical(this, QS("Ошибка"), QS("Нет распознанных бланков для отправки."));
      return;
    }

    LoginDialog loginDialog(QS("Введите логин и пароль школы."), this);
    loginDialog.setLogin(mCtx->model()->settingsDao()->login());
    if(loginDialog.exec() != QDialog::Accepted)
      return;
    mCtx->model()->settingsDao()->setLogin(loginDialog.login());
    mCtx->model()->settingsDao()->setPassword(loginDialog.password());

    mScanNotFoundButton = 0;
    ScanUploader scanUploader(
      mCtx,
      mCtx->model()->quizDao()->selectById(mCtx->model()->settingsDao()->quizId()), 
      scans, 
      QUrl(mCtx->model()->settingsDao()->targetUrl() + SHIKEN_UPLOAD_COMMAND)
    );
    connect(&scanUploader, SIGNAL(scanUploaded(Scan)), this, SLOT(scanUploaded(Scan)));
    connect(&scanUploader, SIGNAL(scanNotFound(Scan)), this, SLOT(scanNotFound(Scan)), Qt::BlockingQueuedConnection);
    runWorker(
      scanUploader, 
      this,
      QS("Выполняется загрузка бланков на сервер..."), 
      QS("При соединении с сервером произошла ошибка: %1.\n\nЕсли ошибка будет повторяться, загрузите архив с отсканированными работами через web-интерфейс."), 
      true
    );

    if(scanUploader.succeeded()) {
      if(!mCtx->model()->scanDao()->selectByQuizIdAndStates(mCtx->model()->settingsDao()->quizId(), QSet<Scan::State>() << Scan::RECOGNIZED).empty()) {
        SHIKEN_LOG_MESSAGE("Some of the scans were not uploaded.");
        QMessageBox::information(this, QS("Внимание"), QS("Некоторые бланки не были загружены на сервер. Попробуйте загрузить бланки еще раз."));
      } else {
        if(!mCtx->model()->scanDao()->selectByQuizIdAndStates(mCtx->model()->settingsDao()->quizId(), QSet<Scan::State>() << Scan::UNRECOGNIZED).empty()) {
          SHIKEN_LOG_MESSAGE("All recognized scans were uploaded. Unrecognized scans exist.");
          if(QMessageBox::question(
            this, 
            QS("Внимание"), 
            QS("Все распознанные бланки были успешно загружены на сервер. У вас остались нераспознанные бланки. Удалить из списка все нераспознанные бланки?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
          ) == QMessageBox::Yes) {
            SHIKEN_LOG_MESSAGE("Removing unrecognized scans...");
            mCtx->model()->scanDao()->removeByQuizIdAndStates(mCtx->model()->settingsDao()->quizId(), QSet<Scan::State>() << Scan::UNRECOGNIZED);
          }
        } else {
          SHIKEN_LOG_MESSAGE("All recognized scans were uploaded. There are no unrecognized scans.");
          QMessageBox::information(this, QS("Внимание"), QS("Все бланки были успешно загружены на сервер"));
        }
      }
    }
  }

  void MainWidget::scanUploaded(Scan scan) {
    SHIKEN_LOG_MESSAGE("scanUploaded(" << scan.fileName() << ")");

    scan.setState(Scan::UPLOADED);
    mCtx->model()->scanDao()->update(scan);
  }

  void MainWidget::scanNotFound(Scan scan) {
    SHIKEN_LOG_MESSAGE("scanNotFound(" << scan.fileName() << ")");

    int button = mScanNotFoundButton != 0 ? mScanNotFoundButton : QMessageBox::question(
      this,
      QS("Внимание"), 
      QS("Файл \"%1\" не найден. Удалить этот файл из списка?").arg(scan.fileName()),
      QMessageBox::Yes | QMessageBox::YesAll | QMessageBox::No | QMessageBox::NoAll,
      QMessageBox::No
    );

    if(button == QMessageBox::YesAll || button == QMessageBox::NoAll)
      mScanNotFoundButton = button;

    if(button == QMessageBox::Yes || button == QMessageBox::YesAll)
      mCtx->model()->scanDao()->remove(scan);
  }

  void MainWidget::updateUserModel() {
    for(int row = 0; row < mUserModel->rowCount(); row++) {
      QString compressedGuid = mUserModel->index(row, 1).data().toString().mid(1);
      mUserModel->setData(mUserModel->index(row, 1), compressedGuid);
      mUserModel->setData(mUserModel->index(row, 0), mSelectedCompressedGuids.contains(compressedGuid) ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);

      mUserModel->item(row, 0)->setFlags(Qt::ItemIsEnabled);
      for(int column = 1; column < mUserModel->columnCount(); column++)
        mUserModel->item(row, column)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }
  }

  void MainWidget::updateScanModel() {
    for(int row = 0; row < mScanModel->rowCount(); row++) {
      QModelIndex stateIdx = mScanModel->index(row, 2);

      Scan::State state = static_cast<Scan::State>(stateIdx.data().toInt());
      mScanModel->setData(stateIdx, Scan::stateString(state));

      QBrush bgBrush;
      switch(state) {
      case Scan::SERVER_FAILED:
      case Scan::SERVER_WRONG:
#if 0
        bgBrush = QBrush(QColor(255, 196, 196));
        break;
#endif
      case Scan::SERVER_RECOGNIZED:
      case Scan::SERVER_MANUAL:
        bgBrush = QBrush(QColor(196, 255, 196));
        break;
      default:
        bgBrush = QBrush(Qt::white);
        break;
      }

      for(int column = 0; column < mScanModel->columnCount(); column++) {
        QStandardItem *item = mScanModel->item(row, column);
        item->setData(bgBrush, Qt::BackgroundRole);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
      }
    }
  }

  void MainWidget::updateUserScanModel() {
    for(int row = 0; row < mUserScanModel->rowCount(); row++)
      for(int column = 0; column < mUserScanModel->columnCount(); column++)
        mUserScanModel->item(row, column)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  }
  
  void MainWidget::updatePageCount() {
    DataAccessDriver *model = mCtx->model();

    int pageCount = model->pageDao()->selectByUserId(mSingleUser.userId()).size();

    mSingleUi->pageCountEdit1->setText(QString::number(pageCount));
    mSingleUi->pageCountEdit2->setText(QString::number(pageCount));
  }

  void MainWidget::updateScanCount() {
    DataAccessDriver *model = mCtx->model();

    int scanCount = model->scanDao()->selectByUserId(mSingleUser.userId()).size();

    mSingleUi->scanCountEdit2->setText(QString::number(scanCount));
  }

  void MainWidget::on_aboutAction_triggered() {
    QMessageBox::information(this, QS("О программе"), QS("Программное обеспечение разработано ООО \"Селект ЛТД\" по заказу НОУ МЦНМО.\nВерсия %1 от %2 %3.").arg(SHIKEN_VERSION).arg(__DATE__).arg(__TIME__));
  }

  void MainWidget::on_helpAction_triggered() {
    QDesktopServices::openUrl(mCtx->model()->settingsDao()->helpUrl());
    QMessageBox::information(
      this, 
      QS("Помощь"), 
      QS("Подробная информация об использовании программного обеспечения доступна в исходном архиве (файл generator.pdf), или по адресу %1").arg(mCtx->model()->settingsDao()->helpUrl())
    );
  }

  void MainWidget::on_proxySettingsAction_triggered() {
    ProxySettingsDialog::query(mCtx, this);
  }

  void MainWidget::deleteScan(QString fileName) {
    Scan scan = mCtx->model()->scanDao()->selectByFileNameAndQuizId(fileName, mCtx->model()->settingsDao()->quizId());

    if(scan.state() >= Scan::UPLOADED) {
      QMessageBox::critical(this, QS("Ошибка"), QS("Этот бланк уже отправлен. Вы не можете его удалить."));
      return;
    }

    mCtx->model()->scanDao()->remove(scan);
  }

  void MainWidget::on_removeScansButton_clicked() {
    QModelIndex idx = mScanView->currentIndex();
    if(!idx.isValid())
      return;

    QString fileName = idx.sibling(idx.row(), 0).data().toString();
    deleteScan(fileName);
  }

  bool MainWidget::eventFilter(QObject* sender, QEvent* e) {
    bool filtered = false;
    if(sender == mScanView) {
      if(e->type() == QEvent::KeyPress) {
        QKeyEvent* k = static_cast<QKeyEvent *>(e);
        switch(k->key()) {
        case Qt::Key_Delete:
          on_removeScansButton_clicked();
          filtered = true;
          break;
        default:
          break;
        }
      }
    }
    return filtered;
  }

  void MainWidget::on_scanView_doubleClicked(QModelIndex idx) {
    if(idx.isValid()) {
      QString fileName = idx.sibling(idx.row(), 0).data().toString();

      if(!QFile::exists(fileName)) {
        QMessageBox::critical(this, QS("Ошибка!"), QS("Файл \"") + fileName + QS("\" не найден. Возможно, он был загружен с другого компьютера."));
      } else {
        ImageDialog dlg(fileName);
        dlg.exec();
      }
    }
  }

} // namespace shiken

