#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "clipping/cohen_sutherland.hpp"
#include "clipping/sutherland_hodgman.hpp"
#include "clipping/Liang_Barsky.hpp"

/*Hàm đọc shader từ file
  Đọc toàn bộ nội dung file vào buffer
  Trả về chuỗi chứa mã shader */
std::string loadShaderSource(const char* filepath) {
    std::ifstream file(filepath);
    std::stringstream buffer;
    
    buffer << file.rdbuf();
    return buffer.str();
}

// Hàm biên dịch shader
GLuint compileShader(GLenum type, const std::string& source) {
    // Tạo shader theo loại (vertex/fragment)
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr); // Gán mã nguồn cho shader
    glCompileShader(shader); // Biên dịch shader

    // Kiểm tra lỗi biên dịch
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, nullptr, info);
        std::cerr << "Shader compile error:\n" << info << std::endl;
    }
    return shader;
}

// Tạo chương trình shader
GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode = loadShaderSource(vertexPath);
    std::string fragmentCode = loadShaderSource(fragmentPath);
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode);

    // Tạo chương trình shader và liên kết các shader
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // Kiểm tra lỗi liên kết
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info[512];
        glGetProgramInfoLog(program, 512, nullptr, info);
        std::cerr << "Program link error:\n" << info << std::endl;
    }

    // Xóa shader sau khi đã liên kết
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// Tạo VAO/VBO từ danh sách điểm
std::pair<GLuint, GLuint> createVAO(const std::vector<glm::vec2>& vertices) {
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

    // Cấu hình thuộc tính đỉnh 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    //unbind
    glBindVertexArray(0); 
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    return {VAO, VBO};
}


// Cập nhật khung cắt theo chuột
void updateClippingWindow(GLFWwindow* window, glm::vec2& min, glm::vec2& max) {
    // Lấy vị trí chuột
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Lấy kích thước cửa sổ
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Chuyển đổi sang tọa độ chuẩn hóa [-1, 1]
    // Tạo khung cắt hình vuông xung quanh chuột
    float x = (xpos / width) * 2.0f - 1.0f;
    float y = 1.0f - (ypos / height) * 2.0f;
    float halfSize = 0.3f;
    min = glm::vec2(x - halfSize, y - halfSize);
    max = glm::vec2(x + halfSize, y + halfSize);
}

int main() {
    // Khởi tạo GLFW và tạo cửa sổ
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1600, 900, "Clipping Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_MULTISAMPLE);

    // Tạo chương trình shader
    GLuint shaderProgram = createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");

    // Danh sách các hình để vẽ
    std::vector<std::vector<glm::vec2>> shapes = {
        {{-0.95f, -0.8f}, {-0.95f, 0.7f}}, // line1
        {{-0.5f, 0.9f}, {0.7f, -0.9f}},    // line2
        {{-0.8f, -0.2f}, {-0.5f, 0.3f}, {-0.2f, -0.2f}}, // triangle1
        {{0.3f, 0.5f}, {0.6f, 0.9f}, {0.9f, 0.4f}},      // triangle2
        {{-0.9f, -0.9f}, {-0.5f, -0.9f}, {-0.5f, -0.5f}, {-0.9f, -0.5f}}, // rectangle1
        {{-0.2f, 0.5f}, {-0.1f, 0.7f}, {0.1f, 0.7f}, {0.2f, 0.5f}, {0.0f, 0.3f}}, // pentagon1
        {{0.5f, 0.0f}, {0.6f, 0.2f}, {0.8f, 0.2f}, {0.9f, 0.0f}, {0.7f, -0.2f}},   // pentagon2
        {{-0.3f, -0.4f}, {-0.15f, -0.15f}, {0.15f, -0.15f}, {0.3f, -0.4f}, {0.15f, -0.65f}, {-0.15f, -0.65f}} // hexagon
    };


    // Vòng lặp chính
    while (!glfwWindowShouldClose(window)) {
        // Xóa màn hình và sử dụng shader
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);

        // Cập nhật khung cắt theo chuột
        glm::vec2 clipMin, clipMax;
        updateClippingWindow(window, clipMin, clipMax);

        // Vẽ khung cắt
        std::vector<glm::vec2> clipBox = {
            clipMin,
            {clipMax.x, clipMin.y},
            clipMax,
            {clipMin.x, clipMax.y}
        };
        auto [clipVAO, clipVBO] = createVAO(clipBox);
        glBindVertexArray(clipVAO);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &clipVAO); // Giải phóng VAO
        glDeleteBuffers(1, &clipVBO);      // Giải phóng VBO

        // Duyệt và cắt từng hình
        for (const auto& shape : shapes) {
            // Nếu là đoạn thẳng
            if (shape.size() == 2) {
                // Co thể thay bằng thuật toán Liang-Barsky 
                //auto result = clipping::clipLineLiangBarsky(shape[0], shape[1], clipMin, clipMax);
                auto result = clipping::clipLine(shape[0], shape[1], clipMin, clipMax);
                if (result) {
                    auto [p1, p2] = result.value();
                    std::vector<glm::vec2> clipped = {p1, p2};
                    auto [vao, vbo] = createVAO(clipped);
                    glBindVertexArray(vao);
                    glDrawArrays(GL_LINES, 0, 2);
                    glBindVertexArray(0);
                    glDeleteVertexArrays(1, &vao); 
                    glDeleteBuffers(1, &vbo);      
                }
            } 
            else { // Nếu là đa giác
                auto clipped = clipping::clipPolygonSutherlandHodgman(shape, clipMin, clipMax);
                if (!clipped.empty()) {
                    auto [vao, vbo] = createVAO(clipped);
                    glBindVertexArray(vao);
                    glDrawArrays(GL_TRIANGLE_FAN, 0, clipped.size());
                    glBindVertexArray(0);
                    glDeleteVertexArrays(1, &vao); 
                    glDeleteBuffers(1, &vbo);      
                }
            }
        }


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Dọn dẹp và thoát
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
