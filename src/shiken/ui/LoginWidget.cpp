#include "LoginWidget.h"
#include <QEvent>
#include <QKeyEvent>
#include "ui_LoginWidget.h"

namespace shiken {
  LoginWidget::LoginWidget(QWidget *parent): 
    QWidget(parent), mUi(new Ui::LoginWidget())
  {
    mUi->setupUi(this);

    on_loginEdit_textChanged(mUi->loginEdit->text());
  }

  LoginWidget::~LoginWidget() {
    return;
  }

  bool LoginWidget::eventFilter(QObject * /*watched*/, QEvent *e) {
    bool filtered = false;
    if(e->type() == QEvent::KeyPress) {
      QKeyEvent* k = static_cast<QKeyEvent *>(e);
      switch(k->key()) {
      case Qt::Key_Return:
      case Qt::Key_Enter:
        if(isFilled())
          Q_EMIT returnPressed();
        filtered = true;
        break;
      default:
        break;
      }
    }
    return filtered;
  }

  void LoginWidget::setLogin(const QString &login) {
    mUi->loginEdit->setText(login);
  }

  QString LoginWidget::login() const {
    return mUi->loginEdit->text();
  }

  void LoginWidget::setPassword(const QString &password) {
    mUi->passwordEdit->setText(password);
  }

  QString LoginWidget::password() const {
    return mUi->passwordEdit->text();
  }

  bool LoginWidget::isFilled() const {
    return !login().isEmpty();
  }

  void LoginWidget::on_loginEdit_textChanged(const QString & /*text*/) {
    Q_EMIT filled(!login().isEmpty());
  }


} // namespace shiken
