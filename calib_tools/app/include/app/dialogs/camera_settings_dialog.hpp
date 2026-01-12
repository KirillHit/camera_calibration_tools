#ifndef CAMERA_SETTINGS_DIALOG_HPP
#define CAMERA_SETTINGS_DIALOG_HPP

#include <QDialog>
#include <QSpinBox>

class CameraSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    CameraSettingsDialog(QWidget* parent = nullptr);

    int camIndex() const;
    int width() const;
    int height() const;
    int fps() const;
    std::string fourcc() const;

private:
    QSpinBox* cam_index_spin_;
    QSpinBox* width_spin_;
    QSpinBox* height_spin_;
    QSpinBox* fps_spin_;
    QLineEdit* fourcc_edit_;
};


#endif
