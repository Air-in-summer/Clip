#pragma once
#include <glm/glm.hpp>
#include <optional>

/*
Thuật toán Liang–Barsky cho đoạn thẳng
Input: p1, p2 (điểm đầu và điểm cuối đoạn thẳng), min (tọa độ góc dưới bên trái), max (tọa độ góc trên bên phải)
Output: đoạn đã được cắt hoặc std::nullopt nếu nằm ngoài hoàn toàn
*/

namespace clipping {

    // Cắt đoạn thẳng bằng thuật toán Liang–Barsky
    // Trả về đoạn đã cắt hoặc std::nullopt nếu nằm ngoài hoàn toàn
    std::optional<std::pair<glm::vec2, glm::vec2>> clipLineLiangBarsky(glm::vec2 p1, glm::vec2 p2, glm::vec2 min, glm::vec2 max) {
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;

        float p[4] = {-dx, dx, -dy, dy};
        float q[4] = {p1.x - min.x, max.x - p1.x, p1.y - min.y, max.y - p1.y};

        float u1 = 0.0f;
        float u2 = 1.0f;

        for (int i = 0; i < 4; ++i) {
            //Nếu p[i] == 0 và q[i] < 0 thì đoạn thẳng nằm ngoài
            if (p[i] == 0) {
                if (q[i] < 0) return std::nullopt; 
            } 
            else { 
                float r = q[i] / p[i];
                if (p[i] < 0) {
                    u1 = std::max(u1, r);
                } 
                else {
                    u2 = std::min(u2, r);
                }
            }
        }

        if (u1 > u2) return std::nullopt; // đoạn nằm ngoài

        glm::vec2 clippedStart = p1 + u1 * glm::vec2(dx, dy);
        glm::vec2 clippedEnd   = p1 + u2 * glm::vec2(dx, dy);

        return std::make_pair(clippedStart, clippedEnd);
    }

}
