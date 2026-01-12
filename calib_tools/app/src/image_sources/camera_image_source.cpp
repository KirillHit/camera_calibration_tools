#include "app/image_sources/camera_image_source.hpp"

#include <iostream>

CameraImageSource::CameraImageSource(const int& device_index,
                                     const int& width,
                                     const int& height,
                                     const int& fps,
                                     const std::string& fourcc) :
    device_index_(device_index), width_(width), height_(height), fps_(fps), fourcc_(fourcc)
{
}

CameraImageSource::~CameraImageSource()
{
    close();
}

void CameraImageSource::start()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (opened_)
        close();

#ifdef _WIN32
    capture_.open(device_index_, cv::CAP_ANY);
#else
    capture_.open(device_index_, cv::CAP_V4L2);
#endif

    if (!capture_.isOpened())
        throw std::runtime_error("Failed to open camera " + std::to_string(device_index_));

    if (fourcc_.size() == 4)
        capture_.set(cv::CAP_PROP_FOURCC,
                     cv::VideoWriter::fourcc(fourcc_[0], fourcc_[1], fourcc_[2], fourcc_[3]));
    if (width_ > 0)
        capture_.set(cv::CAP_PROP_FRAME_WIDTH, width_);
    if (height_ > 0)
        capture_.set(cv::CAP_PROP_FRAME_HEIGHT, height_);
    if (fps_ > 0)
        capture_.set(cv::CAP_PROP_FPS, fps_);

    if (!capture_.isOpened())
        throw std::runtime_error(
          "Failed to apply parameters to camera " + std::to_string(device_index_));

    opened_ = true;
}

void CameraImageSource::close()
{
    if (capture_.isOpened())
        capture_.release();
    opened_ = false;
}

cv::Mat CameraImageSource::grab()
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

IImageSource::GrabMode CameraImageSource::grab_mode()
{
    return GrabMode::Continuous;
}

std::string CameraImageSource::get_info() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!opened_ || !capture_.isOpened())
        return "Camera not opened";

    int real_width_ = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_WIDTH));
    int real_height_ = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_HEIGHT));
    int real_fps_ = static_cast<int>(capture_.get(cv::CAP_PROP_FPS));
    int fourcc_int = static_cast<int>(capture_.get(cv::CAP_PROP_FOURCC));
    char fourcc_str[] = {
      static_cast<char>(fourcc_int & 0xFF),
      static_cast<char>((fourcc_int >> 8) & 0xFF),
      static_cast<char>((fourcc_int >> 16) & 0xFF),
      static_cast<char>((fourcc_int >> 24) & 0xFF),
      '\0'};

    return "Camera: " + std::to_string(device_index_) + " | WxH: " + std::to_string(real_width_) +
           "x" + std::to_string(real_height_) + " | FPS: " + std::to_string(real_fps_) +
           " | FOURCC: " + std::string(fourcc_str);
}
