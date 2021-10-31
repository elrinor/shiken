#ifndef SHIKEN_LOGIN_WIDGET_H
#define SHIKEN_LOGIN_WIDGET_H

#include <shiken/config.h>
#include <QWidget>
#include <QScopedPointer>

namespace Ui {
  class LoginWidget;
}

namespace shiken {
// -------------------------------------------------------------------------- //
// LoginWidget
// -------------------------------------------------------------------------- //
  /**
   * This widget is used in several places where login and password are
   * required.
   */
  class LoginWidget: public QWidget {
    Q_OBJECT;
  public:
    /**
     * @param parent                   Parent widget.
     */
    LoginWidget(QWidget *parent = NULL);

    ~LoginWidget();

    /**
     * @param login                    New login.
     */
    void setLogin(const QString &login);

    /**
     * @returns                        Login that was entered by the user.
     */
    QString login() const;

    /**
     * @param password                 New password.
     */
    void setPassword(const QString &password);

    /**
     * @returns                        Password that was entered by the user.
     */
    QString password() const;

    /**
     * @returns                        True if the login is non-empty, false
     *                                 otherwise.
     */
    bool isFilled() const;

  Q_SIGNALS:
    /** 
     * This signal is emitted when login line edit becomes empty or non-empty. 
     */
    void filled(bool isFilled);

    /**
     * This signal is emitted when return is pressed on one of the line edits.
     */
    void returnPressed();

  protected Q_SLOTS:
    void on_loginEdit_textChanged(const QString &text);

  protected:
    virtual bool eventFilter(QObject *watched, QEvent *e);

  private:
    QScopedPointer<Ui::LoginWidget> mUi;
  };

} // namespace shiken

#endif // SHIKEN_LOGIN_WIDGET_H