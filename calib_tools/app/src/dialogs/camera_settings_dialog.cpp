#include "app/dialogs/camera_settings_dialog.hpp"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

CameraSettingsDialog::CameraSettingsDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("Camera Connection"));

    cam_index_spin_ = new QSpinBox(this);
    cam_index_spin_->setMinimum(0);
    cam_index_spin_->setMaximum(99);
    cam_index_spin_->setValue(0);

    QLabel* paramLabel = new QLabel(tr("Camera parameters:"), this);

    width_spin_ = new QSpinBox(this);
    width_spin_->setMinimum(0);
    width_spin_->setMaximum(10000);
    width_spin_->setValue(0);

    height_spin_ = new QSpinBox(this);
    height_spin_->setMinimum(0);
    height_spin_->setMaximum(10000);
    height_spin_->setValue(0);

    fps_spin_ = new QSpinBox(this);
    fps_spin_->setMinimum(0);
    fps_spin_->setMaximum(240);
    fps_spin_->setValue(0);

    QLabel* fourccLabel = new QLabel(tr("FOURCC codec:"), this);
    fourcc_edit_ = new QLineEdit(this);
    fourcc_edit_->setMaxLength(4);
    fourcc_edit_->setPlaceholderText("MJPG");
    fourcc_edit_->setText("");

    QLabel* hintLabel = new QLabel(tr("0: default value"), this);

    QFormLayout* layout = new QFormLayout(this);
    layout->addRow(tr("Camera index:"), cam_index_spin_);
    layout->addWidget(paramLabel);
    layout->addRow(tr("Width:"), width_spin_);
    layout->addRow(tr("Height:"), height_spin_);
    layout->addRow(tr("FPS:"), fps_spin_);
    layout->addRow(fourccLabel, fourcc_edit_);
    layout->addWidget(hintLabel);

    QDialogButtonBox* buttons =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(buttons);
}

int CameraSettingsDialog::camIndex() const
{
    return cam_index_spin_->value();
}
int CameraSettingsDialog::width() const
{
    return width_spin_->value();
}
int CameraSettingsDialog::height() const
{
    return height_spin_->value();
}
int CameraSettingsDialog::fps() const
{
    return fps_spin_->value();
}
std::string CameraSettingsDialog::fourcc() const
{
    return fourcc_edit_->text().toStdString();
}
