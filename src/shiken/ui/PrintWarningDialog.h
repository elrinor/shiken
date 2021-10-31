#ifndef SHIKEN_PRINT_WARNING_DIALOG_H
#define SHIKEN_PRINT_WARNING_DIALOG_H

#include <shiken/config.h>
#include <QDialog>
#include <QScopedPointer>

namespace Ui {
  class PrintWarningDialog;
  class PrintWarningDialogSingleUser;
}

class QTimer;

namespace shiken {
  class Shiken;

// -------------------------------------------------------------------------- //
// PrintWarningDialog
// -------------------------------------------------------------------------- //
  /**
   * This dialog is displayed whenever a user tries to print something.
   */
  class PrintWarningDialog: public QDialog {
    Q_OBJECT;
  public:
    /**
     * @param parent                   Parent widget.
     */
    PrintWarningDialog(Shiken *ctx, QWidget *parent = 0);

    ~PrintWarningDialog();

  protected Q_SLOTS:
    void timerTimeout();

  protected:
    template<class ActualUi, class DummyUi>
    void initUi(ActualUi *actualUi, DummyUi *dummyUi);

  private:
    QScopedPointer<Ui::PrintWarningDialog> mUi;
    QScopedPointer<Ui::PrintWarningDialogSingleUser> mSingleUi;
    QScopedPointer<QDialog> mDummy;
    QScopedPointer<QTimer> mTimer;
    
    /** Initial label of the "Yes" button. */
    QPushButton *mYesButton;
    QString mYesLabel;

    /** Ticks left until "Yes" button can be pressed. */
    int mTicks;

    Shiken *mCtx;
  };

} // namespace shiken

#endif // SHIKEN_PRINT_WARNING_DIALOG_H
