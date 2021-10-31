#ifndef SHIKEN_CHOOSE_QUIZ_DIALOG_H
#define SHIKEN_CHOOSE_QUIZ_DIALOG_H

#include <shiken/config.h>
#include <QDialog>
#include <QList>
#include <QScopedPointer>

namespace Ui {
  class ChooseQuizDialog;
}

namespace shiken {
  class Shiken;
  class QuizWidget;

// -------------------------------------------------------------------------- //
// ChooseQuizDialog
// -------------------------------------------------------------------------- //
  /**
   * ChooseQuizDialog is a dialog that lets user choose the quiz he wants to
   * work with.
   */
  class ChooseQuizDialog: public QDialog {
    Q_OBJECT;
  public:
    /**
     * @param ctx                      Application context.
     * @param parent                   Parent widget.
     */
    ChooseQuizDialog(Shiken *ctx, QWidget *parent = NULL);

    ~ChooseQuizDialog();

    /**
     * @returns                        true if this dialog was closed by
     *                                 pressing the "back" button, false 
     *                                 otherwise.
     */
    bool backClicked() const {
      return mBackClicked;
    }

  protected Q_SLOTS:
    void on_backButton_clicked();
    void on_okButton_clicked();
    
    void at_quizWidget_clicked();

  private:
    QScopedPointer<Ui::ChooseQuizDialog> mUi;

    Shiken *mCtx;

    QList<QuizWidget *> mQuizWidgets;

    bool mBackClicked;
  };

} // namespace shiken

#endif // SHIKEN_CHOOSE_QUIZ_DIALOG_H
