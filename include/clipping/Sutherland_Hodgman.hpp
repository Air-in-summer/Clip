#pragma once
#include <glm/glm.hpp>
#include <vector>

/*
Thuật toán Sutherland_Hodgman cho trường hợp cửa sổ cắt là 1 hình chữ nhật
Input: polygon (đa giác lồi), min (tọa độ góc dưới bên trái), max (tọa độ góc trên bên phải)
Output: đa giác đã được cắt
*/

namespace clipping {

    // Kiểu định nghĩa cạnh cửa sổ
    enum class Edge {
        LEFT,
        RIGHT,
        BOTTOM,
        TOP
    };

    // Kiểm tra điểm có nằm trong vùng hiển thị theo cạnh
    bool inside(const glm::vec2& p, Edge edge, const glm::vec2& min, const glm::vec2& max) {
        switch (edge) {
            case Edge::LEFT:   return p.x >= min.x;
            case Edge::RIGHT:  return p.x <= max.x;
            case Edge::BOTTOM: return p.y >= min.y;
            case Edge::TOP:    return p.y <= max.y;
        }
        return false;
    }

    // Tính giao điểm giữa đoạn và cạnh cửa sổ
    glm::vec2 intersect(const glm::vec2& p1, const glm::vec2& p2, Edge edge, const glm::vec2& min, const glm::vec2& max) {
        float x, y;
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;

        switch (edge) {
            case Edge::LEFT:
                x = min.x;
                y = p1.y + dy * (min.x - p1.x) / dx;
                break;
            case Edge::RIGHT:
                x = max.x;
                y = p1.y + dy * (max.x - p1.x) / dx;
                break;
            case Edge::BOTTOM:
                y = min.y;
                x = p1.x + dx * (min.y - p1.y) / dy;
                break;
            case Edge::TOP:
                y = max.y;
                x = p1.x + dx * (max.y - p1.y) / dy;
                break;
        }

        return glm::vec2(x, y);
    }

    // Cắt đa giác theo một cạnh
    std::vector<glm::vec2> clipEdge(const std::vector<glm::vec2>& input, Edge edge, const glm::vec2& min, const glm::vec2& max) {
        std::vector<glm::vec2> output;
        if (input.empty()) return output;

        glm::vec2 S = input.back(); // điểm cuối cùng

        for (const glm::vec2& E : input) {
            if (inside(E, edge, min, max)) {
                if (!inside(S, edge, min, max)) {
                    output.push_back(intersect(S, E, edge, min, max));
                }
                output.push_back(E);
            } else if (inside(S, edge, min, max)) {
                output.push_back(intersect(S, E, edge, min, max));
            }
            S = E;
        }

        return output;
    }

    // Cắt đa giác lồi theo cửa sổ hình chữ nhật
    std::vector<glm::vec2> clipPolygonSutherlandHodgman(const std::vector<glm::vec2>& polygon, const glm::vec2& min, const glm::vec2& max) {
        std::vector<glm::vec2> result = polygon;
        result = clipEdge(result, Edge::LEFT, min, max);
        result = clipEdge(result, Edge::RIGHT, min, max);
        result = clipEdge(result, Edge::BOTTOM, min, max);
        result = clipEdge(result, Edge::TOP, min, max);
        return result;
    }
}
