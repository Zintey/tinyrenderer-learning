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
vec3 light_dir = {0, 1, 1};
vec3 camera_dir = {0, 0, -1};
void triangle(TGAImage& framebuffer, vec3 v1, vec3 v2, vec3 v3, TGAColor color)
{
    vec3 l1 = v2 - v1;
    vec3 l2 = v3 - v1;
    vec3 f = l1.cross(l2).normalize();
    double intensity = std::max(f * (-light_dir), 0.0);
    if (f * -camera_dir <= 0) return;
    color = {static_cast<std::uint8_t>(color[0] * intensity), 
             static_cast<std::uint8_t>(color[1] * intensity),
             static_cast<std::uint8_t>(color[2] * intensity)};
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

int main(int argc, char** argv) {
    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::cin >> light_dir.x >> light_dir.y >> light_dir.z;
    light_dir = light_dir.normalize();
    auto start = std::chrono::steady_clock::now();


    // triangle(framebuffer, vec3(7, 45), vec3(35, 100), vec3(45, 60), red);
    // triangle(framebuffer, vec3(120, 35), vec3(90, 5), vec3(45, 110), white);
    // triangle(framebuffer, vec3(115, 83), vec3(80, 90), vec3(85, 120), green);


    Model model(argv[1]);
    
    std::srand(std::time({}));

    for (int i = 0; i < model.get_triangles_size(); i++)
    {
        auto [v1, v2, v3] = model.get_triangle(i);
        triangle(framebuffer, v1, v2, v3, white);//{static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255)});
    }

    auto end = std::chrono::steady_clock::now();
    framebuffer.write_tga_file("framebuffer.tga");
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms" << std::endl;
    system("start framebuffer.tga");
    return 0;
}

