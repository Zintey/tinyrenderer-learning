#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iostream>
#include "tgaimage.h"

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

template <typename T>
T lerp (T a, T b, float t)
{
    return a + (b - a) * t;
}

void line1(TGAImage& framebuffer, Point a, Point b, TGAColor color,int seg = 100)
{
    for (int i = 0; i < seg; i++)
    {
        float t = i / static_cast<float> (seg);
        Point P = lerp(a, b, t);
        framebuffer.set(P.x, P.y, color);
    }
}

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
    // float y = a.y; // 优化1
    int y = a.y; // 优化2
    // float error = 0; // 优化2
    int ierror = 0; // 优化3
    int x_len = b.x - a.x; // 优化3
    // float x_len = static_cast<float>(b.x - a.x); // 优化1
    for (int x = a.x; x <= b.x; x++)
    {
        // float t = (x - a.x) / static_cast<float> (b.x - a.x);
        // int y = std::round(a.y + (b.y - a.y) * t);
        
        Point P {x, y};
        if (steep)
            framebuffer.set(P.y, P.x, color);
        else
            framebuffer.set(P.x, P.y, color);

        // y += (b.y - a.y) / x_len; // 优化1
        // error += (b.y - a.y) / x_len; // 优化2
        ierror += 2 * std::abs(b.y - a.y);
        // if (error >= 0.5) // 优化2
        if (ierror > x_len) // 优化3
        {
            y += b.y > a.y ? 1 : -1;
            // error -= 1.0;
            ierror -= 2 * (b.x - a.x);
        }
        // y += (b.y > a.y ? 1 : -1) * (ierror > b.x - a.x);
        // ierror -= 2 * (b.x-a.x)   * (ierror > b.x - a.x);
    }
}


int main(int argc, char** argv) {
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    Point a {7, 3};
    Point b {12, 37};
    Point c {62, 53};

    line(framebuffer, a, b, blue);
    line(framebuffer, c, b, green);
    line(framebuffer, c, a, yellow);
    line(framebuffer, a, c, red);

    // test
    // 未优化 9756 ms
    // 优化1 6404 ms
    // 优化2? 7898 ms
    // 优化3 8032 ms 去除了float运算
    
    std::srand(std::time({}));
    auto start = std::chrono::steady_clock::now();
    for (int i=0; i<(1<<24); i++) {
        int ax = rand()%width, ay = rand()%height;
        int bx = rand()%width, by = rand()%height;
        Point a{ax, ay}, b{bx, by};
        line(framebuffer, a, b, {static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255)});
    }
    auto end = std::chrono::steady_clock::now();
    framebuffer.write_tga_file("framebuffer.tga");
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms" << std::endl;
    system("start framebuffer.tga");
    return 0;
}

