#ifndef CAMERA_IMAGE_SOURCE_HPP
#define CAMERA_IMAGE_SOURCE_HPP

#include "image_source_interface.hpp"

#include <mutex>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <stdexcept>

class CameraImageSource : public IImageSource
{
public:
    explicit CameraImageSource(const int& device_index,
                               const int& width,
                               const int& height,
                               const int& fps,
                               const std::string& fourcc);
    ~CameraImageSource() override;

    void start() override;
    void close() override;

    cv::Mat grab() override;
    GrabMode grab_mode() override;

    std::string get_info() const override;

private:
    int device_index_;
    int width_, height_, fps_;
    const std::string fourcc_;

    cv::VideoCapture capture_;
    bool opened_ = false;

    mutable std::mutex mutex_;
};

#endif
