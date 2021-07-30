#include "Quadtree.hpp"

#include <cmath>
#include <utility>

namespace Gaia::Modules
{
    float GetQuadtreeNodeDistance(QuadtreeNode* a, QuadtreeNode* b)
    {
        if (!a || !b) return 0;
        float delta_x = a->GetX() - b->GetX();
        float delta_y = a->GetY() - b->GetY();
        return std::sqrt(delta_x * delta_x + delta_y * delta_y);
    }

    void Quadtree::ArrangeNode(QuadtreeNode *starting_node, QuadtreeNode *node)
    {
        if (!starting_node || !node) return;
        if (node->GetX() <= starting_node->GetX())
        {
            if (node->GetY() <= starting_node->GetY())
            {
                if (starting_node->BottomLeft.Node)
                {
                    ArrangeNode(starting_node->BottomLeft.Node);
                }
                else
                {
                    starting_node->BottomLeft.Node = node;
                    starting_node->BottomLeft.Distance = GetQuadtreeNodeDistance(starting_node, node);
                }
            }
            else
            {
                if (starting_node->TopLeft.Node)
                {
                    ArrangeNode(starting_node->TopLeft.Node);
                }
                else
                {
                    starting_node->TopLeft.Node = node;
                    starting_node->TopLeft.Distance = GetQuadtreeNodeDistance(starting_node, node);
                }
            }
        }
        else
        {
            if (node->GetY() <= starting_node->GetY())
            {
                if (starting_node->BottomRight.Node)
                {
                    ArrangeNode(starting_node->BottomRight.Node);
                }
                else
                {
                    starting_node->BottomRight.Node = node;
                    starting_node->BottomRight.Distance = GetQuadtreeNodeDistance(starting_node, node);
                }
            }
            else
                if (starting_node->TopRight.Node)
                {
                    ArrangeNode(starting_node->TopRight.Node);
                }
                else
                {
                    starting_node->TopRight.Node = node;
                    starting_node->TopRight.Distance = GetQuadtreeNodeDistance(starting_node, node);
                }
        }
    }

    void Quadtree::ArrangeNode(QuadtreeNode *node)
    {
        if (!node) return;

        ArrangeNode(CenterAnchor, node);
    }

    QuadtreeNode *Quadtree::AddNode(cv::RotatedRect rectangle)
    {
        auto node_pointer = std::make_unique<QuadtreeNode>();
        auto* node = node_pointer.get();
        node_pointer->Rectangle = std::move(rectangle);
        return std::get<0>(Nodes.emplace(node, std::move(node_pointer)))->second.get();;
    }

    void Quadtree::RemoveNode(QuadtreeNode *node, bool keep_sub_nodes)
    {
        if (node->TopLeft.Node) RemoveNode(node->TopLeft.Node, keep_sub_nodes);
        if (node->TopLeft.Node) RemoveNode(node->TopLeft.Node, keep_sub_nodes);
        if (node->TopLeft.Node) RemoveNode(node->TopLeft.Node, keep_sub_nodes);
        if (node->TopLeft.Node) RemoveNode(node->TopLeft.Node, keep_sub_nodes);
        if (keep_sub_nodes)
        {
            ArrangeNode(node);
        }
        else
        {
            auto finder = Nodes.find(node);
            Nodes.erase(finder);
        }
    }
}