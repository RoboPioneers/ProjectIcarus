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
        ValueThreshold = GetConfigurator()->Get<unsigned int>("ValueThreshold").value_or(150);
        SaturationThreshold = GetConfigurator()->Get<unsigned int>("SaturationThreshold").value_or(200);
        EnemyDilateSize = GetConfigurator()->Get<unsigned int>("EnemyDilateSize").value_or(10);
        AlleyDilateSize = GetConfigurator()->Get<unsigned int>("AlleyDilateSize").value_or(5);
    }

    /// Initialize blackboard values.
    void FilteringContours::OnInitialize()
    {
        InitializeFacilities();

        EnemyMinHue = GetBlackboard()->GetPointer<unsigned int>("EnemyMinHue", 5);
        EnemyMaxHue = GetBlackboard()->GetPointer<unsigned int>("EnemyMaxHue", 30);
        AlleyMinHue = GetBlackboard()->GetPointer<unsigned int>("AlleyMinHue", 100);
        AlleyMaxHue = GetBlackboard()->GetPointer<unsigned int>("AlleyMaxHue", 130);

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

        // Get gray picture.
        cv::cuda::GpuMat gpu_gray_mask;
        cv::cuda::cvtColor(gpu_original_picture,
                           gpu_gray_mask, cv::COLOR_BGR2GRAY, 0,
                           main_stream);
        cv::cuda::threshold(gpu_gray_mask, gpu_gray_mask, IntensityThreshold, 255,
                            cv::THRESH_BINARY, main_stream);

        // Split HSV picture.
        cv::cuda::GpuMat gpu_hsv_picture;
        cv::cuda::cvtColor(gpu_original_picture, gpu_hsv_picture, cv::COLOR_BGR2HSV, 0,
                           main_stream);
        std::vector<cv::cuda::GpuMat> hsv_channels;
        cv::cuda::split(gpu_hsv_picture, hsv_channels, main_stream);

        cv::cuda::GpuMat gpu_enemy_h_mask;
        cv::cuda::inRange(hsv_channels[0], *EnemyMinHue, *EnemyMaxHue,
                          gpu_enemy_h_mask, main_stream);
        cv::cuda::GpuMat gpu_alley_h_mask;
        cv::cuda::inRange(hsv_channels[0], *AlleyMinHue, *AlleyMaxHue,
                          gpu_alley_h_mask, main_stream);
        cv::cuda::GpuMat gpu_h_mask(hsv_channels[0].size(), CV_8UC1, cv::Scalar(0));
        // Fill enemy blocks with 255.
        FastDilate(gpu_enemy_h_mask, gpu_h_mask, cv::Size(static_cast<int>(EnemyDilateSize),
                                                          static_cast<int>(EnemyDilateSize)),
                   0, 255, 255, main_stream);
        // Overwrite alley blocks with 0.
        FastDilate(gpu_alley_h_mask, gpu_h_mask, cv::Size(static_cast<int>(AlleyDilateSize),
                                                          static_cast<int>(AlleyDilateSize)),
                   0, 255, 0, main_stream);

        cv::cuda::GpuMat gpu_s_mask;
        cv::cuda::threshold(hsv_channels[1], gpu_s_mask, SaturationThreshold, 255, cv::THRESH_BINARY,
                            main_stream);
        FastDilate(gpu_s_mask, gpu_s_mask, cv::Size(static_cast<int>(EnemyDilateSize),
                                                    static_cast<int>(EnemyDilateSize)),
                   0, 255, 255, main_stream);
        cv::cuda::GpuMat gpu_color_mask;
        cv::cuda::bitwise_and(gpu_h_mask, gpu_s_mask, gpu_color_mask, cv::noArray(), main_stream);
        FastDilate(gpu_color_mask, gpu_color_mask, cv::Size(static_cast<int>(EnemyDilateSize),
                                                            static_cast<int>(EnemyDilateSize)),
                   0, 255, 255, main_stream);
        cv::cuda::GpuMat gpu_mask;
        cv::cuda::bitwise_and(gpu_color_mask, gpu_gray_mask, gpu_mask);
        gpu_mask.download(*MaskPicture, main_stream);
        main_stream.waitForCompletion();

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