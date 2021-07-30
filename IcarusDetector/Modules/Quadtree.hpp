#pragma once

#include <unordered_map>
#include <memory>
#include <opencv2/opencv.hpp>

namespace Gaia::Modules
{
    class QuadtreeNode;
    class QuadTreeConnection;

    class QuadTreeConnection
    {
    public:
        QuadtreeNode* Node {nullptr};
        float Distance {0};
    };

    class QuadtreeNode
    {
    public:
        QuadTreeConnection TopLeft;
        QuadTreeConnection BottomLeft;
        QuadTreeConnection TopRight;
        QuadTreeConnection BottomRight;

        cv::RotatedRect Rectangle;

        [[nodiscard]] inline float GetX() const
        {
            return Rectangle.center.x;
        }
        [[nodiscard]] inline float GetY() const
        {
            return Rectangle.center.y;
        }
        [[nodiscard]] inline float GetWidth() const
        {
            return Rectangle.size.width;
        }
        [[nodiscard]] inline float GetHeight() const
        {
            return Rectangle.size.height;
        }
    };

    /**
     * @brief Quadtree for rectangle organizing and searching on a plane.
     */
    class Quadtree
    {
    protected:
        std::unordered_map<QuadtreeNode*, std::unique_ptr<QuadtreeNode>> Nodes;

        QuadtreeNode* CenterAnchor;

        void ArrangeNode(QuadtreeNode* starting_node, QuadtreeNode* node);
        /// Find the suitable place and place the node there.
        void ArrangeNode(QuadtreeNode* node);

    public:
        /// Add a node to the quadtree.
        QuadtreeNode* AddNode(cv::RotatedRect rectangle);
        /**
         * @brief Remove a node from the quadtree.
         * @param node Node to remove.
         * @param keep_sub_nodes If true, sub nodes of the given nodes will be rearranged.
         */
        void RemoveNode(QuadtreeNode* node, bool keep_sub_nodes = true);

        /// Get pointer to the center anchor.
        [[nodiscard]] inline QuadtreeNode* GetCenterAnchor() const noexcept
        {
            return CenterAnchor;
        }
    };
}
