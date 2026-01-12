#include "app/dialogs/stream_settings_dialog.hpp"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

StreamSettingsDialog::StreamSettingsDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("Stream Connection"));

    url_edit_ = new QLineEdit(this);
    url_edit_->setPlaceholderText("rtsp://user:pass@ip:port/stream");

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

    QLabel* hintLabel = new QLabel(tr("0: default value"), this);

    QFormLayout* layout = new QFormLayout(this);
    layout->addRow(tr("URL:"), url_edit_);
    layout->addRow(tr("Width:"), width_spin_);
    layout->addRow(tr("Height:"), height_spin_);
    layout->addRow(tr("FPS:"), fps_spin_);
    layout->addWidget(hintLabel);

    QDialogButtonBox* buttons =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(buttons);
}

QString StreamSettingsDialog::streamUrl() const
{
    return url_edit_->text();
}
int StreamSettingsDialog::width() const
{
    return width_spin_->value();
}
int StreamSettingsDialog::height() const
{
    return height_spin_->value();
}
int StreamSettingsDialog::fps() const
{
    return fps_spin_->value();
}
