#include "PrintWarningDialog.h"
#include <QTimer>
#include "ui_PrintWarningDialog.h"
#include "ui_PrintWarningDialogSingleUser.h"
#include <shiken/dao/DataAccessDriver.h>
#include <shiken/dao/SettingsDao.h>
#include <shiken/Shiken.h>

namespace shiken {
  PrintWarningDialog::PrintWarningDialog(Shiken *ctx, QWidget *parent): 
    QDialog(parent), 
    mUi(new Ui::PrintWarningDialog()), 
    mSingleUi(new Ui::PrintWarningDialogSingleUser()),
    mDummy(new QDialog(this)),
    mTicks(10),
    mCtx(ctx)
  {
    if(mCtx->model()->settingsDao()->isSingleUser()) {
      initUi(mSingleUi.data(), mUi.data());
    } else {
      initUi(mUi.data(), mSingleUi.data());
    }

    mYesLabel = mYesButton->text();
    mYesButton->setDisabled(true);
    
    mTimer.reset(new QTimer);
    connect(mTimer.data(), SIGNAL(timeout()), this, SLOT(timerTimeout()));
    mTimer->start(1000);
    timerTimeout();
  }

  template<class ActualUi, class DummyUi>
  void PrintWarningDialog::initUi(ActualUi *actualUi, DummyUi *dummyUi) {
    actualUi->setupUi(this);
    dummyUi->setupUi(mDummy.data());

    mYesButton = actualUi->yesButton;
  }

  PrintWarningDialog::~PrintWarningDialog() {
    return;
  }

  void PrintWarningDialog::timerTimeout() {
    mTicks--;

    if(mTicks > 0) {
      mYesButton->setText(mYesLabel + QString(" (%1)").arg(mTicks));
    } else {
      mYesButton->setEnabled(true);
      mYesButton->setText(mYesLabel);
    }
  }

} // namespace shiken
