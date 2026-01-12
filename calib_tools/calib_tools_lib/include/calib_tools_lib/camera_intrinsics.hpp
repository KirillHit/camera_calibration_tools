#ifndef CAMERA_INTRINSICS_HPP
#define CAMERA_INTRINSICS_HPP

#include <opencv2/core.hpp>


/** @brief Structure representing intrinsic parameters of a pinhole camera model 
 *         with radial–tangential distortion (OpenCV model: k1, k2, p1, p2, k3). 
 */
struct CameraIntrinsics
{
    // --------------- Parameters ---------------

    double fx = 1.0;  // focal length along X axis
    double fy = 1.0;  // focal length along Y axis
    double cx = 0.0;  // optical center X coordinate (pixels)
    double cy = 0.0;  // optical center Y coordinate (pixels)

    // Distortion coefficients
    double k1 = 0.0;  // radial distortion, 1st order
    double k2 = 0.0;  // radial distortion, 2nd order
    double k3 = 0.0;  // radial distortion, 3rd order
    double p1 = 0.0;  // tangential distortion along X
    double p2 = 0.0;  // tangential distortion along Y

    // Image cropping coefficient
    double alpha = 0.0;  // 0.0 — remove empty regions, 1.0 — preserve full view

    // --------------- Methods ---------------
    bool operator==(const CameraIntrinsics& other) const;
    bool operator!=(const CameraIntrinsics& other) const;

    /** @brief Validates intrinsic camera parameters.
     *
     * @throws std::invalid_argument If any parameter is invalid.
    */
    void validate() const;

    /** @brief Returns the 3x3 camera matrix. */
    cv::Mat get_camera_matrix() const;
    /** @brief Returns the 1x5 distortion coefficients. */
    cv::Mat get_dist_coeffs() const;

    /** @brief Sets the camera intrinsic matrix K */
    void set_camera_matrix(const cv::Mat& K);
    /** @brief Sets the distortion coefficients [k1, k2, p1, p2, k3] */
    void set_dist_coeffs(const cv::Mat& D);

    /**
     * @brief Computes rectify maps used for distortion correction.
     *
     * @param imageSize Size of the input image for which the maps are generated.
     * @param map1 First remap matrix (x-coordinates) for cv::remap().
     * @param map2 Second remap matrix (y-coordinates) for cv::remap().
     *
     * @throws std::invalid_argument If camera parameters are invalid (checked in validate()).
     */
    void get_rectify_map(const cv::Size& imageSize, cv::Mat& map1, cv::Mat& map2) const;

    /** 
     * @brief Packs all intrinsic parameters into a fixed-size array.
     * 
     * @return Array in the order: {fx, fy, cx, cy, k1, k2, k3, p1, p2}.
     */
    std::array<double, 9> to_array() const;

    /**
     * @brief Loads intrinsic parameters from a fixed-size array.
     * 
     * @param array  Input array in the order: {fx, fy, cx, cy, k1, k2, k3, p1, p2}.
     */
    void from_array(const std::array<double, 9>& array);

    /**
     * @brief Generates a ROS2-compatible YAML string representing the camera intrinsic parameters.
     *
     * @param image_width   Width of the camera image in pixels.
     * @param image_height  Height of the camera image in pixels.
     * @param camera_name   Optional name assigned to the camera (defaults to "camera_0").
     *
     * @return std::string  A YAML-formatted string containing the camera parameters.
     */
    std::string to_ros2_yaml_string(int image_width, int image_height) const;
};

#endif
