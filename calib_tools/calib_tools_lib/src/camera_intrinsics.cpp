#include "calib_tools_lib/camera_intrinsics.hpp"

#include <iomanip>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

void CameraIntrinsics::validate() const
{
    if (fx <= 0.0)
        throw std::invalid_argument("CameraIntrinsics: fx must be positive");
    if (fy <= 0.0)
        throw std::invalid_argument("CameraIntrinsics: fy must be positive");
    if (alpha < 0.0 || alpha > 1.0)
        throw std::invalid_argument("CameraIntrinsics: alpha must be in [0, 1]");
    if (!std::isfinite(fx) || !std::isfinite(fy) || !std::isfinite(cx) || !std::isfinite(cy) ||
        !std::isfinite(k1) || !std::isfinite(k2) || !std::isfinite(k3) || !std::isfinite(p1) ||
        !std::isfinite(p2) || !std::isfinite(alpha))
        throw std::invalid_argument("CameraIntrinsics: parameters must be finite");
}

bool CameraIntrinsics::operator==(const CameraIntrinsics& other) const
{
    return fx == other.fx && fy == other.fy && cx == other.cx && cy == other.cy && k1 == other.k1 &&
           k2 == other.k2 && k3 == other.k3 && p1 == other.p1 && p2 == other.p2 &&
           alpha == other.alpha;
}

bool CameraIntrinsics::operator!=(const CameraIntrinsics& other) const
{
    return !(*this == other);
}

cv::Mat CameraIntrinsics::get_camera_matrix() const
{
    cv::Mat K = (cv::Mat_<double>(3, 3) << fx, 0, cx, 0, fy, cy, 0, 0, 1);
    return K;
}

cv::Mat CameraIntrinsics::get_dist_coeffs() const
{
    cv::Mat D = (cv::Mat_<double>(1, 5) << k1, k2, p1, p2, k3);
    return D;
}

void CameraIntrinsics::set_camera_matrix(const cv::Mat& K)
{
    if (K.rows == 3 && K.cols == 3 && K.type() == CV_64F)
    {
        fx = K.at<double>(0, 0);
        fy = K.at<double>(1, 1);
        cx = K.at<double>(0, 2);
        cy = K.at<double>(1, 2);
    }
}

void CameraIntrinsics::set_dist_coeffs(const cv::Mat& D)
{
    if ((D.rows == 1 && D.cols == 5) || (D.rows == 5 && D.cols == 1))
    {
        k1 = D.at<double>(0, 0);
        k2 = D.at<double>(0, 1);
        p1 = D.at<double>(0, 2);
        p2 = D.at<double>(0, 3);
        k3 = D.at<double>(0, 4);
    }
}

void
  CameraIntrinsics::get_rectify_map(const cv::Size& imageSize, cv::Mat& map1, cv::Mat& map2) const
{
    validate();

    if (imageSize.width <= 0 || imageSize.height <= 0)
        throw std::invalid_argument("Некорректный размер изображения");

    cv::Mat K = get_camera_matrix();
    cv::Mat D = get_dist_coeffs();

    cv::Mat newK = cv::getOptimalNewCameraMatrix(K, D, imageSize, alpha, imageSize);

    cv::initUndistortRectifyMap(K, D, cv::Mat(), newK, imageSize, CV_32FC1, map1, map2);
}

std::array<double, 9> CameraIntrinsics::to_array() const
{
    return std::array<double, 9> {fx, fy, cx, cy, k1, k2, k3, p1, p2};
}

void CameraIntrinsics::from_array(const std::array<double, 9>& array)
{
    fx = array[0];
    fy = array[1];
    cx = array[2];
    cy = array[3];
    k1 = array[4];
    k2 = array[5];
    k3 = array[6];
    p1 = array[7];
    p2 = array[8];
}

std::string CameraIntrinsics::to_ros2_yaml_string(int image_width, int image_height) const
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);

    ss << "camera_info:\n";
    ss << "  image_width: "
       << ((image_width >= 0) ? std::to_string(image_width) : std::string("<width>")) << "\n";
    ss << "  image_height: "
       << ((image_height >= 0) ? std::to_string(image_height) : std::string("<height>")) << "\n";
    ss << "  camera_name: <name>\n";
    ss << "  camera_matrix:\n";
    ss << "    rows: 3\n";
    ss << "    cols: 3\n";
    ss << "    data: [ " << fx << ", 0., " << cx << ", "
       << "0., " << fy << ", " << cy << ", "
       << "0., 0., 1. ]\n";

    ss << "  distortion_model: plumb_bob\n";
    ss << "  distortion_coefficients:\n";
    ss << "    rows: 1\n";
    ss << "    cols: 5\n";
    ss << "    data: [ " << k1 << ", " << k2 << ", " << p1 << ", " << p2 << ", " << k3 << " ]\n";

    ss << "  rectification_matrix:\n";
    ss << "    rows: 3\n";
    ss << "    cols: 3\n";
    ss << "    data: [1, 0, 0, 0, 1, 0, 0, 0, 1]\n";

    ss << "  projection_matrix:\n";
    ss << "    rows: 3\n";
    ss << "    cols: 4\n";
    ss << "    data: [ " << fx << ", 0., " << cx << ", 0., "
       << "0., " << fy << ", " << cy << ", 0., "
       << "0., 0., 1., 0. ]\n";

    return ss.str();
}
