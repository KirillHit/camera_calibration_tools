#ifndef LINE_BASED_WORKER_HPP
#define LINE_BASED_WORKER_HPP

#include "calib_tools_lib/calib_tools_lib.hpp"

#include <QFuture>
#include <QFutureWatcher>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <atomic>
#include <memory>

class LineBasedWorker : public QObject
{
    Q_OBJECT
public:
    explicit LineBasedWorker(QObject* parent = nullptr);
    ~LineBasedWorker() override;

signals:
    void finished(CameraIntrinsics result_cam);
    void progressMessage(QString msg);

public slots:
    void onStart(std::vector<std::vector<std::array<double, 2>>> lines,
                 CameraIntrinsics init_params);
    void onStop();

private:
    LineBasedCalibrator optimizer_;

    QThread worker_thread_;
    std::atomic<bool> cancel_requested_ {false};
    QFuture<void> future_;
    QFutureWatcher<void> watcher_;
};

class ProgressCallback : public ceres::IterationCallback
{
public:
    std::atomic<bool>* cancel_requested_ = nullptr;
    std::function<void(const QString&)> gui_callback;

    ceres::CallbackReturnType operator()(const ceres::IterationSummary& summary) override
    {
        if (gui_callback)
        {
            gui_callback(
              QString("iteration %1: cost = %2").arg(summary.iteration).arg(summary.cost));
        }

        if (cancel_requested_ && *cancel_requested_)
            return ceres::SOLVER_ABORT;
        return ceres::SOLVER_CONTINUE;
    }
};

#endif
