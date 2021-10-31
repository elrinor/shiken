#ifndef SHIKEN_START_DIALOG_H
#define SHIKEN_START_DIALOG_H

#include <shiken/config.h>
#include <QDialog>
#include <QScopedPointer>

class QLabel;

namespace Ui {
  class StartDialog;
  class StartDialogSingleUser;
}

namespace shiken {
  class Shiken;
  class LoginWidget;

// -------------------------------------------------------------------------- //
// StartDialog
// -------------------------------------------------------------------------- //
  /**
   * StartDialog is the very first dialog the user sees when he starts
   * generator.
   */
  class StartDialog: public QDialog {
    Q_OBJECT;
  public:
    /**
     * Constructor.
     *
     * @param ctx                      Application context.
     * @param parent                   Parent widget.
     */
    StartDialog(Shiken *ctx, QWidget *parent = NULL);

    ~StartDialog();

  protected Q_SLOTS:
    void on_okButton_clicked();
    void on_openXmlButton_clicked();
    void on_proxySettingsLabel_linkActivated();

  protected:
    template<class ActualUi, class DummyUi>
    void initUi(ActualUi *actualUi, DummyUi *dummyUi);

  private:
    QScopedPointer<Ui::StartDialog> mUi;
    QScopedPointer<Ui::StartDialogSingleUser> mSingleUi;
    QScopedPointer<QDialog> mDummy;

    QLabel *mVersionLabel;
    LoginWidget *mLoginWidget;

    Shiken *mCtx;
  };

} // namespace shiken

#endif // SHIKEN_START_DIALOG_H
