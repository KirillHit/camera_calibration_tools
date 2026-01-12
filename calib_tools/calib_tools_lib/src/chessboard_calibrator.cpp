#include "calib_tools_lib/chessboard_calibrator.hpp"

#include <iostream>

ChessboardCalibrator::ChessboardCalibrator(
  cv::Size pattern_size,
  float square_size,
  int distant_threshold,
  int image_max_size,
  bool adaptive_threshold) :
    pattern_size_(pattern_size), square_size_(square_size), distant_threshold_(distant_threshold),
    image_max_size_(image_max_size), adaptive_threshold_(adaptive_threshold)
{
    if (pattern_size_.width <= 0 || pattern_size_.height <= 0)
        throw std::invalid_argument("pattern_size must have positive width and height");
    if (square_size_ <= 0.0f)
        throw std::invalid_argument("square_size must be positive (meters)");
    if (distant_threshold_ < 0)
        throw std::invalid_argument("distant_threshold must be positive");
    if (image_max_size_ <= 0)
        throw std::invalid_argument("image_max_size_ must be positive");

    template_points_ = generate_template_points();
}

ChessboardCalibrator::ChessboardCalibrator(const ChessboardCalibrator& other)
{
    std::lock_guard<std::mutex> lock(other.mutex_);
    pattern_size_ = other.pattern_size_;
    square_size_ = other.square_size_;
    distant_threshold_ = other.distant_threshold_;
    image_max_size_ = other.image_max_size_;
    adaptive_threshold_ = other.adaptive_threshold_;
    image_size_ = other.image_size_;
    image_points_ = other.image_points_;
    object_points_ = other.object_points_;
    template_points_ = other.template_points_;
}

void ChessboardCalibrator::process_frame(const cv::Mat& frame, cv::Mat& out_frame)
{
    if (frame.empty())
        return;

    if (!image_size_.empty() && image_size_ != frame.size())
        reset();

    image_size_ = frame.size();

    cv::Mat proc_img, scaled_img;
    proc_img = frame.clone();
    out_frame = frame.clone();

    if (proc_img.channels() == 3)
        cv::cvtColor(proc_img, proc_img, cv::COLOR_BGR2GRAY);

    int max_size = std::max(frame.cols, frame.rows);
    double image_scale =
      (max_size > image_max_size_) ? static_cast<float>(image_max_size_) / max_size : 1.0;

    if (image_scale != 1.0)
        cv::resize(proc_img, scaled_img, cv::Size(), image_scale, image_scale, cv::INTER_LINEAR);
    else
        scaled_img = proc_img;

    std::vector<cv::Point2f> corners;
    bool found = cv::findChessboardCorners(
      scaled_img,
      pattern_size_,
      corners,
      cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK +
        ((adaptive_threshold_) ? cv::CALIB_CB_ADAPTIVE_THRESH : 0));
    if (!found)
        return;

    if (image_scale != 1.0)
        for (cv::Point2f& p : corners)
            p = cv::Point2f(p.x / image_scale, p.y / image_scale);

    cv::cornerSubPix(proc_img,
                     corners,
                     cv::Size(11, 11),
                     cv::Size(-1, -1),
                     cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.01));
    cv::drawChessboardCorners(out_frame, pattern_size_, corners, true);

    std::lock_guard<std::mutex> lock(mutex_);

    if (!image_points_.empty())
    {
        const auto& prev = image_points_.back();
        double mean_shift = cv::norm(corners, prev, cv::NORM_L1) / corners.size();

        if (mean_shift < distant_threshold_)
            return;
    }

    image_points_.push_back(corners);
    object_points_.push_back(template_points_);
}

ChessboardCalibrator::Result ChessboardCalibrator::calibrate() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    Result res;

    if (image_points_.size() < 3 || image_points_.size() != object_points_.size() ||
        image_size_.empty())
    {
        res.success = false;
        return res;
    }

    cv::Mat camera_matrix = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat dist_coeffs = cv::Mat::zeros(8, 1, CV_64F);

    std::vector<cv::Mat> rvecs, tvecs;

    res.rms_error = cv::calibrateCamera(
      object_points_,
      image_points_,
      image_size_,
      camera_matrix,
      dist_coeffs,
      rvecs,
      tvecs);

    res.success = cv::checkRange(camera_matrix) && cv::checkRange(dist_coeffs);
    if (!res.success)
        return res;

    res.intrinsics.set_camera_matrix(camera_matrix);
    res.intrinsics.set_dist_coeffs(dist_coeffs);

    return res;
}

size_t ChessboardCalibrator::size() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return image_points_.size();
}

void ChessboardCalibrator::reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    image_points_.clear();
    object_points_.clear();
    image_size_ = cv::Size();
}

cv::Size ChessboardCalibrator::pattern_size() const
{
    return pattern_size_;
}

double ChessboardCalibrator::square_size() const
{
    return square_size_;
}

std::vector<cv::Point3f> ChessboardCalibrator::generate_template_points()
{
    std::vector<cv::Point3f> pts;
    pts.reserve(pattern_size_.area());
    for (int y = 0; y < pattern_size_.height; ++y)
        for (int x = 0; x < pattern_size_.width; ++x)
            pts.emplace_back(x * square_size_, y * square_size_, 0.0f);
    return pts;
}
