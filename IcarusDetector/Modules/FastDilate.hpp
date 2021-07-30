#pragma once

/**
 * @brief If a block of the kernel size in the input picture has one or more point in the given range,
 *        then this function will fill that block in the output picture with the given target value.
 * @param input Picture to process.
 * @param output Picture to store result.
 * @param block_size Size of the block.
 * @param lower_bound Lower bound of the required range.
 * @param upper_bound Upper bound of the required range.
 * @param target_value Value to fill the block.
 * @param stream Working stream.
 * @attention
 *  This function will not initialize output picture unless it is empty, and in that case, output
 *  picture will be initialized as a picture filled up with 0.
 */
extern void FastDilate(const cv::cuda::GpuMat& input, cv::cuda::GpuMat& output, const cv::Size& block_size,
                       unsigned char lower_bound, unsigned char upper_bound,
                       unsigned int target_value, const cv::cuda::Stream& stream);

/**
 * @brief If a block of the kernel size in the input picture has one or more point in the given range,
 *        then this function will fill that block in the output picture with the given target value.
 * @param input Picture to process.
 * @param output Picture to store result.
 * @param block_size Size of the block.
 * @param lower_bound Lower bound of the required range.
 * @param upper_bound Upper bound of the required range.
 * @param target_value Value to fill the block.
 * @param stream Working stream.
 * @attention
 *  This function will not initialize output picture unless it is empty, and in that case, output
 *  picture will be initialized as a picture filled up with 0.
 */
extern void FastDilate32S(const cv::cuda::GpuMat& input, cv::cuda::GpuMat& output, const cv::Size& block_size,
                       int lower_bound, int upper_bound, int target_value, const cv::cuda::Stream& stream);