#include "app/mainwindow.hpp"

#include "../ui_mainwindow.h"
#include "app/dialogs/dialogs.hpp"

#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QImage>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPixmap>
#include <iostream>

using namespace Qt::StringLiterals;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);

    onChangeLanguage(QLocale::system());

    image_worker_ = new ImageSourceWorker(nullptr);
    image_worker_->moveToThread(&image_worker_thread_);

    setupConnections();

    image_worker_thread_.start();
    onChessboardCalibratorUpdate();
}

MainWindow::~MainWindow()
{
    if (image_worker_)
    {
        QMetaObject::invokeMethod(image_worker_, "onStop", Qt::BlockingQueuedConnection);
    }
    image_worker_thread_.quit();
    image_worker_thread_.wait();
}

void MainWindow::setupConnections()
{
    // --- Threads and cleanup ---
    connect(&image_worker_thread_, &QThread::finished, image_worker_, &QObject::deleteLater);
    connect(this, &MainWindow::destroyed, &image_worker_thread_, &QThread::quit);

    // --- Interaction with the image source thread ---
    connect(image_worker_, &ImageSourceWorker::newFrame, this, &MainWindow::onNewFrame);
    connect(image_worker_, &ImageSourceWorker::errorOccurred, this, &MainWindow::onError);
    connect(image_worker_,
            &ImageSourceWorker::infoOccurred,
            ui_->labelSourceInfo,
            &QLabel::setText);
    connect(ui_->butPlayPause, &QPushButton::toggled, this, [this](bool checked) {
        QMetaObject::invokeMethod(
          image_worker_,
          [this, checked]() {
              if (checked)
                  image_worker_->onPlay();
              else
                  image_worker_->onPause();
          },
          Qt::QueuedConnection);
    });
    connect(ui_->butSourceRestart,
            &QPushButton::clicked,
            image_worker_,
            &ImageSourceWorker::onStart,
            Qt::QueuedConnection);
    connect(ui_->boxFps,
            qOverload<int>(&QSpinBox::valueChanged),
            image_worker_,
            &ImageSourceWorker::onSetFPS,
            Qt::QueuedConnection);

    // --- Camera intrinsic parameters control ---
    connect(ui_->manualReset, &QPushButton::clicked, this, [this](int) {
        emit cameraIntrinsicsChanged(getDefaultParams());
    });
    auto bindSpin = [this](QDoubleSpinBox* box, double& param) {
        connect(box,
                qOverload<double>(&QDoubleSpinBox::valueChanged),
                this,
                [&param, this](double value) {
                    param = value;
                    emit requestUpdatePixmap();
                });
    };
    bindSpin(ui_->manualFxBox, camera_intrinsics_.fx);
    bindSpin(ui_->manualFyBox, camera_intrinsics_.fy);
    bindSpin(ui_->manualCxBox, camera_intrinsics_.cx);
    bindSpin(ui_->manualCyBox, camera_intrinsics_.cy);
    bindSpin(ui_->manualK1Box, camera_intrinsics_.k1);
    bindSpin(ui_->manualK2Box, camera_intrinsics_.k2);
    bindSpin(ui_->manualP1Box, camera_intrinsics_.p1);
    bindSpin(ui_->manualP2Box, camera_intrinsics_.p2);
    bindSpin(ui_->manualAlphaBox, camera_intrinsics_.alpha);

    // --- Source selection buttons ---
    connect(ui_->actionSelectFile, &QAction::triggered, this, &MainWindow::onSelectFile);
    connect(ui_->actionSelectCam, &QAction::triggered, this, &MainWindow::onSelectCam);
    connect(ui_->actionSelectStream, &QAction::triggered, this, &MainWindow::onSelectStream);

    // --- Other signals ---
    connect(this,
            &MainWindow::cameraIntrinsicsChanged,
            this,
            &MainWindow::onCameraIntrinsicsChanged,
            Qt::QueuedConnection);
    connect(this,
            &MainWindow::requestUpdatePixmap,
            this,
            &MainWindow::updatePixmap,
            Qt::QueuedConnection);
    connect(ui_->tabWidget, &QTabWidget::currentChanged, this, [this](int) {
        emit requestUpdatePixmap();
    });

    // --- Line-based tab ---
    connect(ui_->butAddLine, &QPushButton::clicked, ui_->viewLineBased, &LineBasedView::addLine);
    connect(ui_->viewLineBased, &LineBasedView::linesUpdated, this, [this](int total, int active) {
        ui_->listLineBased->clear();
        for (int i = 0; i < total; ++i)
        {
            ui_->listLineBased->addItem(QString(tr("Line %1")).arg(i));
        }
        if (active >= 0 && active < total)
            ui_->listLineBased->setCurrentRow(active);
    });
    connect(ui_->listLineBased,
            &QListWidget::currentRowChanged,
            ui_->viewLineBased,
            [this](int row) {
                ui_->viewLineBased->setActiveLine(static_cast<std::ptrdiff_t>(row));
            });
    connect(ui_->butDelLine, &QPushButton::clicked, this, [this]() {
        ui_->viewLineBased->removeLine(
          static_cast<std::ptrdiff_t>(ui_->listLineBased->currentRow()));
    });
    connect(ui_->butResetLines,
            &QPushButton::clicked,
            ui_->viewLineBased,
            &LineBasedView::removeAllLines);
    connect(ui_->butLineBasedStart, &QPushButton::clicked, this, [this]() {
        ui_->textLineBasedOut->clear();
        line_based_worker_.onStart(ui_->viewLineBased->get_lines(), getDefaultParams());
    });
    connect(ui_->butLineBasedStop,
            &QPushButton::clicked,
            &line_based_worker_,
            &LineBasedWorker::onStop);
    connect(&line_based_worker_,
            &LineBasedWorker::finished,
            this,
            [this](CameraIntrinsics result_cam) {
                onCameraIntrinsicsChanged(result_cam);
            });
    connect(&line_based_worker_,
            &LineBasedWorker::progressMessage,
            ui_->textLineBasedOut,
            &QPlainTextEdit::appendPlainText);

    // --- Chessboard tab ---
    connect(ui_->butChessboardAccess,
            &QPushButton::clicked,
            this,
            &MainWindow::onChessboardCalibratorUpdate);
    connect(this, &MainWindow::sourceUpdated, this, &MainWindow::onChessboardCalibratorUpdate);
    connect(ui_->butChessboardStart,
            &QPushButton::clicked,
            &chessboard_worker_,
            &ChessboardWorker::onCalibrateAsync);
    connect(ui_->butChessboardStart, &QPushButton::clicked, this, [this]() {
        ui_->butChessboardStart->setEnabled(false);
        ui_->chessboardStatusOut->setText(tr("Calibrating..."));
        ui_->chessboardRMSOut->setText(QString());
        ui_->chessboardResultOut->setText(QString());
    });
    connect(&chessboard_worker_,
            &ChessboardWorker::calibrationFinished,
            this,
            &MainWindow::onChessboardCalibratorResult);
    connect(&chessboard_worker_,
            &ChessboardWorker::frameProcessed,
            this,
            &MainWindow::onUpdateChessboardPixmap);
    connect(ui_->butChessboardPrint,
            &QPushButton::clicked,
            &chessboard_worker_,
            &ChessboardWorker::onPrintChessboard);

    // --- Export ---
    connect(ui_->actionExportROS, &QAction::triggered, this, &MainWindow::onActionExportROS);

    // --- Language change ---
    connect(ui_->actionRussian, &QAction::triggered, this, [this]() {
        onChangeLanguage(QLocale(QLocale::Russian));
    });
    connect(ui_->actionEnglish, &QAction::triggered, this, [this]() {
        onChangeLanguage(QLocale(QLocale::English));
    });
}

void MainWindow::onNewFrame(cv::Mat frame)
{
    if (frame.empty())
        return;

    current_frame_ = std::move(frame);
    emit requestUpdatePixmap();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    emit requestUpdatePixmap();
}

QPixmap MainWindow::get_pixmap(const cv::Mat& frame) const
{
    if (frame.empty())
        return QPixmap();

    cv::Mat rgb;
    if (frame.channels() == 3)
        cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    else if (frame.channels() == 1)
        cv::cvtColor(frame, rgb, cv::COLOR_GRAY2RGB);
    else
        return QPixmap();

    QImage qimg(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(qimg);

    return pixmap;
}

void MainWindow::updatePixmap()
{
    if (current_frame_.empty())
        return;

    auto current_tab = ui_->tabWidget->currentWidget();
    if (current_tab == ui_->manual_tab)
        updateManualPixmap();
    else if (current_tab == ui_->line_based_tab)
        updateLineBasedPixmap();
    else if (current_tab == ui_->chessboard_tab)
        chessboard_worker_.onProcessFrameAsync(current_frame_);
}

void MainWindow::updateManualPixmap()
{
    auto updateLabel = [this](QLabel* label, const cv::Mat& frame) {
        QSize labelSize = label->size();
        QPixmap pixmap =
          get_pixmap(frame).scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        label->setPixmap(pixmap);
    };

    updateLabel(ui_->manualIn, current_frame_);

    cv::Mat correctedFrame;
    try
    {
        cashed_undistort_(current_frame_, correctedFrame, camera_intrinsics_);
    }
    catch (const std::exception& e)
    {
        qDebug() << tr("Error in undistort_image:") << e.what();
        return;
    }

    updateLabel(ui_->manualOut, correctedFrame);
}

void MainWindow::updateLineBasedPixmap()
{
    ui_->viewLineBased->setFrame(get_pixmap(current_frame_));
    ui_->viewLineBased->update();
}

void MainWindow::onUpdateChessboardPixmap(cv::Mat out_mat)
{
    ui_->chessboardSamplesOut->setText(QString::number(chessboard_worker_.collected()));

    QSize labelSize = ui_->chessboardOut->size();
    QPixmap pixmap =
      get_pixmap(out_mat).scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui_->chessboardOut->setPixmap(pixmap);
}

void MainWindow::onError(const QString& message)
{
    QMessageBox::critical(this, tr("Error"), message);
}

void MainWindow::onSelectFile()
{
    QString filePath = QFileDialog::getOpenFileName(
      this,
      tr("Select image or video"),
      QString(),
      tr("Media files (*.png *.jpg *.jpeg *.bmp *.avi *.mp4 *.mkv);;All files (*.*)"));

    if (filePath.isEmpty())
        return;

    auto src = std::make_shared<FileImageSource>(filePath.toStdString());

    QMetaObject::invokeMethod(
      image_worker_,
      [this, src]() {
          image_worker_->onSetSource(src);
          image_worker_->onSetFPS(ui_->boxFps->value());
          image_worker_->onStart();
      },
      Qt::QueuedConnection);

    emit sourceUpdated();
}

void MainWindow::onSelectCam()
{
    CameraSettingsDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    auto src = std::make_shared<CameraImageSource>(
      dlg.camIndex(),
      dlg.width(),
      dlg.height(),
      dlg.fps(),
      dlg.fourcc());

    QMetaObject::invokeMethod(
      image_worker_,
      [this, src]() {
          image_worker_->onSetSource(src);
          image_worker_->onStart();
      },
      Qt::QueuedConnection);

    emit sourceUpdated();
}

void MainWindow::onSelectStream()
{
    StreamSettingsDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    auto src = std::make_shared<StreamImageSource>(
      dlg.streamUrl().toStdString(),
      dlg.width(),
      dlg.height(),
      dlg.fps());

    QMetaObject::invokeMethod(
      image_worker_,
      [this, src]() {
          image_worker_->onSetSource(src);
          image_worker_->onStart();
      },
      Qt::QueuedConnection);

    emit sourceUpdated();
}

void MainWindow::onCameraIntrinsicsChanged(const CameraIntrinsics& newParams)
{
    camera_intrinsics_ = newParams;

    {
        QSignalBlocker blockFx(ui_->manualFxBox);
        ui_->manualFxBox->setValue(camera_intrinsics_.fx);
    }
    {
        QSignalBlocker blockFy(ui_->manualFyBox);
        ui_->manualFyBox->setValue(camera_intrinsics_.fy);
    }
    {
        QSignalBlocker blockCx(ui_->manualCxBox);
        ui_->manualCxBox->setValue(camera_intrinsics_.cx);
    }
    {
        QSignalBlocker blockCy(ui_->manualCyBox);
        ui_->manualCyBox->setValue(camera_intrinsics_.cy);
    }
    {
        QSignalBlocker blockK1(ui_->manualK1Box);
        ui_->manualK1Box->setValue(camera_intrinsics_.k1);
    }
    {
        QSignalBlocker blockK2(ui_->manualK2Box);
        ui_->manualK2Box->setValue(camera_intrinsics_.k2);
    }
    {
        QSignalBlocker blockK3(ui_->manualK3Box);
        ui_->manualK3Box->setValue(camera_intrinsics_.k3);
    }
    {
        QSignalBlocker blockP1(ui_->manualP1Box);
        ui_->manualP1Box->setValue(camera_intrinsics_.p1);
    }
    {
        QSignalBlocker blockP2(ui_->manualP2Box);
        ui_->manualP2Box->setValue(camera_intrinsics_.p2);
    }
    {
        QSignalBlocker blockA(ui_->manualAlphaBox);
        ui_->manualAlphaBox->setValue(camera_intrinsics_.alpha);
    }

    emit requestUpdatePixmap();
}

CameraIntrinsics MainWindow::getDefaultParams() const
{
    CameraIntrinsics defaultParams;

    defaultParams.fx = 800.0;
    defaultParams.fy = 800.0;
    defaultParams.k1 = 0.0;
    defaultParams.k2 = 0.0;
    defaultParams.k3 = 0.0;
    defaultParams.p1 = 0.0;
    defaultParams.p2 = 0.0;

    if (!current_frame_.empty())
    {
        defaultParams.cx = current_frame_.cols / 2.0;
        defaultParams.cy = current_frame_.rows / 2.0;
    }
    else
    {
        defaultParams.cx = 0.0;
        defaultParams.cy = 0.0;
    }

    return defaultParams;
}

void MainWindow::onChessboardCalibratorUpdate()
{
    chessboard_worker_.onUpdateCollection(
      cv::Size(ui_->spinChessboardX->value(), ui_->spinChessboardY->value()),
      static_cast<float>(ui_->spinChessboardSize->value()),
      ui_->spinChessboardDistThreshold->value(),
      ui_->spinChessboardMaxSize->value(),
      ui_->radioChessboardAdaptiveThreshold->isChecked());

    ui_->chessboardSamplesOut->setText(QString::number(0));
}

void MainWindow::onChessboardCalibratorResult(ChessboardCalibrator::Result result)
{
    ui_->butChessboardStart->setEnabled(true);

    if (!result.success)
    {
        ui_->chessboardStatusOut->setText(tr("Error"));
        return;
    }

    ui_->chessboardRMSOut->setText(QString::number(result.rms_error, 'f', 4));

    QString qualityText;
    if (result.rms_error < 0.5)
        qualityText = tr("Good");
    else if (result.rms_error < 1.0)
        qualityText = tr("Medium");
    else
        qualityText = tr("Poor");

    ui_->chessboardResultOut->setText(qualityText);

    ui_->chessboardStatusOut->setText(tr("Success"));

    emit onCameraIntrinsicsChanged(result.intrinsics);
}

void MainWindow::onActionExportROS()
{
    const int w = current_frame_.cols > 0 ? current_frame_.cols : -1;
    const int h = current_frame_.rows > 0 ? current_frame_.rows : -1;

    std::string yaml = camera_intrinsics_.to_ros2_yaml_string(w, h);
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Export ROS2 CameraInfo"));

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QPlainTextEdit* textEdit = new QPlainTextEdit(&dialog);
    textEdit->setPlainText(QString::fromStdString(yaml));
    textEdit->setReadOnly(true);
    textEdit->setMinimumSize(600, 400);
    layout->addWidget(textEdit);

    QPushButton* okButton = new QPushButton(tr("OK"), &dialog);
    okButton->setDefault(true);
    layout->addWidget(okButton);

    QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}

void MainWindow::onChangeLanguage(const QLocale& locale)
{
    qApp->removeTranslator(&translator_);

    if (locale.language() != QLocale::English)
    {
        if (translator_.load(locale, u"calib_tools"_s, u"_"_s, u":/i18n"_s))
        {
            qApp->installTranslator(&translator_);
        }
        else
        {
            qDebug() << "Translation file for locale" << locale.name() << "not found.";
        }
    }

    ui_->retranslateUi(this);
}
