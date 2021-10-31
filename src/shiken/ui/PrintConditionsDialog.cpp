#include "PrintConditionsDialog.h"
#include "ui_PrintConditionsDialog.h"

namespace shiken {
  PrintConditionsDialog::PrintConditionsDialog(const QString &message, QWidget *parent): QDialog(parent), mUi(new Ui::PrintConditionsDialog()) {
    mUi->setupUi(this);

    connect(mUi->checkBox1, SIGNAL(stateChanged(int)), this, SLOT(checkBoxStateChanged()));
    connect(mUi->checkBox2, SIGNAL(stateChanged(int)), this, SLOT(checkBoxStateChanged()));

    mUi->label->setText(message);
  }

  PrintConditionsDialog::~PrintConditionsDialog() {
    return;
  }

  void PrintConditionsDialog::checkBoxStateChanged() {
    mUi->okButton->setEnabled(mUi->checkBox1->checkState() == Qt::Checked && mUi->checkBox2->checkState() == Qt::Checked);
  }

} // namespace shiken
