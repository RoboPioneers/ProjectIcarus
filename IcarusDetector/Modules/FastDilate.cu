#include <opencv2/opencv.hpp>
#include <opencv2/core/cuda_stream_accessor.hpp>
#include <cmath>

__global__ void DeviceFastDilate(cv::cuda::PtrStepb input, cv::cuda::PtrStepb output,
                                 unsigned int picture_width, unsigned int picture_height,
                                 unsigned int kernel_width, unsigned int kernel_height,
                                 unsigned char lower_bound, unsigned char upper_bound,
                                 unsigned char target_value)
{
    const unsigned int id_x = blockIdx.x * blockDim.x + threadIdx.x;
    const unsigned int id_y = blockIdx.y * blockDim.y + threadIdx.y;

    bool found = false;
    const unsigned int x_begin = id_x * kernel_width;
    const unsigned int y_begin = id_y * kernel_height;
    const unsigned int x_end = x_begin + kernel_width <= picture_width ? x_begin + kernel_width : picture_width;
    const unsigned int y_end = y_begin + kernel_height <= picture_height ? y_begin + kernel_height : picture_height;
    for (int row = static_cast<int>(y_begin); row < y_end; ++row)
    {
        for (int column = static_cast<int>(x_begin); column < x_end; ++column)
        {
            const unsigned char& value = input(row, column);
            if(value > lower_bound && value <= upper_bound)
            {
                found = true;
                break;
            }
        }
        if (found)
            break;
    }
    if (!found) return;
    for (int row = static_cast<int>(y_begin); row < y_end; ++row)
    {
        for (int column = static_cast<int>(x_begin); column < x_end; ++column)
        {
            output(row, column) = target_value;
        }
    }
}

__global__ void DeviceFastDilate32S(cv::cuda::PtrStepi input, cv::cuda::PtrStepi output,
                                 unsigned int picture_width, unsigned int picture_height,
                                 unsigned int kernel_width, unsigned int kernel_height,
                                 int lower_bound, int upper_bound,
                                 int target_value)
{
    const unsigned int id_x = blockIdx.x * blockDim.x + threadIdx.x;
    const unsigned int id_y = blockIdx.y * blockDim.y + threadIdx.y;

    bool found = false;
    const unsigned int x_begin = id_x * kernel_width;
    const unsigned int y_begin = id_y * kernel_height;
    const unsigned int x_end = x_begin + kernel_width <= picture_width ? x_begin + kernel_width : picture_width;
    const unsigned int y_end = y_begin + kernel_height <= picture_height ? y_begin + kernel_height : picture_height;
    for (int row = static_cast<int>(y_begin); row < y_end; ++row)
    {
        for (int column = static_cast<int>(x_begin); column < x_end; ++column)
        {
            const int& value = input(row, column);
            if(value > lower_bound && value <= upper_bound)
            {
                found = true;
                break;
            }
        }
        if (found)
            break;
    }
    if (!found) return;
    for (int row = static_cast<int>(y_begin); row < y_end; ++row)
    {
        for (int column = static_cast<int>(x_begin); column < x_end; ++column)
        {
            output(row, column) = target_value;
        }
    }
}

/**
 * @brief Fast-Dilate is a fast algorithm to get the similar effect to dilate with a large kernel.
 * @param input
 * @param output
 * @param size
 * @param stream
 */
void FastDilate(const cv::cuda::GpuMat& input, cv::cuda::GpuMat& output, const cv::Size& area_size,
                unsigned char lower_bound, unsigned char upper_bound,
                unsigned int target_value, const cv::cuda::Stream& stream)
{
    if (output.empty())
    {
        output = cv::cuda::GpuMat(input.size(), CV_8UC1, cv::Scalar(0));
    }

    dim3 block_size;
    block_size.x = 16;
    block_size.y = 16;
    block_size.z = 1;
    dim3 grid_size;
    grid_size.x = std::ceil(input.cols * 1.0 / area_size.width / 16.0);
    grid_size.y = std::ceil(input.rows * 1.0 / area_size.height / 16.0);
    DeviceFastDilate<<<grid_size, block_size, 0, cv::cuda::StreamAccessor::getStream(stream)>>>(
            input, output, input.cols, input.rows, area_size.width, area_size.height,
            lower_bound, upper_bound, target_value);
}

/**
 * @brief Fast-Dilate is a fast algorithm to get the similar effect to dilate with a large kernel.
 * @param input
 * @param output
 * @param size
 * @param stream
 */
void FastDilate32S(const cv::cuda::GpuMat& input, cv::cuda::GpuMat& output, const cv::Size& area_size,
                int lower_bound, int upper_bound, int target_value, const cv::cuda::Stream& stream)
{
    if (output.empty())
    {
        output = cv::cuda::GpuMat(input.size(), CV_8UC1, cv::Scalar(0));
    }

    dim3 block_size;
    block_size.x = 16;
    block_size.y = 16;
    block_size.z = 1;
    dim3 grid_size;
    grid_size.x = std::ceil(input.cols * 1.0 / area_size.width / 16.0);
    grid_size.y = std::ceil(input.rows * 1.0 / area_size.height / 16.0);
    DeviceFastDilate32S<<<grid_size, block_size, 0, cv::cuda::StreamAccessor::getStream(stream)>>>(
            input, output, input.cols, input.rows, area_size.width, area_size.height,
            lower_bound, upper_bound, target_value);
}