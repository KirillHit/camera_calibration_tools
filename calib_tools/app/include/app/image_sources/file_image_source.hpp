#ifndef FILE_IMAGE_SOURCE_HPP
#define FILE_IMAGE_SOURCE_HPP

#include "image_source_interface.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <stdexcept>
#include <string>

class FileImageSource : public IImageSource
{
public:
    explicit FileImageSource(const std::string& path);
    ~FileImageSource() override;

    void start() override;
    void close() override;

    cv::Mat grab() override;
    GrabMode grab_mode() override;

    std::string get_info() const override;

private:
    std::string file_path_;
    cv::VideoCapture capture_;
    cv::Mat image_;
    GrabMode mode_ = GrabMode::Once;
    bool opened_ = false;
};


#endif
