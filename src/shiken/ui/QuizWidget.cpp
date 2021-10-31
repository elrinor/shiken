#include "QuizWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>

namespace shiken {
  QuizWidget::QuizWidget(const Quiz &quiz, QWidget *parent): QFrame(parent), mQuiz(quiz) {
    QLabel *header = new QLabel(quiz.title());
    header->setWordWrap(true);

    //QLabel *desc = new QLabel(quiz.description());
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(header);
    //layout->addWidget(desc);
    setLayout(layout);

    setAutoFillBackground(true);
    setSelected(false);

    setCursor(Qt::PointingHandCursor);

    setFrameShape(QFrame::Box);
  }

  void QuizWidget::setSelected(bool selected) {
    QPalette pal = palette();
    if(selected) {
      pal.setColor(backgroundRole(), QColor(255, 255, 192));
    } else {
      pal.setColor(backgroundRole(), Qt::white);
    }
    setPalette(pal);
  }

  void QuizWidget::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
      event->accept();

      Q_EMIT clicked();
    }
  }

} // namespace shiken
