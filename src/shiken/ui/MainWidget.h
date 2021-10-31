#ifndef SHIKEN_MAIN_WIDGET_H
#define SHIKEN_MAIN_WIDGET_H

#include <shiken/config.h>
#include <QMainWindow>
#include <QScopedPointer>
#include <QModelIndex>
#include <QSet>
#include <shiken/entities/Scan.h>
#include <shiken/entities/Page.h>
#include <shiken/entities/User.h>

class QTimer;
class QSpinBox;
class QLabel;
class QTreeView;

namespace Ui {
  class MainWidget;
  class MainWidgetSingleUser;
}

namespace shiken {
  class SqlQueryModel;
  class Shiken;

// -------------------------------------------------------------------------- //
// MainWidget
// -------------------------------------------------------------------------- //
  /**
   * MainWidget is the main program window of shiken.
   */
  class MainWidget: public QMainWindow {
    Q_OBJECT;
  public:
    /**
     * @param ctx                      Application context.
     */
    MainWidget(Shiken *ctx);
    ~MainWidget();

  protected slots:
    /* Slots processing UI events. */
    void on_selectAllButton_clicked();
    void on_clearSelectionButton_clicked();
    void on_userView_clicked(QModelIndex idx);
    void on_scanView_doubleClicked(QModelIndex idx);
    void on_addScansButton_clicked();
    void on_removeScansButton_clicked();
    void on_sendScansButton_clicked();
    void on_printPdfButton_clicked();
#ifdef Q_WS_WIN
    void on_printButton_clicked();
#endif
    void on_aboutAction_triggered();
    void on_helpAction_triggered();
    void on_proxySettingsAction_triggered();

    /* Callbacks from workers. */
    void pagePrinted(Page page);
    void scanRecognized(Scan scan, QString barcode);
    void scanUploaded(Scan scan);
    void scanNotFound(Scan scan);

    /* Post-processors for item views. */
    void updateUserModel();
    void updateScanModel();
    void updateUserScanModel();

    void updatePageCount();
    void updateScanCount();

    /* Events that are triggered by timer. */
    void getScans();
    void checkUpdates();

  protected:
    virtual bool eventFilter(QObject* sender, QEvent* e) OVERRIDE;
    
    template<class ActualUi, class DummyUi>
    void initUi(ActualUi *actualUi, DummyUi *dummyUi);

  private:
    void print(QPrinter& printer);
    void deleteScan(QString fileName);

    QScopedPointer<Ui::MainWidget> mUi;
    QScopedPointer<Ui::MainWidgetSingleUser> mSingleUi;
    QScopedPointer<QMainWindow> mDummy;

    User mSingleUser;

    QSpinBox *mPagesPerStudentSpinBox;
    QLabel *mSendAddressLabel;
    QTreeView *mScanView;

    Shiken *mCtx;

    SqlQueryModel* mUserModel;
    SqlQueryModel* mUserScanModel;
    SqlQueryModel* mScanModel;

    QSet<QString> mSelectedCompressedGuids;
    int mInvalidUserButton, mDuplicateScanButton, mScanNotFoundButton;

    QTimer *mTimer;
  };

} // namespace shiken

#endif // SHIKEN_MAIN_WIDGET_H
