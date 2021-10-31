#include "ImageDialog.h"
#include <cmath>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QEvent>
#include <QWheelEvent>

namespace shiken {
  /**
   * A graphics view that allows zooming with a mouse wheel.
   */
  class ZoomGraphicsView: public QGraphicsView {
  public:
    ZoomGraphicsView(QGraphicsScene *scene, QWidget *parent = NULL): QGraphicsView(scene, parent) {}

  protected:
    virtual void wheelEvent(QWheelEvent* event) {
      qreal numDegrees = event->delta() / 8.0;
      qreal scaleFactor = std::pow(2.0, numDegrees / 180.0);
      scale(scaleFactor, scaleFactor); /* 2x scale for 180 degree turn. */
      event->accept();
    }

  };

  ImageDialog::ImageDialog(QString fileName, QWidget *parent): QDialog(parent) {
    QGraphicsScene *scene = new QGraphicsScene(this);
    QPixmap pixmap(fileName);
    scene->addPixmap(pixmap);

    mView = new ZoomGraphicsView(scene);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(mView);
    layout->setContentsMargins(0, 0, 0, 0);

    setLayout(layout);

    resize(800, 600);

    float scale = qMin(static_cast<float>(width()) / pixmap.width(), static_cast<float>(height()) / pixmap.height());
    mView->scale(scale, scale);
  }

} // namespace shiken
