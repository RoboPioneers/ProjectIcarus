#include "GeometryFeature.hpp"

namespace Gaia::Modules
{
    GeometryFeature GeometryFeature::Standardize(const cv::RotatedRect& rectangle)
    {
        GeometryFeature parameters;

        parameters.Center = rectangle.center;

        cv::Point2f vertices[4];
        rectangle.points(vertices);

        const double angle = rectangle.angle >= 0.0f ? rectangle.angle : -rectangle.angle;

        if (rectangle.size.width >= rectangle.size.height)
        {
            parameters.Length = rectangle.size.width;
            parameters.Width = rectangle.size.height;

            if (rectangle.angle <= 0)
            {
                if (angle < 180.0f)
                {
                    // [0, -180)

                    parameters.Angle = angle;

                    parameters.Vectors.Direction = vertices[2] - vertices[1];
                    parameters.Vectors.ClockwiseDiagonal = vertices[3] - rectangle.center;
                    parameters.Vectors.AnticlockwiseDiagonal = vertices[2] - rectangle.center;
                }
                else
                {
                    // [180, 180]

                    parameters.Angle = angle;

                    parameters.Vectors.Direction = vertices[1] - vertices[2];
                    parameters.Vectors.ClockwiseDiagonal = vertices[1] - rectangle.center;
                    parameters.Vectors.AnticlockwiseDiagonal = vertices[0] - rectangle.center;
                }
            }
            else
            {
                // (0, 180]

                parameters.Angle = 180 - angle;

                parameters.Vectors.Direction = vertices[1]  - vertices[2];
                parameters.Vectors.ClockwiseDiagonal = vertices[1] - rectangle.center;
                parameters.Vectors.AnticlockwiseDiagonal = vertices[0] - rectangle.center;
            }
        }
        else
        {
            parameters.Length = rectangle.size.height;
            parameters.Width = rectangle.size.width;

            if (rectangle.angle <= 0)
            {
                if (angle < 90.0f)
                {
                    // [0, 90)

                    parameters.Angle = 90.0f + angle;

                    parameters.Vectors.Direction = vertices[1] - vertices[0];
                    parameters.Vectors.ClockwiseDiagonal = vertices[2] - rectangle.center;
                    parameters.Vectors.AnticlockwiseDiagonal = vertices[1] - rectangle.center;
                }
                else
                {
                    // [90 ~ 180]

                    parameters.Angle = angle - 90.0f;

                    parameters.Vectors.Direction = vertices[0] - vertices[1];
                    parameters.Vectors.ClockwiseDiagonal = vertices[0] - rectangle.center;
                    parameters.Vectors.AnticlockwiseDiagonal = vertices[3] - rectangle.center;
                }
            }
            else
            {
                if (rectangle.angle <= 90.0f)
                {
                    parameters.Angle = 90.0f - angle;

                    parameters.Vectors.Direction = vertices[1] - vertices[0];
                    parameters.Vectors.ClockwiseDiagonal = vertices[2] - rectangle.center;
                    parameters.Vectors.AnticlockwiseDiagonal = vertices[1] - rectangle.center;
                }
                else
                {
                    parameters.Angle = 270.0f - angle;

                    parameters.Vectors.Direction = vertices[0] - vertices[1];
                    parameters.Vectors.ClockwiseDiagonal = vertices[0] - rectangle.center;
                    parameters.Vectors.AnticlockwiseDiagonal = vertices[3] - rectangle.center;
                }
            }
        }
        return parameters;
    }

    bool GeometryFeature::IsIdentical(const GeometryFeature& a, const GeometryFeature& b)
    {
        if (a.Center.x != b.Center.x || a.Center.y != b.Center.y) return false;
        if (a.Width != b.Width || a.Length != b.Length) return false;
        if (a.Angle != b.Angle) return false;
        return true;
    }
}