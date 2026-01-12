#ifndef STREAM_IMAGE_SOURCE_HPP
#define STREAM_IMAGE_SOURCE_HPP

#include "image_source_interface.hpp"

#include <mutex>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <stdexcept>

class StreamImageSource : public IImageSource
{
public:
    explicit StreamImageSource(const std::string& rtsp_url,
                               const int& width = 0,
                               const int& height = 0,
                               const int& fps = 0);
    ~StreamImageSource() override;

    void start() override;
    void close() override;

    cv::Mat grab() override;
    GrabMode grab_mode() override;

    std::string get_info() const override;

private:
    std::string rtsp_url_;
    int width_, height_, fps_;

    cv::VideoCapture capture_;
    bool opened_ = false;

    mutable std::mutex mutex_;
};

#endif
