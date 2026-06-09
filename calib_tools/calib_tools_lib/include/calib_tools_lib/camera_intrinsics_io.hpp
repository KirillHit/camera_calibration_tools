#ifndef CAMERA_INTRINSICS_IO_HPP
#define CAMERA_INTRINSICS_IO_HPP

#include "calib_tools_lib/camera_intrinsics.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace calib_tools_lib::camera_intrinsics_io {

inline std::string
  to_ros2_yaml_string(const CameraIntrinsics& intrinsics, int image_width, int image_height)
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
    ss << "    data: [ " << intrinsics.fx << ", 0., " << intrinsics.cx << ", "
       << "0., " << intrinsics.fy << ", " << intrinsics.cy << ", "
       << "0., 0., 1. ]\n";

    ss << "  distortion_model: plumb_bob\n";
    ss << "  distortion_coefficients:\n";
    ss << "    rows: 1\n";
    ss << "    cols: 5\n";
    ss << "    data: [ " << intrinsics.k1 << ", " << intrinsics.k2 << ", " << intrinsics.p1 << ", "
       << intrinsics.p2 << ", " << intrinsics.k3 << " ]\n";

    ss << "  rectification_matrix:\n";
    ss << "    rows: 3\n";
    ss << "    cols: 3\n";
    ss << "    data: [1, 0, 0, 0, 1, 0, 0, 0, 1]\n";

    ss << "  projection_matrix:\n";
    ss << "    rows: 3\n";
    ss << "    cols: 4\n";
    ss << "    data: [ " << intrinsics.fx << ", 0., " << intrinsics.cx << ", 0., "
       << "0., " << intrinsics.fy << ", " << intrinsics.cy << ", 0., "
       << "0., 0., 1., 0. ]\n";

    return ss.str();
}

inline std::string
  to_ros2_json_string(const CameraIntrinsics& intrinsics, int image_width, int image_height)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);

    ss << "{";
    ss << "\"image_width\":";
    if (image_width >= 0)
        ss << image_width;
    else
        ss << "\"<width>\"";

    ss << ",\"image_height\":";
    if (image_height >= 0)
        ss << image_height;
    else
        ss << "\"<height>\"";

    ss << ",\"camera_name\":\"<name>\"";
    ss << ",\"camera_matrix\":{\"rows\":3,\"cols\":3,\"data\":[" << intrinsics.fx << ",0,"
       << intrinsics.cx << ",0," << intrinsics.fy << "," << intrinsics.cy << ",0,0,1]}";
    ss << ",\"distortion_model\":\"plumb_bob\"";
    ss << ",\"distortion_coefficients\":{\"rows\":1,\"cols\":5,\"data\":[" << intrinsics.k1 << ","
       << intrinsics.k2 << "," << intrinsics.p1 << "," << intrinsics.p2 << "," << intrinsics.k3
       << "]}";
    ss << ",\"rectification_matrix\":{\"rows\":3,\"cols\":3,\"data\":[1,0,0,0,1,0,0,0,1]}";
    ss << ",\"projection_matrix\":{\"rows\":3,\"cols\":4,\"data\":[" << intrinsics.fx << ",0,"
       << intrinsics.cx << ",0,0," << intrinsics.fy << "," << intrinsics.cy << ",0,0,0,1,0]}";
    ss << "}";

    return ss.str();
}


inline std::string extract_data_array_text(const std::string& text, const std::string& sectionName)
{
    const std::size_t sectionPos = text.find(sectionName);
    if (sectionPos == std::string::npos)
        throw std::invalid_argument("Missing section: " + sectionName);

    const std::size_t dataPos = text.find("data", sectionPos);
    if (dataPos == std::string::npos)
        throw std::invalid_argument("Missing data in section: " + sectionName);

    const std::size_t arrayBegin = text.find('[', dataPos);
    if (arrayBegin == std::string::npos)
        throw std::invalid_argument("Missing data array in section: " + sectionName);

    const std::size_t arrayEnd = text.find(']', arrayBegin);
    if (arrayEnd == std::string::npos)
        throw std::invalid_argument("Unterminated data array in section: " + sectionName);

    return text.substr(arrayBegin + 1, arrayEnd - arrayBegin - 1);
}

inline std::vector<double> parse_double_array(std::string arrayText)
{
    std::replace(arrayText.begin(), arrayText.end(), ',', ' ');

    std::istringstream stream(arrayText);
    std::vector<double> values;
    double value = 0.0;

    while (stream >> value)
        values.push_back(value);

    if (!stream.eof())
        throw std::invalid_argument("Invalid number in data array");

    return values;
}

inline std::vector<double> extract_data_array(const std::string& text,
                                              const std::string& sectionName,
                                              std::size_t expectedSize)
{
    std::vector<double> values = parse_double_array(extract_data_array_text(text, sectionName));
    if (values.size() != expectedSize)
    {
        std::ostringstream message;
        message << "Invalid data size in section " << sectionName << ": expected " << expectedSize
                << ", got " << values.size();
        throw std::invalid_argument(message.str());
    }

    return values;
}


inline CameraIntrinsics from_ros2_string(const std::string& text)
{
    if (text.find_first_not_of(" \t\r\n") == std::string::npos)
        throw std::invalid_argument("Import text is empty");

    const std::vector<double> cameraMatrix = extract_data_array(text, "camera_matrix", 9);
    const std::vector<double> distortionCoeffs =
      extract_data_array(text, "distortion_coefficients", 5);

    CameraIntrinsics intrinsics;
    intrinsics.fx = cameraMatrix[0];
    intrinsics.fy = cameraMatrix[4];
    intrinsics.cx = cameraMatrix[2];
    intrinsics.cy = cameraMatrix[5];
    intrinsics.k1 = distortionCoeffs[0];
    intrinsics.k2 = distortionCoeffs[1];
    intrinsics.p1 = distortionCoeffs[2];
    intrinsics.p2 = distortionCoeffs[3];
    intrinsics.k3 = distortionCoeffs[4];
    intrinsics.alpha = 0.0;
    intrinsics.validate();

    return intrinsics;
}

}  // namespace calib_tools_lib::camera_intrinsics_io

#endif
