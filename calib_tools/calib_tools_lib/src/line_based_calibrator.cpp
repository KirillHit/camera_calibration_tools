#include "calib_tools_lib/line_based_calibrator.hpp"


LineBasedCalibrator::LineBasedCalibrator()
{
    options_.linear_solver_type = ceres::SPARSE_NORMAL_CHOLESKY;
    options_.use_nonmonotonic_steps = true;
    options_.max_num_iterations = 1000;
    options_.minimizer_progress_to_stdout = false;
}

void LineBasedCalibrator::set_callback(ceres::IterationCallback* callback)
{
    std::lock_guard<std::mutex> lock(mtx_);

    options_.callbacks.push_back(callback);
}

ceres::Solver::Summary
  LineBasedCalibrator::optimize(const std::vector<std::vector<std::array<double, 2>>>& lines,
                                const CameraIntrinsics& init_cam_params,
                                CameraIntrinsics& out_cam_params)
{
    std::lock_guard<std::mutex> lock(mtx_);

    if (lines.empty() || (lines.size() == 1 && lines[0].size() < 3))
    {
        out_cam_params = init_cam_params;
        return ceres::Solver::Summary();
    }

    ceres::Problem problem;

    std::vector<std::vector<std::array<double, 2>>> undistorted_lines;
    std::vector<std::array<double, 3>> line_params;
    std::array<double, 9> cam_params_array = init_cam_params.to_array();

    for (const std::vector<std::array<double, 2>>& line : lines)
    {
        if (line.size() < 3)
            continue;

        undistorted_lines.push_back(line);

        double x0 = line.front()[0];
        double y0 = line.front()[1];
        double xl = line.back()[0];
        double yl = line.back()[1];

        double a = yl - y0;
        double b = x0 - xl;
        double c = -(a * x0 + b * y0);

        line_params.push_back({a, b, c});
    }

    for (size_t idx = 0; idx < undistorted_lines.size(); ++idx)
    {
        for (std::array<double, 2>& undistorted_point : undistorted_lines[idx])
        {
            auto loss = new ceres::HuberLoss(0.8);

            problem.AddResidualBlock(
              ReprojectionFunctor::create(undistorted_point[0], undistorted_point[1]),
              loss,
              cam_params_array.data(),
              undistorted_point.data());

            problem.AddResidualBlock(LineErrorFunctor::create(),
                                     loss,
                                     line_params[idx].data(),
                                     undistorted_point.data());
        }
    }

    problem.SetParameterLowerBound(cam_params_array.data(), 0, 1e-6);  // fx >= 1e-6
    problem.SetParameterLowerBound(cam_params_array.data(), 1, 1e-6);  // fy >= 1e-6

    problem.SetParameterLowerBound(cam_params_array.data(), 2, -1e6);
    problem.SetParameterUpperBound(cam_params_array.data(), 2, 1e6);
    problem.SetParameterLowerBound(cam_params_array.data(), 3, -1e6);
    problem.SetParameterUpperBound(cam_params_array.data(), 3, 1e6);

    for (int i = 4; i <= 8; ++i)
    {
        problem.SetParameterLowerBound(cam_params_array.data(), i, -3.0);
        problem.SetParameterUpperBound(cam_params_array.data(), i, 3.0);
    }

    ceres::Solver::Summary summary;
    ceres::Solve(options_, &problem, &summary);

    out_cam_params.from_array(cam_params_array);

    return summary;
}
