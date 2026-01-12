#ifndef CASHED_UNDISTORT_HPP
#define CASHED_UNDISTORT_HPP

#include "camera_intrinsics.hpp"

#include <opencv2/core.hpp>

/** @brief Undistortion helper that caches rectification maps to avoid recomputation. */
class CashedUndistort
{
public:
    /**
     * @brief Undistort an image using cached rectify maps.
     *
     * @param input_frame  Source distorted image (1 or 3 channels).
     * @param output_frame Destination image after undistortion.
     * @param cam_params   Camera intrinsic parameters used for rectification.
     */
    void operator()(const cv::Mat& input_frame,
                    cv::Mat& output_frame,
                    const CameraIntrinsics& cam_params);

private:
    CameraIntrinsics last_params_;
    cv::Size last_image_size_;
    cv::Mat map1_, map2_;
    bool maps_initialized_ = false;
};


#endif
