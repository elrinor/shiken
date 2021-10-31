#include "WorkerRunner.h"
#include <QProgressDialog>
#include <QEventLoop>
#include <QMessageBox>
#include <QPushButton>

namespace shiken {
  WorkerRunnerBase::WorkerRunnerBase(WorkerBase *worker, QWidget *widget, QString progressMessage, QString errorMessage, bool cancellable): 
    mWorker(worker), 
    mWidget(widget),
    mErrorMessage(errorMessage),
    mCancelButton(NULL)
  {
    assert(cancellable ? !progressMessage.isEmpty() : true);

    if(!progressMessage.isEmpty()) {
      mDialog.reset(new QProgressDialog(mWidget));
      mDialog->setWindowTitle(QS("Пожалуйста, подождите"));
      mDialog->setLabelText(progressMessage);

      /* Not setting this to zero may cause a lot of pain as if the dialog
       * is shown and then hidden before the minimumDuration milliseconds pass,
       * it will be forcibly shown again when they pass. */
      mDialog->setMinimumDuration(0); 

      if(!cancellable) {
        mDialog->setCancelButton(NULL);
      } else {
        mCancelButton = new QPushButton(QS("Отмена"));
        mDialog->setCancelButton(mCancelButton);

        /* We cheat and exploit our knowledge of QProgressDialog's internals 
         * here. We don't want the code that actually handles cancellation
         * to be executed as it hides the dialog and messes some things up.
         * This code resides in cancel() slot, which is always called
         * through meta object system. So, we just disconnect it. */
        disconnect(mDialog.data(), SIGNAL(canceled()), mDialog.data(), SLOT(cancel())); 
      }
      mDialog->setWindowFlags((mDialog->windowFlags() & Qt::WindowType_Mask) | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

      connect(worker, SIGNAL(started(int)), this, SLOT(onStarted(int)));
      connect(worker, SIGNAL(advanced(int)), this, SLOT(onAdvanced(int)));
      connect(worker, SIGNAL(finished()), mDialog.data(), SLOT(accept()));
      connect(mDialog.data(), SIGNAL(canceled()), this, SLOT(onCanceled()));
    } else {
      mLoop.reset(new QEventLoop());
      connect(worker, SIGNAL(finished()), mLoop.data(), SLOT(quit()));
    }

    /* User may fire two critical signals in a row. To prevent two error
     * message dialogs from popping up simultaneously, blocking connection is
     * used. Note that worker is always moved to a different thread before
     * executions, so the usage of a blocking connection doesn't cause 
     * deadlocks. */
    connect(worker, SIGNAL(critical(QString, QString)), this, SLOT(onCritical(QString, QString)), Qt::BlockingQueuedConnection);
  }

  WorkerRunnerBase::~WorkerRunnerBase() {
    return;
  }

  void WorkerRunnerBase::exec() const {
    if(mDialog.isNull()) {
      mLoop->exec();
    } else {
      mDialog->exec();
    }
  }

  void WorkerRunnerBase::onStarted(int maximum) const {
    assert(!mDialog.isNull());

    mDialog->setMinimum(0);
    mDialog->setMaximum(maximum);
    mDialog->setValue(0);
  }

  void WorkerRunnerBase::onAdvanced(int amount) const {
    assert(!mDialog.isNull());

    mDialog->setValue(mDialog->value() + amount);
  }

  void WorkerRunnerBase::onCritical(QString message, QString details) const {
    mWorker->setSucceeded(false);

    if(!mErrorMessage.isEmpty()) {
      if(!mDialog.isNull())
        mDialog->hide();

      QMessageBox::critical(
        mWidget,
        QS("Произошла ошибка"),
        mErrorMessage.arg(message) + 
          (details.isEmpty() ? QString() : QS("\n\nДополнительная информация: %1.").arg(details))
      );

      if(!mDialog.isNull())
        mDialog->show();
    }
  }

  void WorkerRunnerBase::onCanceled() const {
    mWorker->setCanceled(true);
    mCancelButton->setDisabled(true);
  }

} // namespace shiken
