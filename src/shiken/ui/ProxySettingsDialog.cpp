#include "ProxySettingsDialog.h"
#include <QNetworkProxy>
#include <shiken/dao/DataAccessDriver.h>
#include <shiken/dao/SettingsDao.h>
#include <shiken/network/ConnectionManager.h>
#include <shiken/Shiken.h>
#include "ui_ProxySettingsDialog.h"

namespace shiken {
  ProxySettingsDialog::ProxySettingsDialog(Shiken *ctx, QWidget *parent):
    QDialog(parent),
    mUi(new Ui::ProxySettingsDialog()),
    mCtx(ctx)
  {
    mUi->setupUi(this);

    QNetworkProxy userProxy = mCtx->model()->settingsDao()->userProxy();
    mUi->addressEdit->setText(userProxy.hostName());
    mUi->portSpinBox->setValue(userProxy.port());
    mUi->typeComboBox->setCurrentIndex(userProxy.type() == QNetworkProxy::Socks5Proxy ? 1 : 0);
    mUi->loginEdit->setText(userProxy.user());
    mUi->passwordEdit->setText(userProxy.password());
  }

  ProxySettingsDialog::~ProxySettingsDialog() {
    return;
  }

  void ProxySettingsDialog::query(Shiken *ctx, QWidget *parent) {
    ProxySettingsDialog dlg(ctx, parent);
    if(dlg.exec() != QDialog::Accepted)
      return;

    SettingsDao *settings = ctx->model()->settingsDao();
    QNetworkProxy oldProxy = settings->userProxy();

    QString address = dlg.mUi->addressEdit->text();
    int port = dlg.mUi->portSpinBox->value();
    if(address.isEmpty() || port == 0)
      return;

    QNetworkProxy newProxy(
      dlg.mUi->typeComboBox->currentIndex() == 0 ? QNetworkProxy::HttpProxy : QNetworkProxy::Socks5Proxy,
      address,
      port
    );
    settings->setUserProxy(newProxy);
    settings->setProxyLogin(newProxy, dlg.mUi->loginEdit->text());
    settings->setProxyPassword(newProxy, dlg.mUi->passwordEdit->text());

    if(newProxy.hostName() != oldProxy.hostName() || newProxy.port() != oldProxy.port() || newProxy.type() != oldProxy.type())
      ctx->connectionManager()->testProxy(newProxy);
  }

} // namespace shiken
