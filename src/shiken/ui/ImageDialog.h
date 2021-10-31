#ifndef SHIKEN_IMAGE_DIALOG_H
#define SHIKEN_IMAGE_DIALOG_H

#include <shiken/config.h>
#include <QDialog>

namespace shiken {
  class ZoomGraphicsView;

// -------------------------------------------------------------------------- //
// ImageDialog
// -------------------------------------------------------------------------- //
  /**
   * This dialog shows a preview of the given image.
   */
  class ImageDialog: public QDialog {
    Q_OBJECT;
  public:
    /**
     * @param fileName                 Name of the image file to show preview
     *                                 of.
     * @param parent                   Parent widget.
     */
    ImageDialog(QString fileName, QWidget *parent = NULL);

  private:
    ZoomGraphicsView *mView;
  };

} // namespace shiken

#endif // SHIKEN_IMAGE_DIALOG_H
