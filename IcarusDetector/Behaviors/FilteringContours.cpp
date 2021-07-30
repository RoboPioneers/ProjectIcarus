#include "FilteringContours.hpp"

#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaarithm.hpp>
#include "../Components/ContourElement.hpp"
#include "../Modules/RectangleTool.hpp"
#include "../Modules/GeometryFeature.hpp"
#include "../Modules/FastDilate.hpp"

namespace Icarus
{
    /// Load contour filtering configurations.
    void FilteringContours::LoadConfigurations()
    {
        IntensityThreshold = GetConfigurator()->Get<unsigned int>("IntensityThreshold").value_or(150);
        ColorThreshold = GetConfigurator()->Get<int>("ColorThreshold").value_or(100);
        EnemyDilateSize = GetConfigurator()->Get<unsigned int>("EnemyDilateSize").value_or(30);
    }

    /// Initialize blackboard values.
    void FilteringContours::OnInitialize()
    {
        InitializeFacilities();

        IsEnemyRed = GetBlackboard()->GetPointer<bool>("IsEnemyRed", true);
        MainPicture = GetBlackboard()->GetPointer<cv::Mat>("MainPicture");
        MaskPicture = GetBlackboard()->GetPointer<cv::Mat>("MaskPicture");

        InterestedArea = GetBlackboard()->GetPointer<cv::Rect>("InterestedArea");

        Contours = GetBlackboard()->GetPointer<tbb::concurrent_vector<ContourElement::Pointer>>
                ("Contours");

        LoadConfigurations();

        DilateFilter = cv::cuda::createMorphologyFilter(
                cv::MORPH_DILATE, CV_8U, cv::getStructuringElement(cv::MORPH_RECT,
                                                                   cv::Size(3, 9)));
        MaskWriter =
                std::make_unique<Gaia::SharedPicture::PictureWriter>("icarus.detector.mask", 1920 * 1080 * 3);
    }

    BehaviorTree::Result FilteringContours::OnExecute()
    {
        DEBUG_BEGIN
        CheckReloadConfiguration();
        DEBUG_END

        this->Contours->clear();

        cv::cuda::Stream main_stream;
        cv::cuda::GpuMat gpu_original_picture;

        gpu_original_picture.upload((*MainPicture)(*InterestedArea), main_stream);

        // Prepare intensity mask.
        cv::cuda::GpuMat gpu_gray_mask;
        cv::cuda::cvtColor(gpu_original_picture,
                           gpu_gray_mask, cv::COLOR_BGR2GRAY, 0,
                           main_stream);
        cv::cuda::threshold(gpu_gray_mask, gpu_gray_mask, IntensityThreshold, 255,
                            cv::THRESH_BINARY, main_stream);

        // Prepare color mask.
        cv::cuda::GpuMat gpu_color_channels[3];
        cv::cuda::split(gpu_original_picture, gpu_color_channels, main_stream);
        cv::cuda::GpuMat gpu_b_signed, gpu_r_signed;
        gpu_color_channels[0].convertTo(gpu_b_signed, CV_32SC1, main_stream);
        gpu_color_channels[2].convertTo(gpu_r_signed, CV_32SC1, main_stream);
        cv::cuda::GpuMat gpu_color_difference;
        cv::cuda::GpuMat gpu_dilated_color_difference(gpu_r_signed.size(), CV_32SC1, cv::Scalar(0));
        if (*IsEnemyRed)
        {
            cv::cuda::addWeighted(gpu_r_signed, 1.0, gpu_b_signed, -1.0,
                                  0.0, gpu_color_difference, -1, main_stream);
        }
        else
        {
            cv::cuda::addWeighted(gpu_r_signed, -1.0, gpu_b_signed, 1.0,
                                  0.0, gpu_color_difference, -1, main_stream);
        }
        FastDilate32S(gpu_color_difference, gpu_dilated_color_difference,
                      cv::Size(static_cast<int>(EnemyDilateSize),
                               static_cast<int>(EnemyDilateSize)),
                      ColorThreshold, 255, 255, main_stream);
        cv::cuda::GpuMat gpu_color_mask;
        gpu_dilated_color_difference.convertTo(gpu_color_mask, CV_8UC1, main_stream);

        cv::cuda::GpuMat gpu_mask;
        cv::cuda::bitwise_and(gpu_color_mask, gpu_gray_mask, gpu_mask,
                              cv::noArray(), main_stream);
        gpu_mask.download(*MaskPicture, main_stream);
        main_stream.waitForCompletion();

        #ifdef OFFLINE
        cv::Mat color_mask_display;
        gpu_color_mask.download(color_mask_display, main_stream);
        main_stream.waitForCompletion();
        cv::resize(color_mask_display, color_mask_display, color_mask_display.size() / 2);
        cv::imshow("Color Mask", color_mask_display);
        #endif

        DEBUG_BEGIN
        cv::Mat mask_display;
        cv::resize(*MaskPicture, mask_display,
                   cv::Size(MaskPicture->cols / 2, MaskPicture->rows / 2));
        MaskWriter->Write(mask_display);
        DEBUG_END

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(*MaskPicture, contours,
                         cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE,
                         InterestedArea->tl());

        tbb::parallel_for_each(contours, [this](const std::vector<cv::Point>& contour){
            auto contour_element = std::make_shared<ContourElement>();
            contour_element->Points = contour;
            contour_element->Rectangle = cv::minAreaRect(contour);
            contour_element->Feature = Modules::GeometryFeature::Standardize(contour_element->Rectangle);
            this->Contours->emplace_back(std::move(contour_element));
        });

        return BehaviorTree::Result::Success;
    }
}