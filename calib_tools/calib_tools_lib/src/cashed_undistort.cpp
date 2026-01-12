#include "calib_tools_lib/cashed_undistort.hpp"

#include <opencv2/imgproc.hpp>


void CashedUndistort::operator()(const cv::Mat& input_frame,
                                 cv::Mat& output_frame,
                                 const CameraIntrinsics& cam_params)
{
    if (input_frame.empty())
        throw std::invalid_argument("input_frame is empty");

    if (input_frame.channels() != 1 && input_frame.channels() != 3)
        throw std::invalid_argument("input_frame must have 1 or 3 channels");

    if (!maps_initialized_ || !(cam_params == last_params_) ||
        !(input_frame.size() == last_image_size_))
    {
        cam_params.get_rectify_map(input_frame.size(), map1_, map2_);
        last_params_ = cam_params;
        last_image_size_ = input_frame.size();
        maps_initialized_ = true;
    }

    cv::remap(input_frame, output_frame, map1_, map2_, cv::INTER_LINEAR);
}
