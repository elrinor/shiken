#ifndef SHIKEN_PRINT_CONDITIONS_DIALOG_H
#define SHIKEN_PRINT_CONDITIONS_DIALOG_H

#include <shiken/config.h>
#include <QDialog>
#include <QScopedPointer>

namespace Ui {
  class PrintConditionsDialog;
}

namespace shiken {
// -------------------------------------------------------------------------- //
// PrintConditionsDialog
// -------------------------------------------------------------------------- //
  /**
   * This dialog is displayed whenever a user tries to print something.
   *
   * It contains several checkboxes that the user must check in order to
   * proceed.
   */
  class PrintConditionsDialog: public QDialog {
    Q_OBJECT;
  public:
    /**
     * @param message                  Message for this dialog. This is the
     *                                 line of text that will be displayed 
     *                                 above the checkboxes.
     * @param parent                   Parent widget.
     */
    PrintConditionsDialog(const QString &message, QWidget *parent = 0);

    ~PrintConditionsDialog();

  private slots:
    void checkBoxStateChanged();

  private:
    QScopedPointer<Ui::PrintConditionsDialog> mUi;
  };

} // namespace shiken

#endif // SHIKEN_PRINT_CONDITIONS_DIALOG_H
