#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iostream>
#include <algorithm>

#include "tgaimage.h"
#include "model.h"

constexpr int width  = 800;
constexpr int height = 800;

constexpr TGAColor white   = {255, 255, 255, 255};
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};



// Point project(vec3 v){return {static_cast<int>(v.x),static_cast<int>(v.y)};}
Point project(vec3 v)
{
    return {static_cast<int>((v.x + 1) * width / 2.0), 
        static_cast<int>((v.y + 1) * height / 2.0)};
}

double area(vec3 v1, vec3 v2, vec3 v3) {
    std::vector<vec3> vertices {v1, v2, v3};
    int n = 3;
    double area = 0.0;
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        area += (vertices[i].x * vertices[j].y) - (vertices[j].x * vertices[i].y);
    }
    return std::abs(area) / 2.0;
}
double area(const std::vector<vec3>& vertices) {
    int n = vertices.size();
    if (n < 3) return 0.0;
    double area = 0.0;
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        area += (vertices[i].x * vertices[j].y) - (vertices[j].x * vertices[i].y);
    }
    return std::abs(area) / 2.0;
}

TGAColor operator* (const double& d, const TGAColor& c)  { return {static_cast<std::uint8_t>(c.bgra[0] * d),static_cast<std::uint8_t>(c.bgra[1] * d),static_cast<std::uint8_t>(c.bgra[2] * d)};}
TGAColor operator* (const TGAColor& c, const double& d)  { return d * c;}
TGAColor operator+ (const TGAColor& c, const TGAColor& d)  { return {static_cast<uint8_t>(c.bgra[0] + d.bgra[0]), static_cast<uint8_t>(c.bgra[1] + d.bgra[1]), static_cast<uint8_t>(c.bgra[2] + d.bgra[2])};}
TGAColor get_rand_color() {return {static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255)};}
void triangle(TGAImage& framebuffer, vec3 v1, vec3 v2, vec3 v3, TGAColor c1, TGAColor c2, TGAColor c3)
{
    Triangle t{project(v1), project(v2), project(v3)};
    
    int minX = std::max(0, std::min({t.a.x, t.b.x, t.c.x}));
    int minY = std::max(0, std::min({t.a.y, t.b.y, t.c.y}));
    int maxX = std::min(width - 1, std::max({t.a.x, t.b.x, t.c.x}));
    int maxY = std::min(height - 1, std::max({t.a.y, t.b.y, t.c.y}));

    double st = area(t.a, t.b, t.c);
    for (int x = minX; x <= maxX; x++) 
        for (int y = minY; y <= maxY; y++) 
            if (t.contains(Point{x, y}))
            {
                double a = area(t.a, t.b, {1.0 * x, 1.0 * y}) / st;
                double b = area(t.b, t.c, {1.0 * x, 1.0 * y}) / st;
                double c = area(t.c, t.a, {1.0 * x, 1.0 * y}) / st;
                TGAColor color = (a * c1) + (b * c2) + (c * c3);
                framebuffer.set(x, y, color);
            }
                

}

void triangle(TGAImage& framebuffer, vec3 v1, vec3 v2, vec3 v3, TGAColor color)
{
    vec3 l1 = v2 - v1;
    vec3 l2 = v3 - v1;
    vec3 f = l1.cross(l2).normalize();
    Triangle t{project(v1), project(v2), project(v3)}; 
    
    int minX = std::max(0, std::min({t.a.x, t.b.x, t.c.x}));
    int minY = std::max(0, std::min({t.a.y, t.b.y, t.c.y}));
    int maxX = std::min(width - 1, std::max({t.a.x, t.b.x, t.c.x}));
    int maxY = std::min(height - 1, std::max({t.a.y, t.b.y, t.c.y}));

    for (int x = minX; x <= maxX; x++) 
        for (int y = minY; y <= maxY; y++) 
            if (t.contains(Point{x, y})) 
                framebuffer.set(x, y, color);
}
void triangle(TGAImage& framebuffer,const Triangle& t, TGAColor color)
{
    int minX = std::max(0, std::min({t.a.x, t.b.x, t.c.x}));
    int minY = std::max(0, std::min({t.a.y, t.b.y, t.c.y}));
    int maxX = std::min(width - 1, std::max({t.a.x, t.b.x, t.c.x}));
    int maxY = std::min(height - 1, std::max({t.a.y, t.b.y, t.c.y}));

    for (int x = minX; x <= maxX; x++) 
        for (int y = minY; y <= maxY; y++) 
            if (t.contains(Point{x, y})) 
                framebuffer.set(x, y, color);
}

int main(int argc, char** argv) {
    TGAImage framebuffer(width, height, TGAImage::RGB);
    auto start = std::chrono::steady_clock::now();

    std::srand(std::time({}));

    // triangle(framebuffer, vec3(7  / 128.0 - 0.5, 45 / 128.0 - 0.5), vec3(3 / 128.0 - 0.5, 100 / 128.0 - 0.5), vec3(45 / 128.0 - 0.5, 60 / 128.0 - 0.5)
    //         , get_rand_color(), get_rand_color(), get_rand_color());
    // triangle(framebuffer, vec3(120 / 128.0 - 0.5, 35 / 128.0 - 0.5), vec3(90 / 128.0 - 0.5, 5 / 128.0 - 0.5), vec3(45 / 128.0 - 0.5, 110 / 128.0 - 0.5)
    //         , get_rand_color(), get_rand_color(), get_rand_color());
    // triangle(framebuffer, vec3(115 / 128.0 - 0.5, 83 / 128.0 - 0.5), vec3(80 / 128.0 - 0.5, 90 / 128.0 - 0.5), vec3(85 / 128.0 - 0.5, 120 / 128.0 - 0.5)
    //         , get_rand_color(), get_rand_color(), get_rand_color());


    Model model(argv[1]);
    
    std::vector<Triangle> triangles;
    for (int i = 0; i < model.get_triangles_size(); i++)
    {
        auto [v1, v2, v3] = model.get_triangle(i);
        triangles.emplace_back(v1, v2, v3, project);
    }
    std::sort(triangles.begin(), triangles.end(), [](const Triangle& t1, const Triangle& t2){
        return t1.z < t2.z;
    });
    for (const auto& t : triangles)
        triangle(framebuffer, t, get_rand_color());
    
    auto end = std::chrono::steady_clock::now();
    framebuffer.write_tga_file("framebuffer.tga");
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms" << std::endl;
    system("start framebuffer.tga");
    return 0;
}

