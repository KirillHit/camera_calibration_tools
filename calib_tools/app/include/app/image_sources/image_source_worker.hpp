#ifndef IMAGE_SOURCE_WORKER_HPP
#define IMAGE_SOURCE_WORKER_HPP

#include "image_source_interface.hpp"

#include <QObject>
#include <QTimer>
#include <memory>


class ImageSourceWorker : public QObject
{
    Q_OBJECT
public:
    explicit ImageSourceWorker(QObject* parent = nullptr);
    ~ImageSourceWorker() override;

signals:
    void newFrame(cv::Mat frame);
    void errorOccurred(const QString& message);
    void infoOccurred(const QString& message);

public slots:
    void onSetSource(std::shared_ptr<IImageSource> src);
    void onSetFPS(unsigned int fps);
    void onStart();
    void onStop();
    void onPlay();
    void onPause();

private slots:
    void grabFrame();

private:
    std::shared_ptr<IImageSource> source_;
    std::unique_ptr<QTimer> timer_;
    bool running_ = false;
    bool pause_ = false;
    int interval_ms_ = 30;
};

#endif
