#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iostream>
#include "tgaimage.h"
#include "model.h"

constexpr int width  = 800;
constexpr int height = 800;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

struct Point {
    int x, y;
    Point operator* (float f) const {
        return {(int) std::round(x * f), (int) std::round(y * f)};
    }
    Point operator+ (const Point& p) const {
        return {x + p.x, y + p.y};
    }
    Point operator- (const Point& p) const {
        return {x - p.x, y - p.y};
    }
};

void line(TGAImage& framebuffer, Point a, Point b, TGAColor color,int seg = 100)
{
    bool steep = (std::abs(a.x - b.x) < std::abs(a.y - b.y));
    if (steep)
    {
        std::swap(a.x, a.y);
        std::swap(b.x, b.y);
    }
    if (a.x > b.x)
        std::swap(a, b);
    float y = a.y;
    float x_len = static_cast<float>(b.x - a.x);
    for (int x = a.x; x <= b.x; x++)
    {
        
        Point P {x, y};
        if (steep)
            framebuffer.set(P.y, P.x, color);
        else
            framebuffer.set(P.x, P.y, color);

        y += (b.y - a.y) / x_len;
    }
}

Point project(vec3 v)
{
    return {static_cast<int>((v.x + 1) * width / 2.0), 
        static_cast<int>((v.y + 1) * height / 2.0)};
}

int main(int argc, char** argv) {
    TGAImage framebuffer(width, height, TGAImage::RGB);
    Model model(argv[1]);
    
    std::srand(std::time({}));
    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < model.get_triangles_size(); i++)
    {
        auto [v1, v2, v3] = model.get_triangle(i);
        line(framebuffer, project(v1), project(v2), red);
        line(framebuffer, project(v1), project(v3), red);
        line(framebuffer, project(v2), project(v3), red);
        // std::cout << "triangle " << i << ":\n";
        // std::cout << v1 << std::endl;
        // std::cout << v2 << std::endl;
        // std::cout << v3 << std::endl; 
    }

    auto end = std::chrono::steady_clock::now();
    framebuffer.write_tga_file("framebuffer.tga");
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms" << std::endl;
    system("start framebuffer.tga");
    return 0;
}

