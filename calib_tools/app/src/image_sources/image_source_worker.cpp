#include "app/image_sources/image_source_worker.hpp"

#include <QCoreApplication>
#include <QMessageBox>
#include <QMetaObject>
#include <QThread>

ImageSourceWorker::ImageSourceWorker(QObject* parent) : QObject(parent) {}

ImageSourceWorker::~ImageSourceWorker()
{
    onStop();
}

void ImageSourceWorker::onSetSource(std::shared_ptr<IImageSource> src)
{
    onStop();
    source_ = std::move(src);
}

void ImageSourceWorker::onSetFPS(unsigned int fps)
{
    if (fps == 0)
        return;
    interval_ms_ = 1000 / fps;
    if (timer_ && source_ && source_->grab_mode() == IImageSource::GrabMode::Timed)
        timer_->setInterval(interval_ms_);
}

void ImageSourceWorker::onStart()
{
    onStop();

    if (!source_)
        return;

    try
    {
        source_->start();
    }
    catch (const std::exception& e)
    {
        emit errorOccurred(QString::fromStdString(e.what()));
        emit infoOccurred(QString::fromStdString(e.what()));
        return;
    }

    emit infoOccurred(QString::fromStdString(source_->get_info()));

    running_ = true;

    timer_ = std::make_unique<QTimer>(this);
    connect(timer_.get(),
            &QTimer::timeout,
            this,
            &ImageSourceWorker::grabFrame,
            Qt::UniqueConnection);

    switch (source_->grab_mode())
    {
        case IImageSource::GrabMode::Timed:
            timer_->setSingleShot(false);
            timer_->start(interval_ms_);
            break;

        case IImageSource::GrabMode::Continuous:
            timer_->setSingleShot(false);
            timer_->start(1);
            break;

        case IImageSource::GrabMode::Once:
            timer_->setSingleShot(true);
            timer_->start(0);
            break;
    }
}

void ImageSourceWorker::onStop()
{
    running_ = false;

    if (timer_)
    {
        if (timer_->isActive())
            timer_->stop();
        timer_.reset();
    }

    if (source_)
        source_->close();
}


void ImageSourceWorker::onPlay()
{
    if (!source_ || !timer_)
        return;

    if (pause_)
    {
        pause_ = false;
        timer_->start();
    }
}

void ImageSourceWorker::onPause()
{
    if (!source_ || !timer_)
        return;

    if (!pause_)
    {
        pause_ = true;
        timer_->stop();
    }
}

void ImageSourceWorker::grabFrame()
{
    if (!running_ || !source_)
        return;

    try
    {
        cv::Mat frame = source_->grab();
        if (frame.empty())
        {
            onStop();
            return;
        }

        emit newFrame(frame);
    }
    catch (const std::exception& e)
    {
        emit errorOccurred(QString::fromStdString(e.what()));
        onStop();
    }
}
