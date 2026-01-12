#include "app/image_sources/file_image_source.hpp"

#include <filesystem>

FileImageSource::FileImageSource(const std::string& path) : file_path_(path) {}


FileImageSource::~FileImageSource()
{
    close();
}


void FileImageSource::start()
{
    if (opened_)
        close();

    capture_.open(file_path_);
    if (capture_.isOpened())
    {
        mode_ = GrabMode::Timed;
        opened_ = true;
        return;
    }

    image_ = cv::imread(file_path_, cv::IMREAD_COLOR);
    if (!image_.empty())
    {
        mode_ = GrabMode::Once;
        opened_ = true;
        return;
    }

    throw std::runtime_error("Failed to open file: " + file_path_);
}


void FileImageSource::close()
{
    if (capture_.isOpened())
        capture_.release();
    image_.release();
    opened_ = false;
}


cv::Mat FileImageSource::grab()
{
    if (!opened_)
        return {};

    if (mode_ == GrabMode::Once)
        return image_;

    cv::Mat frame;
    if (!capture_.read(frame))
    {
        close();
        return {};
    }
    return frame;
}

IImageSource::GrabMode FileImageSource::grab_mode()
{
    return mode_;
}

std::string FileImageSource::get_info() const
{
    std::string filename = std::filesystem::path(file_path_).filename().string();

    constexpr size_t max_length = 40;
    if (filename.length() > max_length)
        filename = filename.substr(0, max_length - 3) + "...";

    if (!opened_)
        return "Failed to open file: " + file_path_;

    int width = 0;
    int height = 0;

    if (mode_ == GrabMode::Once && !image_.empty())
    {
        width = image_.cols;
        height = image_.rows;
    }
    else if (capture_.isOpened())
    {
        width = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_WIDTH));
        height = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_HEIGHT));
    }

    return "File: " + filename + " | WxH: " + std::to_string(width) + "x" + std::to_string(height);
}
