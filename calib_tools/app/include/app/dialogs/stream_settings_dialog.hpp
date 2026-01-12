#ifndef STREAM_SETTINGS_DIALOG_HPP
#define STREAM_SETTINGS_DIALOG_HPP

#include <QDialog>
#include <QSpinBox>

class StreamSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    StreamSettingsDialog(QWidget* parent = nullptr);

    QString streamUrl() const;
    int width() const;
    int height() const;
    int fps() const;

private:
    QLineEdit* url_edit_;
    QSpinBox* width_spin_;
    QSpinBox* height_spin_;
    QSpinBox* fps_spin_;
};


#endif
