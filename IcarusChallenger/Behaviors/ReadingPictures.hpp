#pragma once

#include "../Framework/ProcessorBase.hpp"
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>

namespace Icarus
{
    /// Display the main picture.
    class ReadingPictures : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)

    protected:

        /// Main picture in the blackboard.
        cv::Mat* MainPicture {nullptr};

        bool Flip {false};
        std::string PicturesFolder;

        boost::filesystem::directory_iterator PictureEnd {};
        boost::filesystem::directory_iterator PictureIterator {};


        /// Initialize camera facilities.
        void OnInitialize() override
        {
            InitializeFacilities();

            MainPicture = GetBlackboard()->GetPointer<cv::Mat>("MainPicture");
            if (boost::filesystem::exists(PicturesFolder))
            {
                PictureIterator = boost::filesystem::directory_iterator(PicturesFolder);
            }
            else throw std::runtime_error("Pictures Folder does not exist.");
        }

        /// Read a picture.
        BehaviorTree::Result OnExecute() override
        {
            if (PictureIterator == PictureEnd)
            {
                PictureIterator = boost::filesystem::directory_iterator(PicturesFolder);
                if (PictureIterator == PictureEnd)
                    throw std::runtime_error("Empty pictures folder given to ReadingPictures.");
            }
            auto picture = cv::imread(PictureIterator->path().string());
            if (picture.empty())
            {
                ++PictureIterator;
                return BehaviorTree::Result::Success;
            }
            if (Flip)
            {
                cv::flip(picture, picture, -1);
            }
            *MainPicture = picture;
            cv::resize(picture, picture, picture.size() / 2);
            cv::imshow("Picture", picture);
            if (cv::waitKey(1) == 32)
                ++PictureIterator;
            return BehaviorTree::Result::Success;
        }

    public:
        ReadingPictures(Behavior *parent_behavior, const std::unordered_set<std::string>& type_names,
                       std::string pictures_folder) : ProcessorBase(parent_behavior, type_names),
                                                   PicturesFolder(std::move(pictures_folder))
        {}
    };
}