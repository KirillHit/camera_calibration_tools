#include "app/tools/chessboard/chessboard_worker.hpp"

#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QtConcurrent>

ChessboardWorker::ChessboardWorker(QObject* parent) : QObject(parent) {}

ChessboardWorker::~ChessboardWorker()
{
    if (future_calib_.isRunning())
    {
        watcher_calib_.waitForFinished();
    }
    if (future_frame_.isRunning())
    {
        watcher_frame_.waitForFinished();
    }
}

size_t ChessboardWorker::collected() const
{
    if (!chessboard_calibrator_)
        return 0;
    return chessboard_calibrator_->size();
}

void ChessboardWorker::onUpdateCollection(
  cv::Size pattern_size,
  float square_size,
  int distant_threshold,
  float image_max_size,
  bool adaptive_threshold)
{
    try
    {
        chessboard_calibrator_ = std::make_shared<ChessboardCalibrator>(
          pattern_size,
          square_size,
          distant_threshold,
          image_max_size,
          adaptive_threshold);
    }
    catch (const std::invalid_argument& e)
    {
        chessboard_calibrator_.reset();
        qWarning() << tr("Failed to create ChessboardCalibrator:") << e.what();
    }
}

void ChessboardWorker::onProcessFrameAsync(const cv::Mat& frame)
{
    if (!chessboard_calibrator_ || frame.empty() || future_frame_.isRunning())
        return;

    cv::Mat frame_copy = frame.clone();
    auto calibrator_copy = chessboard_calibrator_;

    future_frame_ = QtConcurrent::run([this, frame_copy, calibrator_copy]() {
        cv::Mat out_frame;
        calibrator_copy->process_frame(frame_copy, out_frame);
        emit frameProcessed(out_frame);
    });
    watcher_frame_.setFuture(future_frame_);
}

void ChessboardWorker::onCalibrateAsync()
{
    if (!chessboard_calibrator_ || future_calib_.isRunning())
        return;

    auto calibrator_copy = std::make_shared<ChessboardCalibrator>(*chessboard_calibrator_);
    future_calib_ = QtConcurrent::run([this, calibrator_copy]() {
        auto result = calibrator_copy->calibrate();
        emit calibrationFinished(result);
    });
    watcher_calib_.setFuture(future_calib_);
}

void ChessboardWorker::onPrintChessboard()
{
    if (!chessboard_calibrator_)
        return;

    const cv::Size pattern_size = chessboard_calibrator_->pattern_size();
    const int squares_y = pattern_size.width + 1;
    const int squares_x = pattern_size.height + 1;

    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, qobject_cast<QWidget*>(parent()));
    if (dialog.exec() != QDialog::Accepted)
        return;

    QPainter painter(&printer);
    if (!painter.isActive())
        return;

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);

    QRectF rect = painter.viewport();

    double cell_width = rect.width() / squares_x;
    double cell_height = rect.height() / squares_y;
    double cell_size = std::min(cell_width, cell_height);

    double offset_x = rect.left() + (rect.width() - cell_size * squares_x) / 2.0;
    double offset_y = rect.top() + (rect.height() - cell_size * squares_y) / 2.0;

    for (int y = 0; y < squares_y; ++y)
    {
        for (int x = 0; x < squares_x; ++x)
        {
            if ((x + y) % 2 == 0)
                painter.drawRect(
                  QRectF(offset_x + x * cell_size, offset_y + y * cell_size, cell_size, cell_size));
        }
    }
}
