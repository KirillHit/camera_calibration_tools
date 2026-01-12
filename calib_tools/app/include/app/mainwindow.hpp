#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "app/image_sources/image_sources.hpp"
#include "app/tools/chessboard/chessboard_worker.hpp"
#include "app/tools/line_based/line_based_worker.hpp"
#include "calib_tools_lib/calib_tools_lib.hpp"

#include <QLabel>
#include <QLocale>
#include <QMainWindow>
#include <QScopedPointer>
#include <QThread>
#include <QTranslator>
#include <memory>
#include <opencv2/imgproc.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = 0);
    virtual ~MainWindow();

signals:
    /** @brief Signal to request the Pixmap to be updated. */
    void requestUpdatePixmap();

    /** @brief Signal emitted when camera intrinsic parameters have changed. */
    void cameraIntrinsicsChanged(const CameraIntrinsics& newParams);

    /** @brief Signal emitted when the image source has been updated. */
    void sourceUpdated();

public slots:
    /** @brief Slot to update camera intrinsic parameters in the GUI if they were changed elsewhere. */
    void onCameraIntrinsicsChanged(const CameraIntrinsics& newParams);

    /** @brief Slot to update the chessboard calibrator parameters from the GUI. */
    void onChessboardCalibratorUpdate();

    /** @brief Slot called when the chessboard calibration is finished to update the GUI. */
    void onChessboardCalibratorResult(ChessboardCalibrator::Result result);

    /** @brief Slot to receive a new frame from ImageSourceWorker.
     * 
     * @param frame New image frame. Passed by move, the original object becomes invalid.
     */
    void onNewFrame(cv::Mat frame);

    /** @brief Updates the image displayed on the Chessboard tab.
     * 
     * @param out_mat The processed image to display.
     */
    void onUpdateChessboardPixmap(cv::Mat out_mat);

    /** @brief Slot to display error messages from other threads. */
    void onError(const QString& message);

    /** @brief Slot to select an image or video file as the source. */
    void onSelectFile();

    /** @brief Slot to select a camera as the image source. */
    void onSelectCam();

    /** @brief Slot to select a stream as the image source. */
    void onSelectStream();

    /** @brief Slot to export camera intrinsics to ROS2 CameraInfo format. */
    void onActionExportROS();

    void onChangeLanguage(const QLocale& locale);

protected:
    /** @brief Overridden resize event handler to update the Pixmap when the window size changes. */
    void resizeEvent(QResizeEvent* event) override;

private:
    /** @brief Sets up all signal-slot connections between UI and worker threads. */
    void setupConnections();

    /** @brief Generates a QPixmap from a cv::Mat frame.
     * 
     * @param frame Image in cv::Mat format.
     * @return Corresponding QPixmap for display in QLabel.
     */
    QPixmap get_pixmap(const cv::Mat& frame) const;

    /** @brief Updates all displayed images based on the current internal state. */
    void updatePixmap();

    /** @brief Updates the image display on the Manual correction tab. */
    void updateManualPixmap();

    /** @brief Updates the image display on the Line-based tab. */
    void updateLineBasedPixmap();

    /** @brief Returns default camera intrinsic parameters. */
    CameraIntrinsics getDefaultParams() const;

    QScopedPointer<Ui::MainWindow> ui_;

    cv::Mat current_frame_;
    CameraIntrinsics camera_intrinsics_;
    CashedUndistort cashed_undistort_;

    ImageSourceWorker* image_worker_ = nullptr;
    QThread image_worker_thread_;

    LineBasedWorker line_based_worker_;
    ChessboardWorker chessboard_worker_;


    QTranslator translator_;
};

#endif
