#include "LoginDialog.h"
#include <QPushButton>
#include "ui_LoginDialog.h"

namespace shiken {
  LoginDialog::LoginDialog(const QString &text, QWidget* parent): 
    QDialog(parent), mUi(new Ui::LoginDialog())
  {
    mUi->setupUi(this);
    mUi->label->setText(text);
  }

  LoginDialog::~LoginDialog() {
    return;
  }

  void LoginDialog::setLogin(const QString &login) {
    mUi->loginWidget->setLogin(login);
  }

  QString LoginDialog::login() const {
    return mUi->loginWidget->login();
  }

  QString LoginDialog::password() const {
    return mUi->loginWidget->password();
  }


} // namespace shiken
