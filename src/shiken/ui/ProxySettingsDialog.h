#ifndef SHIKEN_PROXY_SETTINGS_DIALOG_H
#define SHIKEN_PROXY_SETTINGS_DIALOG_H

#include <shiken/config.h>
#include <QDialog>
#include <QScopedPointer>

namespace Ui {
  class ProxySettingsDialog;
}

namespace shiken {
  class Shiken;

// -------------------------------------------------------------------------- //
// ProxySettingsDialog
// -------------------------------------------------------------------------- //
  /**
   * This is a proxy configuration dialog.
   *
   * It lets the user to configure proxy address, port, type, login and
   * password.
   */
  class ProxySettingsDialog: public QDialog {
  public:
    ProxySettingsDialog(Shiken *ctx, QWidget *parent = NULL);

    ~ProxySettingsDialog();

    /**
     * Executes ProxySettingsDialog and saves resulting proxy as a 
     * user-specified proxy in settings.
     */
    static void query(Shiken *ctx, QWidget *parent = NULL);

  private:
    QScopedPointer<Ui::ProxySettingsDialog> mUi;

    Shiken *mCtx;
  };

} // namespace shiken

#endif // SHIKEN_PROXY_SETTINGS_DIALOG_H
