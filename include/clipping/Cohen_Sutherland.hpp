#pragma once
#include <glm/glm.hpp>
#include <optional>

/*
Thuật toán Cohen–Sutherland cho đoạn thẳng
Input: p1, p2 (điểm đầu và điểm cuối đoạn thẳng), min (tọa độ góc dưới bên trái), max (tọa độ góc trên bên phải)
Output: đoạn đã được cắt hoặc std::nullopt nếu nằm ngoài hoàn toàn
*/

namespace clipping {

    // Mã vùng (region code)
    enum RegionCode {
        INSIDE = 0,  // 0000
        LEFT   = 1,  // 0001
        RIGHT  = 2,  // 0010
        BOTTOM = 4,  // 0100
        TOP    = 8   // 1000
    };

    // Tính mã vùng cho một điểm
    int computeCode(const glm::vec2& p, const glm::vec2& min, const glm::vec2& max) {
        int code = INSIDE;

        if (p.x < min.x) code |= LEFT;
        else if (p.x > max.x) code |= RIGHT;

        if (p.y < min.y) code |= BOTTOM;
        else if (p.y > max.y) code |= TOP;

        return code;
    }

    // Cắt đoạn thẳng theo cửa sổ hình chữ nhật
    // Trả về đoạn đã cắt hoặc std::nullopt nếu nằm ngoài hoàn toàn
    std::optional<std::pair<glm::vec2, glm::vec2>> clipLine(glm::vec2 p1, glm::vec2 p2, glm::vec2 min, glm::vec2 max) {
        int code1 = computeCode(p1, min, max);
        int code2 = computeCode(p2, min, max);

        bool accept = false;

        while (true) {
            if ((code1 | code2) == 0) {
                // Cả hai điểm nằm trong
                accept = true;
                break;
            } else if ((code1 & code2) != 0) {
                // Cả hai điểm nằm ngoài cùng một phía
                break;
            } else {
                // Ít nhất một điểm nằm ngoài -> tìm giao điểm
                int outCode = code1 ? code1 : code2;
                glm::vec2 newPoint;

                if (outCode & TOP) {
                    newPoint.x = p1.x + (p2.x - p1.x) * (max.y - p1.y) / (p2.y - p1.y);
                    newPoint.y = max.y;
                } 
                else if (outCode & BOTTOM) {
                    newPoint.x = p1.x + (p2.x - p1.x) * (min.y - p1.y) / (p2.y - p1.y);
                    newPoint.y = min.y;
                } 
                else if (outCode & RIGHT) {
                    newPoint.y = p1.y + (p2.y - p1.y) * (max.x - p1.x) / (p2.x - p1.x);
                    newPoint.x = max.x;
                } 
                else if (outCode & LEFT) {
                    newPoint.y = p1.y + (p2.y - p1.y) * (min.x - p1.x) / (p2.x - p1.x);
                    newPoint.x = min.x;
                }

                if (outCode == code1) {
                    p1 = newPoint;
                    code1 = computeCode(p1, min, max);
                } 
                else {
                    p2 = newPoint;
                    code2 = computeCode(p2, min, max);
                }
            }
        }
        
        // nếu chấp nhận đoạn, trả về đoạn đã cắt, ngược lại trả về std::nullopt
        if (accept) return std::make_pair(p1, p2);
        else return std::nullopt;
    }

}
