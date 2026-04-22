#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iostream>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"

constexpr int width  = 128;
constexpr int height = 128;

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
    bool operator<(const Point& p) const {
        return y != p.y ? y < p.y : x < p.x;
    }
};

std::ostream& operator<<(std::ostream& out, const Point& p)
{
    out << '(' << p.x << ", " << p.y << ')';
    return out;
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

// Point project(vec3 v){return {static_cast<int>(v.x),static_cast<int>(v.y)};}
Point project(vec3 v)
{
    return {static_cast<int>((v.x + 1) * width / 2.0), 
        static_cast<int>((v.y + 1) * height / 2.0)};
}

void get_line_point(std::vector<Point>& points, Point a, Point b)
{
    if (a.x > b.x)
        std::swap(a, b);
    float y = a.y;
    float x_len = static_cast<float>(b.x - a.x);
    for (int x = a.x; x <= b.x; x++)
    {
        Point P {x, y};
        points.push_back({P.x, P.y});
        y += (b.y - a.y) / x_len;
    }
}

void triangle(TGAImage& framebuffer, vec3 v1, vec3 v2, vec3 v3, TGAColor color)
{
    Point p1 = project(v1), p2 = project(v2), p3 = project(v3);
    std::vector<Point> points;
    get_line_point(points, p1, p2);
    get_line_point(points, p1, p3);
    get_line_point(points, p3, p2);
    std::sort(points.begin(), points.end());
    if (points.empty()) return;
    Point left, right;
    left = points[0];
    for (int i = 1; i < points.size(); i++)
    {
        if (left.y == points[i].y) continue;
        right = points[i - 1];
        line(framebuffer, left, right, color);
        left = points[i];
    }
}

int main(int argc, char** argv) {
    TGAImage framebuffer(width, height, TGAImage::RGB);
    auto start = std::chrono::steady_clock::now();


    // triangle(framebuffer, vec3(7, 45), vec3(35, 100), vec3(45, 60), red);
    // triangle(framebuffer, vec3(120, 35), vec3(90, 5), vec3(45, 110), white);
    // triangle(framebuffer, vec3(115, 83), vec3(80, 90), vec3(85, 120), green);


    Model model(argv[1]);
    
    std::srand(std::time({}));

    for (int i = 0; i < model.get_triangles_size(); i++)
    {
        auto [v1, v2, v3] = model.get_triangle(i);
        triangle(framebuffer, v1, v2, v3, (rand() % 3 ? yellow : blue));
    }

    auto end = std::chrono::steady_clock::now();
    framebuffer.write_tga_file("framebuffer.tga");
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms" << std::endl;
    system("start framebuffer.tga");
    return 0;
}

