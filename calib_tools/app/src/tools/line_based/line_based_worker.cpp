#include "app/tools/line_based/line_based_worker.hpp"

#include <QtConcurrent>
#include <ceres/ceres.h>


LineBasedWorker::LineBasedWorker(QObject* parent) : QObject(parent)
{
    auto* progress_cb = new ProgressCallback();
    progress_cb->cancel_requested_ = &cancel_requested_;
    progress_cb->gui_callback = [this](const QString& msg) {
        QMetaObject::invokeMethod(
          this,
          [this, msg]() {
              emit progressMessage(msg);
          },
          Qt::QueuedConnection);
    };

    optimizer_.set_callback(progress_cb);
}

LineBasedWorker::~LineBasedWorker()
{
    onStop();
    if (future_.isRunning())
    {
        watcher_.waitForFinished();
    }
}

void LineBasedWorker::onStart(std::vector<std::vector<std::array<double, 2>>> lines,
                              CameraIntrinsics init_params)
{
    if (future_.isRunning())
        return;
    cancel_requested_ = false;

    future_ = QtConcurrent::run(
      [this, lines = std::move(lines), init_params = std::move(init_params)]() mutable {
          CameraIntrinsics output_params;
          ceres::Solver::Summary summary = optimizer_.optimize(lines, init_params, output_params);

          std::string split = "\n====================\n";
          emit progressMessage(QString::fromStdString(split + summary.BriefReport() + split));
          switch (summary.termination_type)
          {
              case ceres::CONVERGENCE:
                  emit progressMessage(tr("Converged successfully"));
                  break;
              case ceres::NO_CONVERGENCE:
                  emit progressMessage(tr("Maximum iterations reached"));
                  break;
              case ceres::FAILURE:
                  emit progressMessage(tr("Minimizer failed"));
                  break;
              case ceres::USER_SUCCESS:
              case ceres::USER_FAILURE:
                  emit progressMessage(tr("Stopped by user"));
                  break;
          }

          emit finished(output_params);
      });

    watcher_.setFuture(future_);
}

void LineBasedWorker::onStop()
{
    cancel_requested_ = true;
}
