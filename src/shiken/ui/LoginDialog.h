#ifndef SHIKEN_LOGIN_DIALOG_H
#define SHIKEN_LOGIN_DIALOG_H

#include <shiken/config.h>
#include <QDialog>
#include <QScopedPointer>

namespace Ui {
  class LoginDialog;
}

namespace shiken {
// -------------------------------------------------------------------------- //
// LoginDialog
// -------------------------------------------------------------------------- //
  /**
   * This is a login/password request dialog.
   */
  class LoginDialog: public QDialog {
    Q_OBJECT;
  public:
    /**
     * @param text                     Message that will be displayed above
     *                                 the line edits for login and password.
     * @param parent                   Parent widget.
     */
    LoginDialog(const QString &text, QWidget *parent = 0);

    ~LoginDialog();

    /**
     * @param login                    New login.
     */
    void setLogin(const QString &login);

    /**
     * @returns                        Login that was entered by the user.
     */
    QString login() const;

    /**
     * @returns                        Password that was entered by the user.
     */
    QString password() const;

  private:
    QScopedPointer<Ui::LoginDialog> mUi;
  };

} // namespace shiken

#endif // SHIKEN_LOGIN_DIALOG_H
