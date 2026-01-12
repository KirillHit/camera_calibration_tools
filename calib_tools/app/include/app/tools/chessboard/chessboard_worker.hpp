#ifndef CHESSBOARD_WORKER_HPP
#define CHESSBOARD_WORKER_HPP

#include "calib_tools_lib/calib_tools_lib.hpp"

#include <QFuture>
#include <QFutureWatcher>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <atomic>
#include <memory>

class ChessboardWorker : public QObject
{
    Q_OBJECT
public:
    explicit ChessboardWorker(QObject* parent = nullptr);
    ~ChessboardWorker() override;

    size_t collected() const;

signals:
    void calibrationFinished(ChessboardCalibrator::Result success);
    void frameProcessed(cv::Mat frame);

public slots:
    void onUpdateCollection(cv::Size pattern_size,
                            float square_size,
                            int distant_threshold,
                            float image_max_size,
                            bool adaptive_threshold);
    void onProcessFrameAsync(const cv::Mat& frame);
    void onCalibrateAsync();
    void onPrintChessboard();

private:
    std::shared_ptr<ChessboardCalibrator> chessboard_calibrator_;

    QThread worker_thread_;
    std::atomic<bool> cancel_requested_ {false};
    QFuture<void> future_frame_;
    QFutureWatcher<void> watcher_frame_;
    QFuture<void> future_calib_;
    QFutureWatcher<void> watcher_calib_;
};

#endif
