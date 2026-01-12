#include "app/image_sources/stream_image_source.hpp"

#include <iostream>

StreamImageSource::StreamImageSource(const std::string& rtsp_url,
                                     const int& width,
                                     const int& height,
                                     const int& fps) :
    rtsp_url_(rtsp_url), width_(width), height_(height), fps_(fps)
{
}

StreamImageSource::~StreamImageSource()
{
    close();
}

void StreamImageSource::start()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (opened_)
        close();

    capture_.open(rtsp_url_, cv::CAP_FFMPEG);

    if (!capture_.isOpened())
        throw std::runtime_error("Failed to open RTSP stream: " + rtsp_url_);

    if (width_ > 0)
        capture_.set(cv::CAP_PROP_FRAME_WIDTH, width_);
    if (height_ > 0)
        capture_.set(cv::CAP_PROP_FRAME_HEIGHT, height_);
    if (fps_ > 0)
        capture_.set(cv::CAP_PROP_FPS, fps_);

    opened_ = true;
}

void StreamImageSource::close()
{
    if (capture_.isOpened())
        capture_.release();
    opened_ = false;
}

cv::Mat StreamImageSource::grab()
{
    if (!opened_)
        return {};

    cv::Mat frame;
    if (!capture_.read(frame))
    {
        close();
        return {};
    }
    return frame;
}

IImageSource::GrabMode StreamImageSource::grab_mode()
{
    return GrabMode::Continuous;
}

std::string StreamImageSource::get_info() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!opened_ || !capture_.isOpened())
        return "RTSP stream not opened";

    int real_width = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_WIDTH));
    int real_height = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_HEIGHT));
    double real_fps = capture_.get(cv::CAP_PROP_FPS);

    return "RTSP: " + rtsp_url_ + " | WxH: " + std::to_string(real_width) + "x" +
           std::to_string(real_height) + " | FPS: " + std::to_string(real_fps);
}
