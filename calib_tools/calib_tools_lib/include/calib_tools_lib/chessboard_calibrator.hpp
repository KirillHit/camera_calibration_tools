#ifndef CHESSBOARD_CALIBRATION_HPP
#define CHESSBOARD_CALIBRATION_HPP

#include "camera_intrinsics.hpp"

#include <mutex>
#include <opencv2/opencv.hpp>


/** @brief Chessboard-based camera calibrator */
class ChessboardCalibrator
{
public:
    /** @brief Calibration result container */
    struct Result
    {
        bool success = false;         // Indicates whether calibration succeeded
        double rms_error = 0.0;       // RMS reprojection error
        CameraIntrinsics intrinsics;  // Estimated camera intrinsics
    };

    /**
     * @brief Construct a chessboard calibrator
     * 
     * @param pattern_size Number of inner corners per chessboard row and column
     * @param square_size Physical size of one chessboard square in meters
     * @param distant_threshold Minimum distance between accepted frames in pixels
     * @param image_scale Image resize scale factor (1.0 = original resolution)
     */
    ChessboardCalibrator(cv::Size pattern_size,
                         float square_size,
                         int distant_threshold,
                         int image_max_size = 720,
                         bool adaptive_threshold = true);

    ChessboardCalibrator(const ChessboardCalibrator&);
    ChessboardCalibrator& operator=(const ChessboardCalibrator& other) = delete;
    ChessboardCalibrator(ChessboardCalibrator&&) = delete;
    ChessboardCalibrator& operator=(ChessboardCalibrator&&) = delete;

    /**
     * @brief Process a single frame and detect chessboard corners
     * 
     * @param frame Input image
     * @param out_frame Output image with visualization
     */
    void process_frame(const cv::Mat& frame, cv::Mat& out_frame);

    /**
     * @brief Run camera calibration using collected frames
     * 
     * @return Result Calibration result and estimated parameters
     */
    Result calibrate() const;

    /** @brief Number of collected valid frames */
    size_t size() const;

    /** @brief Reset all collected calibration data */
    void reset();

    /** @brief Get the chessboard pattern size (number of inner corners per row and column). */
    cv::Size pattern_size() const;

    /** @brief Get the physical size of a single chessboard square in meters. */
    double square_size() const;

private:
    /** @brief Generate 3D object points for a chessboard pattern */
    std::vector<cv::Point3f> generate_template_points();

    cv::Size pattern_size_;
    double square_size_;
    int distant_threshold_;
    int image_max_size_;
    bool adaptive_threshold_;

    cv::Size image_size_;

    std::vector<std::vector<cv::Point2f>> image_points_;
    std::vector<std::vector<cv::Point3f>> object_points_;
    std::vector<cv::Point3f> template_points_;

    mutable std::mutex mutex_;
};

#endif
