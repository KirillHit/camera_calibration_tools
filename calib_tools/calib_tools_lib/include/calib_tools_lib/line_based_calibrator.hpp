#ifndef LINE_BASED_OPTIMIZER_HPP
#define LINE_BASED_OPTIMIZER_HPP

#include "camera_intrinsics.hpp"

#include <ceres/ceres.h>
#include <vector>

// -------------------- Residual Functors --------------------

/** @brief Projects a 2D point using a pinhole camera model with radial and tangential distortion. */
template <typename T>
struct PinholeCameraModel
{
    T fx, fy, cx, cy, k1, k2, k3, p1, p2;

    bool project(const T* point, T* distorted_point) const
    {
        T x_norm = (point[0] - cx) / ((fx < T(1e-12)) ? T(1e-12) : fx);
        T y_norm = (point[1] - cy) / ((fy < T(1e-12)) ? T(1e-12) : fy);

        T r2 = x_norm * x_norm + y_norm * y_norm;
        T r4 = r2 * r2;
        T r6 = r4 * r2;

        T radial = T(1.0) + k1 * r2 + k2 * r4 + k3 * r6;
        T xd =
          x_norm * radial + T(2.0) * p1 * x_norm * y_norm + p2 * (r2 + T(2.0) * x_norm * x_norm);
        T yd =
          y_norm * radial + p1 * (r2 + T(2.0) * y_norm * y_norm) + T(2.0) * p2 * x_norm * y_norm;

        distorted_point[0] = fx * xd + cx;
        distorted_point[1] = fy * yd + cy;

        return true;
    }
};

/** @brief Computes reprojection residuals between observed and predicted distorted points. */
struct ReprojectionFunctor
{
    ReprojectionFunctor(double obs_x, double obs_y) : obs_x_(obs_x), obs_y_(obs_y) {}

    template <typename T>
    bool operator()(const T* const cam, const T* const pred_point, T* residual) const
    {
        // cam = [fx, fy, cx, cy, k1, k2, k3, p1, p2]
        PinholeCameraModel<T>
          model {cam[0], cam[1], cam[2], cam[3], cam[4], cam[5], cam[6], cam[7], cam[8]};

        T projected_point[2];
        if (!model.project(pred_point, projected_point))
            return false;

        residual[0] = T(obs_x_) - projected_point[0];
        residual[1] = T(obs_y_) - projected_point[1];

        return true;
    }

    static ceres::CostFunction* create(const double observed_x, const double observed_y)
    {
        return new ceres::AutoDiffCostFunction<ReprojectionFunctor, 2, 9, 2>(
          new ReprojectionFunctor(observed_x, observed_y));
    }

    double obs_x_, obs_y_;
};

/** @brief Computes the perpendicular distance of a point to a 2D line ax + by + c = 0. */
struct LineErrorFunctor
{
    template <typename T>
    bool operator()(const T* const line, const T* const pred_point, T* residual) const
    {
        T a = line[0];
        T b = line[1];
        T c = line[2];

        T norm = sqrt(a * a + b * b);
        if (norm < T(1e-12))
            norm = T(1e-12);

        residual[0] = (a * pred_point[0] + b * pred_point[1] + c) / norm;

        return true;
    }

    static ceres::CostFunction* create()
    {
        return new ceres::AutoDiffCostFunction<LineErrorFunctor, 1, 3, 2>(new LineErrorFunctor());
    }
};

// -------------------- Optimizer --------------------


/**
 * @brief Performs joint optimization of camera intrinsics and undistorted line endpoints.
 * 
 * The optimizer creates reprojection and line-distance constraints for every point and solves
 * for camera parameters and corrected point locations.
 */
class LineBasedCalibrator
{
public:
    LineBasedCalibrator();
    ~LineBasedCalibrator() = default;

    /** @brief Registers a custom iteration callback. */
    void set_callback(ceres::IterationCallback* callback);

    /**
     * @brief Runs optimization on a set of line point sequences.
     * 
     * @param lines            Input distorted line points.
     * @param init_cam_params  Initial camera intrinsic parameters.
     * @return Ceres solver summary containing convergence statistics.
     */
    ceres::Solver::Summary optimize(const std::vector<std::vector<std::array<double, 2>>>& lines,
                                    const CameraIntrinsics& init_cam_params,
                                    CameraIntrinsics& out_cam_params);

private:
    ceres::Solver::Options options_;
    std::mutex mtx_;
};

#endif
