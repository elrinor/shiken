#ifndef SHIKEN_QUIZ_WIDGET_H
#define SHIKEN_QUIZ_WIDGET_H

#include <shiken/config.h>
#include <QFrame>
#include <shiken/entities/Quiz.h>

class QMouseEvent;

namespace shiken {
// -------------------------------------------------------------------------- //
// QuizWidget
// -------------------------------------------------------------------------- //
  /**
   * QuizWidget is a widget representing a single quiz in a quiz selection
   * dialog.
   */
  class QuizWidget: public QFrame {
    Q_OBJECT;
  public:
    /**
     * @param quiz                     Quiz that will be represented by this 
     *                                 QuizWidget.
     * @param parent                   Parent widget.
     */
    QuizWidget(const Quiz &quiz, QWidget *parent = NULL);

    /**
     * @returns                        Quiz associated with this QuizWidget.
     */
    const Quiz &quiz() {
      return mQuiz;
    }

    /**
     * @param selected                 Whether this QuizWidget is selected.
     */
    void setSelected(bool selected);

  Q_SIGNALS:
    /**
     * This signal is emitted whenever user clicks on this QuizWidget.
     */
    void clicked();

  protected:
    virtual void mousePressEvent(QMouseEvent *event);

  private:
    Quiz mQuiz;
  };

} // namespace shiken

#endif // SHIKEN_QUIZ_WIDGET_H
